/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/repositories/virtuals/package_id.hh>
#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/repositories/virtuals/virtuals_repository.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_spec.hh>
#include <paludis/environment.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/formatter.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/package_database.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>

using namespace paludis;
using namespace paludis::virtuals;

namespace paludis
{
    template <>
    struct Implementation<VirtualsDepKey>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<DependencySpecTree> value;
        const std::tr1::shared_ptr<const DependenciesLabelSequence> labels;
        const std::tr1::shared_ptr<const PackageDepSpec> spec;

        const std::string raw_name;
        const std::string human_name;

        Implementation(const Environment * const e, const std::tr1::shared_ptr<const PackageID> & v,
                const std::tr1::shared_ptr<const DependenciesLabelSequence> & l,
                bool exact, const std::string & h, const std::string & r) :
            env(e),
            value(new DependencySpecTree(make_shared_ptr(new AllDepSpec))),
            labels(l),
            spec(exact ?
                    make_shared_ptr(new PackageDepSpec(
                            make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                            .package(v->name())
                            .version_requirement(make_named_values<VersionRequirement>(
                                    n::version_operator() = vo_equal,
                                    n::version_spec() = v->version()))
                            .slot_requirement(make_shared_ptr(new UserSlotExactRequirement(
                                        v->slot_key() ? v->slot_key()->value() : SlotName("UNKNOWN"))))
                            .in_repository(v->repository()->name())))
                    :
                    make_shared_ptr(new PackageDepSpec(
                            make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                            .package(v->name())
                            ))
                ),
            raw_name(r),
            human_name(h)
        {
            value->root()->append(spec);
        }
    };
}

VirtualsDepKey::VirtualsDepKey(const Environment * const e, const std::string & r, const std::string & h,
        const std::tr1::shared_ptr<const PackageID> & v,
        const std::tr1::shared_ptr<const DependenciesLabelSequence> & l,
        const bool exact) :
    PrivateImplementationPattern<VirtualsDepKey>(new Implementation<VirtualsDepKey>(e, v, l, exact, r, h)),
    _imp(PrivateImplementationPattern<VirtualsDepKey>::_imp)
{
}

VirtualsDepKey::~VirtualsDepKey()
{
}

const std::tr1::shared_ptr<const DependencySpecTree>
VirtualsDepKey::value() const
{
    return _imp->value;
}

const std::string
VirtualsDepKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
VirtualsDepKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
VirtualsDepKey::type() const
{
    return mkt_dependencies;
}

std::string
VirtualsDepKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    if (_imp->env)
    {
        if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(*_imp->spec, MatchPackageOptions()) |
                    filter::InstalledAtRoot(_imp->env->root()))]->empty())
            return f.format(*_imp->spec, format::Installed());
        else if (! (*_imp->env)[selection::SomeArbitraryVersion(generator::Matches(*_imp->spec, MatchPackageOptions()) |
                    filter::SupportsAction<InstallAction>() | filter::NotMasked())]->empty())
            return f.format(*_imp->spec, format::Installable());
        else
            return f.format(*_imp->spec, format::Plain());
    }
    else
        return f.format(*_imp->spec, format::Plain());
}

std::string
VirtualsDepKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    return pretty_print(f);
}

const std::tr1::shared_ptr<const DependenciesLabelSequence>
VirtualsDepKey::initial_labels() const
{
    return _imp->labels;
}

namespace paludis
{
    template <>
    struct Implementation<VirtualsPackageID>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const Repository> repository;
        const QualifiedPackageName name;
        const VersionSpec version;
        std::tr1::shared_ptr<DependenciesLabelSequence> bdep_labels;
        std::tr1::shared_ptr<DependenciesLabelSequence> rdep_labels;
        const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > > virtual_for;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > bdep;
        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > rdep;
        mutable bool has_masks;
        mutable Mutex mutex;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<const Repository> & o,
                const QualifiedPackageName & n,
                const std::tr1::shared_ptr<const PackageID> & p,
                const bool b) :
            env(e),
            repository(o),
            name(n),
            version(p->version()),
            bdep_labels(new DependenciesLabelSequence),
            rdep_labels(new DependenciesLabelSequence),
            virtual_for(new LiteralMetadataValueKey<std::tr1::shared_ptr<const PackageID> > ("VIRTUAL_FOR", "Virtual for", mkt_normal, p)),
            bdep(new virtuals::VirtualsDepKey(e, "DEPEND", "Build dependencies", p, bdep_labels, b)),
            rdep(new virtuals::VirtualsDepKey(e, "RDEPEND", "Run dependencies", p, rdep_labels, b)),
            has_masks(false)
        {
            bdep_labels->push_back(make_shared_ptr(new DependenciesBuildLabel("DEPEND",
                            return_literal_function(true))));
            rdep_labels->push_back(make_shared_ptr(new DependenciesRunLabel("RDEPEND",
                            return_literal_function(true))));
        }
    };
}

