/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/version_metadata.hh>
#include <paludis/portage_dep_parser.hh>

/** \file
 * Implementation for FakeRepository.
 *
 * \ingroup grpfakerepository
 */

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for FakeRepository.
     *
     * \ingroup grpfakerepository
     */
    template<>
    struct Implementation<FakeRepository> :
        private InstantiationPolicy<Implementation<FakeRepository>, instantiation_method::NonCopyableTag>,
        InternalCounted<Implementation<FakeRepository> >
    {
        /// Our category names.
        CategoryNamePartCollection::Pointer category_names;

        /// Our package names.
        std::map<CategoryNamePart, PackageNamePartCollection::Pointer > package_names;

        /// Our versions.
        std::map<QualifiedPackageName, VersionSpecCollection::Pointer > versions;

        /// Our metadata.
        std::map<std::string, VersionMetadata::Pointer > metadata;

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Constructor.
        Implementation();
    };

    Implementation<FakeRepository>::Implementation() :
        category_names(new CategoryNamePartCollection::Concrete)
    {
    }
}

FakeRepository::FakeRepository(const RepositoryName & name) :
    Repository(name, RepositoryCapabilities::create()
            .installable_interface(0)
            .installed_interface(0)
            .mask_interface(this)
            .news_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            ),
    RepositoryMaskInterface(),
    RepositoryUseInterface(),
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>)
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "fake");

    _info->add_section(config_info);
}

FakeRepository::~FakeRepository()
{
}

bool
FakeRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return (_imp->category_names->end() != _imp->category_names->find(c));
}

bool
FakeRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    return has_category_named(q.category) &&
        (_imp->package_names.find(q.category)->second->end() !=
         _imp->package_names.find(q.category)->second->find(q.package));
}

CategoryNamePartCollection::ConstPointer
FakeRepository::do_category_names() const
{
    return _imp->category_names;
}

QualifiedPackageNameCollection::ConstPointer
FakeRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        throw InternalError(PALUDIS_HERE, "no category named " + stringify(c));
    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);
    PackageNamePartCollection::Iterator p(_imp->package_names.find(c)->second->begin()),
        p_end(_imp->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(c + *p);
    return result;
}

VersionSpecCollection::ConstPointer
FakeRepository::do_version_specs(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.category))
        throw InternalError(PALUDIS_HERE, "no category");
    if (! has_package_named(n))
        throw InternalError(PALUDIS_HERE, "no package");
    return _imp->versions.find(n)->second;
}

bool
FakeRepository::do_has_version(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_category_named(q.category))
        throw InternalError(PALUDIS_HERE, "no category");
    if (! has_package_named(q))
        throw InternalError(PALUDIS_HERE, "no package");
    return _imp->versions.find(q)->second->find(v) !=
        _imp->versions.find(q)->second->end();
}

void
FakeRepository::add_category(const CategoryNamePart & c)
{
    _imp->category_names->insert(c);
    _imp->package_names.insert(std::make_pair(c, new PackageNamePartCollection::Concrete));
}

void
FakeRepository::add_package(const QualifiedPackageName & q)
{
    add_category(q.category);
    _imp->package_names.find(q.category)->second->insert(q.package);
    _imp->versions.insert(std::make_pair(q, new VersionSpecCollection::Concrete));
}

VersionMetadata::Pointer
FakeRepository::add_version(const QualifiedPackageName & q, const VersionSpec & v)
{
    add_package(q);
    _imp->versions.find(q)->second->insert(v);
    _imp->metadata.insert(
            std::make_pair(stringify(q) + "-" + stringify(v),
                VersionMetadata::Pointer(new VersionMetadata::Ebuild(PortageDepParser::parse_depend))));
    VersionMetadata::Pointer r(_imp->metadata.find(stringify(q) + "-" + stringify(v))->second);
    r->slot = SlotName("0");
    r->eapi = "0";
    r->get_ebuild_interface()->keywords = "test";
    return r;
}

VersionMetadata::ConstPointer
FakeRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_version(q, v))
        throw InternalError(PALUDIS_HERE, "no version");
    return _imp->metadata.find(stringify(q) + "-" + stringify(v))->second;
}

bool
FakeRepository::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

bool
FakeRepository::do_query_profile_masks(const QualifiedPackageName &, const VersionSpec &) const
{
    return false;
}

UseFlagState
FakeRepository::do_query_use(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return use_unspecified;
}

bool
FakeRepository::do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return false;
}

bool
FakeRepository::do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const
{
    return false;
}

bool
FakeRepository::do_is_arch_flag(const UseFlagName &) const
{
    return false;
}

bool
FakeRepository::do_is_expand_flag(const UseFlagName &) const
{
    return false;
}


bool
FakeRepository::do_is_expand_hidden_flag(const UseFlagName &) const
{
    return false;
}

std::string::size_type
FakeRepository::do_expand_flag_delim_pos(const UseFlagName &) const
{
    return 0;
}

bool
FakeRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
FakeRepository::invalidate() const
{
}

Repository::ProvideMapIterator
FakeRepository::begin_provide_map() const
{
    return _imp->provide_map.begin();
}

Repository::ProvideMapIterator
FakeRepository::end_provide_map() const
{
    return _imp->provide_map.end();
}

