/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>

#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_operator.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/pretty_printer.hh>
#include <paludis/dep_spec_data.hh>

using namespace paludis;
using namespace paludis::virtuals;

namespace paludis
{
    template <>
    struct Imp<VirtualsDepKey>
    {
        const Environment * const env;
        const std::shared_ptr<DependencySpecTree> value;
        const std::shared_ptr<const DependenciesLabelSequence> labels;
        const std::shared_ptr<const PackageDepSpec> spec;

        const std::string raw_name;
        const std::string human_name;

        Imp(const Environment * const e, const std::shared_ptr<const PackageID> & v,
                const std::shared_ptr<const DependenciesLabelSequence> & l,
                bool exact, const std::string & h, const std::string & r) :
            env(e),
            value(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>())),
            labels(l),
            spec(exact ?
                    std::make_shared<PackageDepSpec>(
                            MutablePackageDepSpecData({ })
                            .require_package(v->name())
                            .require_version(vrc_and, vo_equal, v->version())
                            .require_exact_slot(v->slot_key() ? v->slot_key()->parse_value() : SlotName("UNKNOWN"), false)
                            .require_in_repository(v->repository_name()))
                    :
                    std::make_shared<PackageDepSpec>(
                            MutablePackageDepSpecData({ })
                            .require_package(v->name())
                            )
                ),
            raw_name(r),
            human_name(h)
        {
            value->top()->append(spec);
        }
    };
}

VirtualsDepKey::VirtualsDepKey(const Environment * const e, const std::string & r, const std::string & h,
        const std::shared_ptr<const PackageID> & v,
        const std::shared_ptr<const DependenciesLabelSequence> & l,
        const bool exact) :
    _imp(e, v, l, exact, r, h)
{
}

VirtualsDepKey::~VirtualsDepKey()
{
}

const std::shared_ptr<const DependencySpecTree>
VirtualsDepKey::parse_value() const
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

const std::string
VirtualsDepKey::pretty_print_value(
        const PrettyPrinter & pretty_printer,
        const PrettyPrintOptions &) const
{
    return pretty_printer.prettify(*_imp->spec);
}

const std::shared_ptr<const DependenciesLabelSequence>
VirtualsDepKey::initial_labels() const
{
    return _imp->labels;
}

namespace
{
    struct VirtualsDepKeyData :
        Singleton<VirtualsDepKeyData>
    {
        std::shared_ptr<DependenciesLabelSequence> bdep_labels;
        std::shared_ptr<DependenciesLabelSequence> rdep_labels;

        VirtualsDepKeyData() :
            bdep_labels(std::make_shared<DependenciesLabelSequence>()),
            rdep_labels(std::make_shared<DependenciesLabelSequence>())
        {
            bdep_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("DEPEND"));
            rdep_labels->push_back(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("RDEPEND"));
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<VirtualsPackageID>
    {
        const Environment * const env;
        const RepositoryName repository_name;
        const QualifiedPackageName name;
        const VersionSpec version;
        const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > virtual_for;
        const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > bdep;
        const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > rdep;
        mutable bool has_masks;
        mutable Mutex mutex;

        Imp(
                const Environment * const e,
                const RepositoryName & r,
                const QualifiedPackageName & n,
                const std::shared_ptr<const PackageID> & p,
                const bool b) :
            env(e),
            repository_name(r),
            name(n),
            version(p->version()),
            virtual_for(std::make_shared<LiteralMetadataValueKey<std::shared_ptr<const PackageID> > >("VIRTUAL_FOR", "Virtual for", mkt_normal, p)),
            bdep(std::make_shared<virtuals::VirtualsDepKey>(e, "DEPEND", "Build dependencies", p, VirtualsDepKeyData::get_instance()->bdep_labels, b)),
            rdep(std::make_shared<virtuals::VirtualsDepKey>(e, "RDEPEND", "Run dependencies", p, VirtualsDepKeyData::get_instance()->rdep_labels, b)),
            has_masks(false)
        {
        }
    };
}

VirtualsPackageID::VirtualsPackageID(
        const Environment * const e,
        const RepositoryName & r,
        const QualifiedPackageName & virtual_name,
        const std::shared_ptr<const PackageID> & virtual_for,
        const bool exact) :
    _imp(e, r, virtual_name, virtual_for, exact)
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
            return stringify(_imp->name) + "-" + stringify(_imp->version) + "::" + stringify(_imp->repository_name) +
                " (virtual for " + stringify(*_imp->virtual_for->parse_value()) + ")";

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repository_name) +
                " (virtual for " + _imp->virtual_for->parse_value()->canonical_form(idcf_no_version) + ")";

        case idcf_version:
            return stringify(_imp->version) + " (for " + stringify(_imp->virtual_for->parse_value()->canonical_form(idcf_no_version)) + ")";

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repository_name) +
                " (virtual for " + stringify(*_imp->virtual_for->parse_value()) + ")";

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
VirtualsPackageID::uniquely_identifying_spec() const
{
    /* hack: ensure the slot key's loaded, so that stringify returns the full form */
    _imp->virtual_for->parse_value()->slot_key();
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            (slot_key() ? ":" + stringify(slot_key()->parse_value()) : "") + "::" + stringify(repository_name()) +
            "[." + _imp->virtual_for->raw_name() + "=" + stringify(*_imp->virtual_for->parse_value()) + "]",
            _imp->env, { });
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

