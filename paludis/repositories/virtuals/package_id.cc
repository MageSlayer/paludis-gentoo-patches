/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <paludis/metadata_key.hh>
#include <paludis/hashed_containers.hh>

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
        const tr1::shared_ptr<const DependencySpecTree::ConstItem> value;

        Implementation(const tr1::shared_ptr<const PackageID> & v, bool exact) :
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

VirtualsDepKey::VirtualsDepKey(const std::string & r, const std::string & h,
        const tr1::shared_ptr<const PackageID> & v, const bool exact) :
    MetadataSpecTreeKey<DependencySpecTree>(r, h, mkt_dependencies),
    PrivateImplementationPattern<VirtualsDepKey>(new Implementation<VirtualsDepKey>(v, exact)),
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

namespace paludis
{
    template <>
    struct Implementation<VirtualsPackageID>
    {
        const tr1::shared_ptr<const Repository> repository;
        const QualifiedPackageName name;
        const VersionSpec version;
        const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > bdep;
        const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > rdep;

         Implementation(
                const tr1::shared_ptr<const Repository> & o,
                const QualifiedPackageName & n,
                const tr1::shared_ptr<const PackageID> & p) :
            repository(o),
            name(n),
            version(p->version()),
            virtual_for(new virtuals::VirtualsPackageIDKey(p)),
            bdep(new virtuals::VirtualsDepKey("DEPEND", "Build dependencies", p, o->installable_interface)),
            rdep(new virtuals::VirtualsDepKey("RDEPEND", "Run dependencies", p, o->installable_interface))
        {
        }
    };
}

VirtualsPackageID::VirtualsPackageID(
        const tr1::shared_ptr<const Repository> & owner,
        const QualifiedPackageName & virtual_name,
        const tr1::shared_ptr<const PackageID> & virtual_for) :
    PrivateImplementationPattern<VirtualsPackageID>(
            new Implementation<VirtualsPackageID>(owner, virtual_name, virtual_for)),
    _imp(PrivateImplementationPattern<VirtualsPackageID>::_imp.get())
{
    add_key(_imp->virtual_for);
    add_key(_imp->bdep);
    add_key(_imp->rdep);
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

const tr1::shared_ptr<const EAPI>
VirtualsPackageID::eapi() const
{
    return _imp->virtual_for->value()->eapi();
}

const tr1::shared_ptr<const MetadataPackageIDKey>
VirtualsPackageID::virtual_for_key() const
{
    return _imp->virtual_for;
}

const tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >
VirtualsPackageID::keywords_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<KeywordNameCollection> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> >
VirtualsPackageID::iuse_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<IUseFlagCollection> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >
VirtualsPackageID::inherited_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<InheritedCollection> >();
}

const tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >
VirtualsPackageID::use_key() const
{
    return tr1::shared_ptr<const MetadataCollectionKey<UseFlagNameCollection> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >
VirtualsPackageID::license_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> >();
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

const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >
VirtualsPackageID::restrict_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VirtualsPackageID::src_uri_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VirtualsPackageID::homepage_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
}

const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >
VirtualsPackageID::bin_uri_key() const
{
    return tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> >();
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

