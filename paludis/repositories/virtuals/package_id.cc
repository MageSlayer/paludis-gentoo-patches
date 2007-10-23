/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_spec.hh>
#include <paludis/environment.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/formatter.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>

using namespace paludis;
using namespace paludis::virtuals;

namespace paludis
{
    template <>
    struct Implementation<VirtualsPackageIDKey>
    {
        const tr1::shared_ptr<const PackageID> value;

        Implementation(const tr1::shared_ptr<const PackageID> & v) :
            value(v)
        {
        }
    };

    template <>
    struct Implementation<VirtualsDepKey>
    {
        const Environment * const env;
        const tr1::shared_ptr<const TreeLeaf<DependencySpecTree, PackageDepSpec> > value;

        Implementation(const Environment * const e, const tr1::shared_ptr<const PackageID> & v, bool exact) :
            env(e),
            value(exact ?
                    new TreeLeaf<DependencySpecTree, PackageDepSpec>(make_shared_ptr(new PackageDepSpec(
                                make_shared_ptr(new QualifiedPackageName(v->name())),
                                tr1::shared_ptr<CategoryNamePart>(),
                                tr1::shared_ptr<PackageNamePart>(),
                                make_equal_to_version_requirements(v->version()),
                                vr_and,
                                make_shared_ptr(new SlotName(v->slot())),
                                make_shared_ptr(new RepositoryName(v->repository()->name()))
                                )))
                    :
                    new TreeLeaf<DependencySpecTree, PackageDepSpec>(make_shared_ptr(new PackageDepSpec(
                                make_shared_ptr(new QualifiedPackageName(v->name()))
                                )))
                    )
        {
        }
    };
}

VirtualsPackageIDKey::VirtualsPackageIDKey(const tr1::shared_ptr<const PackageID> & v) :
    MetadataPackageIDKey("VIRTUAL_FOR", "Virtual for", mkt_normal),
    PrivateImplementationPattern<VirtualsPackageIDKey>(new Implementation<VirtualsPackageIDKey>(v)),
    _imp(PrivateImplementationPattern<VirtualsPackageIDKey>::_imp.get())
{
}

VirtualsPackageIDKey::~VirtualsPackageIDKey()
{
}

const tr1::shared_ptr<const PackageID>
VirtualsPackageIDKey::value() const
{
    return _imp->value;
}

VirtualsDepKey::VirtualsDepKey(const Environment * const e, const std::string & r, const std::string & h,
        const tr1::shared_ptr<const PackageID> & v, const bool exact) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, mkt_dependencies),
    PrivateImplementationPattern<VirtualsDepKey>(new Implementation<VirtualsDepKey>(e, v, exact)),
    _imp(PrivateImplementationPattern<VirtualsDepKey>::_imp.get())
{
}

VirtualsDepKey::~VirtualsDepKey()
{
}

const tr1::shared_ptr<const DependencySpecTree::ConstItem>
VirtualsDepKey::value() const
{
    return _imp->value;
}

std::string
VirtualsDepKey::pretty_print(const DependencySpecTree::ItemFormatter & f) const
{
    if (_imp->env)
    {
        if (! _imp->env->package_database()->query(query::Matches(*_imp->value->item()) &
                    query::InstalledAtRoot(_imp->env->root()), qo_whatever)->empty())
            return f.format(*_imp->value->item(), format::Installed());
        else if (! _imp->env->package_database()->query(query::Matches(*_imp->value->item()) &
                    query::SupportsAction<InstallAction>() & query::NotMasked(), qo_whatever)->empty())
            return f.format(*_imp->value->item(), format::Installable());
        else
            return f.format(*_imp->value->item(), format::Plain());
    }
    else
        return f.format(*_imp->value->item(), format::Plain());
}

std::string
VirtualsDepKey::pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const
{
    if (_imp->env)
    {
        if (! _imp->env->package_database()->query(query::Matches(*_imp->value->item()) &
                    query::InstalledAtRoot(_imp->env->root()), qo_whatever)->empty())
            return f.format(*_imp->value->item(), format::Installed());
        else if (! _imp->env->package_database()->query(query::Matches(*_imp->value->item()) &
                    query::SupportsAction<InstallAction>() & query::NotMasked(), qo_whatever)->empty())
            return f.format(*_imp->value->item(), format::Installable());
        else
            return f.format(*_imp->value->item(), format::Plain());
    }
    else
        return f.format(*_imp->value->item(), format::Plain());
}

