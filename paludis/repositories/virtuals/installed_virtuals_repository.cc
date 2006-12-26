/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/repositories/virtuals/vr_entry.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/collection_concrete.hh>
#include <vector>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<InstalledVirtualsRepository> :
        InternalCounted<Implementation<InstalledVirtualsRepository> >
    {
        const Environment * const env;

        mutable std::vector<VREntry> entries;
        mutable bool has_entries;

        Implementation(const Environment * const e) :
            env(e),
            has_entries(false)
        {
        }
    };

}

InstalledVirtualsRepository::InstalledVirtualsRepository(const Environment * const env) :
    Repository(RepositoryName("installed_virtuals"), RepositoryCapabilities::create()
            .installable_interface(0)
            .mask_interface(this)
            .installed_interface(this)
            .use_interface(0)
            .news_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(this)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .destination_interface(0),
            "installed_virtuals"),
    PrivateImplementationPattern<InstalledVirtualsRepository>(
            new Implementation<InstalledVirtualsRepository>(env))
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "installed_virtuals");
    _info->add_section(config_info);
}

InstalledVirtualsRepository::~InstalledVirtualsRepository()
{
}

void
InstalledVirtualsRepository::need_entries() const
{
    if (_imp->has_entries)
        return;

    /* Populate our _imp->entries. We need to iterate over each repository in
     * our env's package database, see if it has a provides interface, and if it
     * does create an entry for each provided package. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->provides_interface)
            continue;

        RepositoryProvidesInterface::ProvidesCollection::ConstPointer pp(
                (*r)->provides_interface->provided_packages());

        for (RepositoryProvidesInterface::ProvidesCollection::Iterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            _imp->entries.push_back(VREntry::create()
                    .virtual_name(p->virtual_name)
                    .version(p->version)
                    .provided_by_name(p->provided_by_name)
                    .provided_by_repository((*r)->name()));
        }
    }

    /* Our entries must be sorted, for fast lookups. But it's a bit trickier
     * than a straight operator< comparison, because the same virtual version
     * can be provided by more than one thing. For ease of coding elsewhere, we
     * want the 'best' entry to come earlier (sort lower). We define best as
     * being from the most important repository, and failing that from the
     * greater provided name (the second part purely for consistency between
     * invokations). */
    std::sort(_imp->entries.begin(), _imp->entries.end(),
            EntriesComparator(_imp->env->package_database()));

    _imp->has_entries = true;
}

CountedPtr<Repository>
InstalledVirtualsRepository::make_installed_virtuals_repository(
        const Environment * const env,
        const PackageDatabase * const,
        AssociativeCollection<std::string, std::string>::ConstPointer)
{
    return CountedPtr<Repository>(new InstalledVirtualsRepository(env));
}

Contents::ConstPointer
InstalledVirtualsRepository::do_contents(
        const QualifiedPackageName &, const VersionSpec &) const
{
    /* virtual packages don't have any genuine contents. don't return the
     * content of our real package -- that'll cause extreme confusion with
     * paludis --owner. */

    return Contents::ConstPointer(new Contents);
}

bool
InstalledVirtualsRepository::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

bool
InstalledVirtualsRepository::do_query_profile_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

VersionMetadata::ConstPointer
InstalledVirtualsRepository::do_version_metadata(
        const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, v, QualifiedPackageName("dummy/package"), RepositoryName("dummy_repository"))));

    if (p.first == p.second)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    return _imp->env->package_database()->fetch_repository(
            p.first->provided_by_repository)->provides_interface->provided_package_version_metadata(
                RepositoryProvidesEntry::create()
                .virtual_name(p.first->virtual_name)
                .version(p.first->version)
                .provided_by_name(p.first->provided_by_name));
}

bool
InstalledVirtualsRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    if (q.category.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, v, QualifiedPackageName("dummy/package"), RepositoryName("dummy_repository"))));

    return p.first != p.second;
}

VersionSpecCollection::ConstPointer
InstalledVirtualsRepository::do_version_specs(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return VersionSpecCollection::Pointer(new VersionSpecCollection::Concrete);

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    VersionSpecCollection::Pointer result(new VersionSpecCollection::Concrete);
    for ( ; p.first != p.second ; ++p.first)
        result->insert(p.first->version);

    return result;
}

QualifiedPackageNameCollection::ConstPointer
InstalledVirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return QualifiedPackageNameCollection::Pointer(new QualifiedPackageNameCollection::Concrete);

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(c + PackageNamePart("dummy"), VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesCategoryComparator()));


    QualifiedPackageNameCollection::Pointer result(new QualifiedPackageNameCollection::Concrete);
#if 0
    /// \todo: in theory, this can be a lot faster
    for ( ; p.first != p.second ; ++p.first)
        result->insert(p.first->virtual_name);
#else
    fast_unique_copy(p.first, p.second,
            transform_inserter(result->inserter(), EntriesNameExtractor()),
            EntriesNameComparator());
#endif

    return result;
}

CategoryNamePartCollection::ConstPointer
InstalledVirtualsRepository::do_category_names() const
{
    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("virtual"));

#if 0
    need_entries();

    fast_unique_copy(_imp->entries.begin(), _imp->entries.end(),
            transform_inserter(result->inserter(), EntriesCategoryExtractor()),
            EntriesCategoryComparator());
#endif

    return result;
}

bool
InstalledVirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    return p.first != p.second;
}

bool
InstalledVirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(c + PackageNamePart("dummy"), VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesCategoryComparator()));

    return p.first != p.second;
}

bool
InstalledVirtualsRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
InstalledVirtualsRepository::invalidate() const
{
    _imp->has_entries = false;
    _imp->entries.clear();
}

void
InstalledVirtualsRepository::do_uninstall(const QualifiedPackageName &, const VersionSpec &,
        const InstallOptions &) const
{
}