const RepositoryName
VirtualsPackageID::repository_name() const
{
    return _imp->repository_name;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
VirtualsPackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
VirtualsPackageID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
VirtualsPackageID::provide_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::dependencies_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::build_dependencies_key() const
{
    return _imp->bdep;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::run_dependencies_key() const
{
    return _imp->rdep;
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::post_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::suggested_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
VirtualsPackageID::short_description_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
VirtualsPackageID::long_description_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
VirtualsPackageID::fetches_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
VirtualsPackageID::homepage_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
VirtualsPackageID::contents_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >();
}

const std::shared_ptr<const MetadataTimeKey>
VirtualsPackageID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
VirtualsPackageID::from_repositories_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >();
}

bool
VirtualsPackageID::arbitrary_less_than_comparison(const PackageID & a) const
{
    if (a.virtual_for_key())
        return PackageIDSetComparator()(_imp->virtual_for->parse_value(), a.virtual_for_key()->parse_value());
    return false;
}

std::size_t
VirtualsPackageID::extra_hash_value() const
{
    return Hash<PackageID>()(*_imp->virtual_for->parse_value());
}

void
VirtualsPackageID::need_keys_added() const
{
}

namespace
{
    struct PerformAction
    {
        const Environment * const env;
        const PackageID * const id;

        void visit(const InstallAction & a)
        {
            SupportsActionTest<InstallAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }

        void visit(const PretendFetchAction & a)
        {
            SupportsActionTest<PretendFetchAction> t;
            auto repo(env->fetch_repository(id->repository_name()));
            if (! repo->some_ids_might_support_action(t))
                throw ActionFailedError("Unsupported action: " + a.simple_name());
        }
    };
}

void
VirtualsPackageID::perform_action(Action & a) const
{
    PerformAction b{_imp->env, this};
    a.accept(b);
}

bool
VirtualsPackageID::supports_action(const SupportsActionTestBase & b) const
{
    auto repo(_imp->env->fetch_repository(repository_name()));
    return repo->some_ids_might_support_action(b);
}

namespace
{
    class VirtualsAssociationMask :
        public AssociationMask
    {
        private:
            const PackageDepSpec _spec;

        public:
            VirtualsAssociationMask(const PackageDepSpec & s) :
                _spec(s)
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

            const PackageDepSpec associated_package_spec() const
            {
                return _spec;
            }
    };
}

void
VirtualsPackageID::need_masks_added() const
{
    Lock l(_imp->mutex);

    if (_imp->has_masks)
        return;

    if (_imp->virtual_for->parse_value()->masked())
        add_mask(std::make_shared<VirtualsAssociationMask>(_imp->virtual_for->parse_value()->uniquely_identifying_spec()));

    _imp->has_masks = true;
}

std::shared_ptr<const Set<std::string> >
VirtualsPackageID::breaks_portage() const
{
    return std::shared_ptr<const Set<std::string> >();
}

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
VirtualsPackageID::contains_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
VirtualsPackageID::contained_in_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataValueKey<FSPath> >
VirtualsPackageID::fs_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSPath> >();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
VirtualsPackageID::slot_key() const
{
    return _imp->virtual_for->parse_value()->slot_key();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
VirtualsPackageID::behaviours_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
VirtualsPackageID::choices_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >();
}

