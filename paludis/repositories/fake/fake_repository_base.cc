/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <map>
#include <paludis/repositories/fake/fake_repository_base.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/version_metadata.hh>
#include <paludis/portage_dep_parser.hh>

/** \file
 * Implementation for FakeRepositoryBase.
 *
 * \ingroup grpfakerepository
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for FakeRepositoryBase.
     *
     * \ingroup grpfakerepository
     */
    template<>
    struct Implementation<FakeRepositoryBase> :
        private InstantiationPolicy<Implementation<FakeRepositoryBase>, instantiation_method::NonCopyableTag>
    {
        /// Our category names.
        tr1::shared_ptr<CategoryNamePartCollection> category_names;

        /// Our package names.
        std::map<CategoryNamePart, tr1::shared_ptr<PackageNamePartCollection> > package_names;

        /// Our versions.
        std::map<QualifiedPackageName, tr1::shared_ptr<VersionSpecCollection> > versions;

        /// Our metadata.
        std::map<std::string, tr1::shared_ptr<VersionMetadata> > metadata;

        /// Our sets.
        std::map<SetName, tr1::shared_ptr<SetSpecTree::ConstItem> > sets;

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        const Environment * const env;

        /// Constructor.
        Implementation(const Environment * const);
    };

    Implementation<FakeRepositoryBase>::Implementation(const Environment * const e) :
        category_names(new CategoryNamePartCollection::Concrete),
        env(e)
    {
    }
}

FakeRepositoryBase::FakeRepositoryBase(const Environment * const e,
        const RepositoryName & our_name, const RepositoryCapabilities & caps,
        const std::string & f) :
    Repository(our_name, caps, f),
    RepositoryMaskInterface(),
    RepositoryUseInterface(),
    PrivateImplementationPattern<FakeRepositoryBase>(new Implementation<FakeRepositoryBase>(e))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "fake");

    _info->add_section(config_info);
}

FakeRepositoryBase::~FakeRepositoryBase()
{
}

bool
FakeRepositoryBase::do_has_category_named(const CategoryNamePart & c) const
{
    return (_imp->category_names->end() != _imp->category_names->find(c));
}

bool
FakeRepositoryBase::do_has_package_named(const QualifiedPackageName & q) const
{
    return has_category_named(q.category) &&
        (_imp->package_names.find(q.category)->second->end() !=
         _imp->package_names.find(q.category)->second->find(q.package));
}

tr1::shared_ptr<const CategoryNamePartCollection>
FakeRepositoryBase::do_category_names() const
{
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
FakeRepositoryBase::do_package_names(const CategoryNamePart & c) const
{
    tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    if (! has_category_named(c))
        return result;

    PackageNamePartCollection::Iterator p(_imp->package_names.find(c)->second->begin()),
        p_end(_imp->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(c + *p);
    return result;
}

tr1::shared_ptr<const VersionSpecCollection>
FakeRepositoryBase::do_version_specs(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.category))
        return tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);
    if (! has_package_named(n))
        return tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);
    return _imp->versions.find(n)->second;
}

bool
FakeRepositoryBase::do_has_version(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_category_named(q.category))
        return false;
    if (! has_package_named(q))
        return false;
    return _imp->versions.find(q)->second->find(v) !=
        _imp->versions.find(q)->second->end();
}

void
FakeRepositoryBase::add_category(const CategoryNamePart & c)
{
    _imp->category_names->insert(c);
    _imp->package_names.insert(std::make_pair(c, new PackageNamePartCollection::Concrete));
}

void
FakeRepositoryBase::add_package(const QualifiedPackageName & q)
{
    add_category(q.category);
    _imp->package_names.find(q.category)->second->insert(q.package);
    _imp->versions.insert(std::make_pair(q, new VersionSpecCollection::Concrete));
}

tr1::shared_ptr<VersionMetadata>
FakeRepositoryBase::add_version(const QualifiedPackageName & q, const VersionSpec & v)
{
    add_package(q);
    _imp->versions.find(q)->second->insert(v);
    _imp->metadata.insert(
            std::make_pair(stringify(q) + "-" + stringify(v),
                tr1::shared_ptr<VersionMetadata>(new FakeVersionMetadata)));
    tr1::shared_ptr<VersionMetadata> r(_imp->metadata.find(stringify(q) + "-" + stringify(v))->second);
    r->slot = SlotName("0");
    r->eapi = EAPIData::get_instance()->eapi_from_string("0");
    return r;
}