namespace paludis
{
    template <>
    struct Implementation<VirtualsPackageID>
    {
        const Environment * const env;
        const tr1::shared_ptr<const Repository> repository;
        const QualifiedPackageName name;
        const VersionSpec version;
        const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > bdep;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > rdep;
        mutable bool has_masks;
        mutable Mutex mutex;

        Implementation(
                const Environment * const e,
                const tr1::shared_ptr<const Repository> & o,
                const QualifiedPackageName & n,
                const tr1::shared_ptr<const PackageID> & p,
                const bool b) :
            env(e),
            repository(o),
            name(n),
            version(p->version()),
            virtual_for(new virtuals::VirtualsPackageIDKey(p)),
            bdep(new virtuals::VirtualsDepKey(e, "DEPEND", "Build dependencies", p, b)),
            rdep(new virtuals::VirtualsDepKey(e, "RDEPEND", "Run dependencies", p, b)),
            has_masks(false)
        {
        }
    };
}

VirtualsPackageID::VirtualsPackageID(
        const Environment * const e,
        const tr1::shared_ptr<const Repository> & owner,
        const QualifiedPackageName & virtual_name,
        const tr1::shared_ptr<const PackageID> & virtual_for,
        const bool exact) :
    PrivateImplementationPattern<VirtualsPackageID>(
            new Implementation<VirtualsPackageID>(e, owner, virtual_name, virtual_for, exact)),
    _imp(PrivateImplementationPattern<VirtualsPackageID>::_imp.get())
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

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
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

const SlotName
VirtualsPackageID::slot() const
{
    return _imp->virtual_for->value()->slot();
}

const tr1::shared_ptr<const Repository>
VirtualsPackageID::repository() const
{
    return _imp->repository;
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VirtualsPackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >
VirtualsPackageID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> >();
}

const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >
VirtualsPackageID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
VirtualsPackageID::provide_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::build_dependencies_key() const
{
    return _imp->bdep;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::run_dependencies_key() const
{
    return _imp->rdep;
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::post_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
VirtualsPackageID::suggested_dependencies_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const tr1::shared_ptr<const MetadataStringKey>
VirtualsPackageID::short_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
VirtualsPackageID::long_description_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
VirtualsPackageID::fetches_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
VirtualsPackageID::homepage_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const tr1::shared_ptr<const MetadataContentsKey>
VirtualsPackageID::contents_key() const
{
    return tr1::shared_ptr<const MetadataContentsKey>();
}

const tr1::shared_ptr<const MetadataTimeKey>
VirtualsPackageID::installed_time_key() const
{
    return tr1::shared_ptr<const MetadataTimeKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
VirtualsPackageID::source_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
}

const tr1::shared_ptr<const MetadataStringKey>
VirtualsPackageID::binary_origin_key() const
{
    return tr1::shared_ptr<const MetadataStringKey>();
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
    return CRCHash<PackageID>()(*_imp->virtual_for->value());
}

void
VirtualsPackageID::need_keys_added() const
{
}

namespace
{
    struct PerformAction :
        ConstVisitor<ActionVisitorTypes>
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
                throw UnsupportedActionError(*id, a);
        }

        void visit(const InstalledAction & a)
        {
            SupportsActionTest<InstalledAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const UninstallAction & a)
        {
            SupportsActionTest<UninstallAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const ConfigAction & a)
        {
            SupportsActionTest<ConfigAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const FetchAction & a)
        {
            SupportsActionTest<FetchAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const InfoAction & a)
        {
            SupportsActionTest<InfoAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
        }

        void visit(const PretendAction & a)
        {
            SupportsActionTest<PretendAction> t;
            if (! id->repository()->some_ids_might_support_action(t))
                throw UnsupportedActionError(*id, a);
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
            const tr1::shared_ptr<const PackageID> _id;

        public:
            VirtualsAssociationMask(const tr1::shared_ptr<const PackageID> & i) :
                _id(i)
            {
            }

            const char key() const
            {
                return 'A';
            }

            const std::string description() const
            {
                return "by association";
            }

            const tr1::shared_ptr<const PackageID> associated_package() const
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

bool
VirtualsPackageID::breaks_portage() const
{
    return (version().has_try_part() || version().has_scm_part());
}

const tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >
VirtualsPackageID::contains_key() const
{
    return tr1::shared_ptr<const MetadataSetKey<PackageIDSequence> >();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VirtualsPackageID::contained_in_key() const
{
    return tr1::shared_ptr<const MetadataPackageIDKey>();
}

const tr1::shared_ptr<const MetadataFSEntryKey>
VirtualsPackageID::fs_location_key() const
{
    return tr1::shared_ptr<const MetadataFSEntryKey>();
}

