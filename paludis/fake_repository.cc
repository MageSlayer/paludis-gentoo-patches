/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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
#include <paludis/fake_repository.hh>
#include <paludis/util/stringify.hh>
#include <paludis/version_metadata.hh>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for FakeRepository.
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

        /// Constructor.
        Implementation() :
            category_names(new CategoryNamePartCollection)
        {
        }
    };
}

FakeRepository::FakeRepository(const RepositoryName & name) :
    Repository(name),
    PrivateImplementationPattern<FakeRepository>(new Implementation<FakeRepository>)
{
    _info.insert(std::make_pair(std::string("format"), std::string("fake")));
}

FakeRepository::~FakeRepository()
{
}

bool
FakeRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return (_implementation->category_names->end() != _implementation->category_names->find(c));
}

bool
FakeRepository::do_has_package_named(const CategoryNamePart & c,
        const PackageNamePart & p) const
{
    return has_category_named(c) &&
        (_implementation->package_names.find(c)->second->end() !=
         _implementation->package_names.find(c)->second->find(p));
}

CategoryNamePartCollection::ConstPointer
FakeRepository::do_category_names() const
{
    return _implementation->category_names;
}

QualifiedPackageNameCollection::ConstPointer
FakeRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        throw InternalError(PALUDIS_HERE, "no category named " + stringify(c));
    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection);
    PackageNamePartCollection::Iterator p(_implementation->package_names.find(c)->second->begin()),
        p_end(_implementation->package_names.find(c)->second->end());
    for ( ; p != p_end ; ++p)
        result->insert(QualifiedPackageName(c, *p));
    return result;
}

VersionSpecCollection::ConstPointer
FakeRepository::do_version_specs(const QualifiedPackageName & n) const
{
    if (! has_category_named(n.get<qpn_category>()))
        throw InternalError(PALUDIS_HERE, "no category");
    if (! has_package_named(n.get<qpn_category>(), n.get<qpn_package>()))
        throw InternalError(PALUDIS_HERE, "no package");
    return _implementation->versions.find(n)->second;
}

bool
FakeRepository::do_has_version(const CategoryNamePart & c,
        const PackageNamePart & p, const VersionSpec & v) const
{
    if (! has_category_named(c))
        throw InternalError(PALUDIS_HERE, "no category");
    if (! has_package_named(c, p))
        throw InternalError(PALUDIS_HERE, "no package");
    return _implementation->versions.find(QualifiedPackageName(c, p))->second->find(v) !=
        _implementation->versions.find(QualifiedPackageName(c, p))->second->end();
}

void
FakeRepository::add_category(const CategoryNamePart & c)
{
    _implementation->category_names->insert(c);
    _implementation->package_names.insert(std::make_pair(c, new PackageNamePartCollection));
}

void
FakeRepository::add_package(const CategoryNamePart & c, const PackageNamePart & p)
{
    add_category(c);
    _implementation->package_names.find(c)->second->insert(p);
    _implementation->versions.insert(std::make_pair(QualifiedPackageName(c, p),
                new VersionSpecCollection));
}

VersionMetadata::Pointer
FakeRepository::add_version(const CategoryNamePart & c, const PackageNamePart & p,
        const VersionSpec & v)
{
    add_package(c, p);
    _implementation->versions.find(QualifiedPackageName(c, p))->second->insert(v);
    _implementation->metadata.insert(
            std::make_pair(stringify(c) + "/" + stringify(p) + "-" + stringify(v), new VersionMetadata));
    VersionMetadata::Pointer r(_implementation->metadata.find(stringify(c) +
                "/" + stringify(p) + "-" + stringify(v))->second);
    r->set(vmk_slot, "0");
    r->set(vmk_keywords, "test");
    return r;
}

VersionMetadata::ConstPointer
FakeRepository::do_version_metadata(
        const CategoryNamePart & c, const PackageNamePart & p, const VersionSpec & v) const
{
    if (! has_version(c, p, v))
        throw InternalError(PALUDIS_HERE, "no version");
    return _implementation->metadata.find(stringify(c) + "/" + stringify(p) + "-" + stringify(v))->second;
}

bool
FakeRepository::do_query_repository_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    return false;
}

bool
FakeRepository::do_query_profile_masks(const CategoryNamePart &,
        const PackageNamePart &, const VersionSpec &) const
{
    return false;
}

UseFlagState
FakeRepository::do_query_use(const UseFlagName &) const
{
    return use_unspecified;
}

bool
FakeRepository::do_query_use_mask(const UseFlagName &) const
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
FakeRepository::do_is_licence(const std::string &) const
{
    return false;
}

bool
FakeRepository::do_is_mirror(const std::string &) const
{
    return false;
}