tr1::shared_ptr<const VersionMetadata>
FakeRepositoryBase::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_version(q, v))
        throw InternalError(PALUDIS_HERE, "no version");
    return _imp->metadata.find(stringify(q) + "-" + stringify(v))->second;
}

bool
FakeRepositoryBase::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

bool
FakeRepositoryBase::do_query_profile_masks(const QualifiedPackageName &, const VersionSpec &) const
{
    return false;
}

UseFlagState
FakeRepositoryBase::do_query_use(const UseFlagName &, const PackageDatabaseEntry &) const
{
    return use_unspecified;
}

bool
FakeRepositoryBase::do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry &) const
{
    return false;
}

bool
FakeRepositoryBase::do_query_use_force(const UseFlagName &, const PackageDatabaseEntry &) const
{
    return false;
}

tr1::shared_ptr<const UseFlagNameCollection>
FakeRepositoryBase::do_arch_flags() const
{
    return tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

void
FakeRepositoryBase::invalidate()
{
}

tr1::shared_ptr<const UseFlagNameCollection>
FakeRepositoryBase::do_use_expand_flags() const
{
    return tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

tr1::shared_ptr<const UseFlagNameCollection>
FakeRepositoryBase::do_use_expand_hidden_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

tr1::shared_ptr<const UseFlagNameCollection>
FakeRepositoryBase::do_use_expand_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameCollection>(new UseFlagNameCollection::Concrete);
}

void
FakeRepositoryBase::add_package_set(const SetName & n, tr1::shared_ptr<SetSpecTree::ConstItem> s)
{
    _imp->sets.insert(std::make_pair(n, s));
}

tr1::shared_ptr<SetSpecTree::ConstItem>
FakeRepositoryBase::do_package_set(const SetName & id) const
{
    std::map<SetName, tr1::shared_ptr<SetSpecTree::ConstItem> >::const_iterator i(_imp->sets.find(id));
    if (_imp->sets.end() == i)
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
    else
        return i->second;
}

tr1::shared_ptr<const SetNameCollection>
FakeRepositoryBase::sets_list() const
{
    tr1::shared_ptr<SetNameCollection> result(new SetNameCollection::Concrete);
    std::copy(_imp->sets.begin(), _imp->sets.end(),
            transform_inserter(result->inserter(), SelectFirst<SetName, tr1::shared_ptr<SetSpecTree::ConstItem> >()));
    return result;
}

std::string
FakeRepositoryBase::do_describe_use_flag(const UseFlagName &,
        const PackageDatabaseEntry &) const
{
    return "";
}

const Environment *
FakeRepositoryBase::environment() const
{
    return _imp->env;
}

FakeVersionMetadata::FakeVersionMetadata() :
    VersionMetadata(
            VersionMetadataBase::create()
            .slot(SlotName("0"))
            .homepage("")
            .description("")
            .eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"))
            .interactive(false),
            VersionMetadataCapabilities::create()
            .ebuild_interface(this)
            .deps_interface(this)
            .license_interface(this)
            .cran_interface(0)
            .virtual_interface(0)
            .origins_interface(0)
            .ebin_interface(0)
            ),
    VersionMetadataEbuildInterface(),
    VersionMetadataDepsInterface(&PortageDepParser::parse_depend),
    VersionMetadataLicenseInterface(&PortageDepParser::parse_license)
{
    set_keywords("test");
}

FakeVersionMetadata::~FakeVersionMetadata()
{
}

FakeVirtualVersionMetadata::FakeVirtualVersionMetadata(const SlotName & s, const PackageDatabaseEntry & p) :
    VersionMetadata(
            VersionMetadataBase::create()
            .slot(s)
            .homepage("")
            .description("")
            .eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"))
            .interactive(false),
            VersionMetadataCapabilities::create()
            .ebuild_interface(0)
            .deps_interface(this)
            .license_interface(0)
            .cran_interface(0)
            .virtual_interface(this)
            .origins_interface(0)
            .ebin_interface(0)
            ),
    VersionMetadataDepsInterface(&PortageDepParser::parse_depend),
    VersionMetadataVirtualInterface(p)
{
}

FakeVirtualVersionMetadata::~FakeVirtualVersionMetadata()
{
}