VirtualsPackageID::VirtualsPackageID(
        const Environment * const e,
        const std::tr1::shared_ptr<const Repository> & owner,
        const QualifiedPackageName & virtual_name,
        const std::tr1::shared_ptr<const PackageID> & virtual_for,
        const bool exact) :
    PrivateImplementationPattern<VirtualsPackageID>(
            new Implementation<VirtualsPackageID>(e, owner, virtual_name, virtual_for, exact)),
    _imp(PrivateImplementationPattern<VirtualsPackageID>::_imp)
{
    add_metadata_key(_imp->virtual_for);
    add_metadata_key(_imp->bdep);
    add_metadata_key(_imp->rdep);
}

VirtualsPackageID::~VirtualsPackageID()
{
}

const std::string
VirtualsPackageID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) + "::" + stringify(_imp->repository->name()) +
                " (virtual for " + stringify(*_imp->virtual_for->value()) + ")";

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repository->name()) +
                " (virtual for " + _imp->virtual_for->value()->canonical_form(idcf_no_version) + ")";

        case idcf_version:
            return stringify(_imp->version) + " (for " + stringify(_imp->virtual_for->value()->canonical_form(idcf_no_version)) + ")";

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repository->name()) +
                " (virtual for " + stringify(*_imp->virtual_for->value()) + ")";

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
VirtualsPackageID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->value()) : "") + "::" + stringify(repository()->name()) +
            "[." + _imp->virtual_for->raw_name() + "=" + stringify(*_imp->virtual_for->value()) + "]",
            _imp->env, UserPackageDepSpecOptions());
}

const QualifiedPackageName
VirtualsPackageID::name() const
{
    return _imp->name;
}

const VersionSpec
VirtualsPackageID::version() const
{
    return _imp->version;
}

const std::tr1::shared_ptr<const Repository>
VirtualsPackageID::repository() const
{
    return _imp->repository;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
VirtualsPackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
VirtualsPackageID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
VirtualsPackageID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::build_dependencies_key() const
{
    return _imp->bdep;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::run_dependencies_key() const
{
    return _imp->rdep;
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VirtualsPackageID::short_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
VirtualsPackageID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
VirtualsPackageID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
VirtualsPackageID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
VirtualsPackageID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
VirtualsPackageID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
VirtualsPackageID::from_repositories_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

bool
VirtualsPackageID::arbitrary_less_than_comparison(const PackageID & a) const
{
    if (a.virtual_for_key())
        return PackageIDSetComparator()(_imp->virtual_for->value(), a.virtual_for_key()->value());
    return false;
}

std::size_t
VirtualsPackageID::extra_hash_value() const
{
    return Hash<PackageID>()(*_imp->virtual_for->value());
}

void
VirtualsPackageID::need_keys_added() const
{
}

namespace
{
    struct PerformAction
    {
        const PackageID * const id;

        PerformAction(const PackageID * const i) :
            id(i)
        {
        }

        void visit(const InstallAction & a)
        {
            SupportsActionTest<InstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendFetchAction & a)
        {
            SupportsActionTest<PretendFetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }
    };
}

void
VirtualsPackageID::perform_action(Action & a) const
{
    PerformAction b(this);
    a.accept(b);
}

bool
VirtualsPackageID::supports_action(const SupportsActionTestBase & b) const
{
    return repository()->some_ids_might_support_action(b);
}

namespace
{
    class VirtualsAssociationMask :
        public AssociationMask
    {
        private:
            const std::tr1::shared_ptr<const PackageID> _id;

        public:
            VirtualsAssociationMask(const std::tr1::shared_ptr<const PackageID> & i) :
                _id(i)
            {
            }

            char key() const
            {
                return 'A';
            }

            const std::string description() const
            {
                return "by association";
            }

            const std::tr1::shared_ptr<const PackageID> associated_package() const
            {
                return _id;
            }
    };
}

void
VirtualsPackageID::need_masks_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_masks)
        return;

    if (_imp->virtual_for->value()->masked())
        add_mask(make_shared_ptr(new VirtualsAssociationMask(_imp->virtual_for->value())));

    _imp->has_masks = true;
}

void
VirtualsPackageID::invalidate_masks() const
{
    Lock l(_imp->mutex);

    if (! _imp->has_masks)
        return;

    _imp->has_masks = false;
    PackageID::invalidate_masks();
}

std::tr1::shared_ptr<const Set<std::string> >
VirtualsPackageID::breaks_portage() const
{
    return std::tr1::shared_ptr<const Set<std::string> >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
VirtualsPackageID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
VirtualsPackageID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VirtualsPackageID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
VirtualsPackageID::slot_key() const
{
    return _imp->virtual_for->value()->slot_key();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
VirtualsPackageID::behaviours_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
VirtualsPackageID::choices_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >();
}

