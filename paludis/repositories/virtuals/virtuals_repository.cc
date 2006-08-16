/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "virtuals_repository.hh"
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/log.hh>
#include "vr_entry.hh"

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VirtualsRepository> :
        InternalCounted<Implementation<VirtualsRepository> >
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

VirtualsRepository::VirtualsRepository(const Environment * const env) :
    Repository(RepositoryName("virtuals"), RepositoryCapabilities::create()
            .installable_interface(this)
            .mask_interface(this)
            .installed_interface(0)
            .use_interface(0)
            .news_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)),
    PrivateImplementationPattern<VirtualsRepository>(
            new Implementation<VirtualsRepository>(env))
{
    RepositoryInfoSection::Pointer config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "virtuals");
    _info->add_section(config_info);
}

VirtualsRepository::~VirtualsRepository()
{
}

void
VirtualsRepository::need_entries() const
{
    if (_imp->has_entries)
        return;

    Context context("When loading entries for virtuals repository:");

    /* Populate our _imp->entries. We need to iterate over each repository in
     * our env's package database, see if it has a virtuals interface, and if it
     * does create an entry for each virtual package. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->virtuals_interface)
            continue;

        RepositoryVirtualsInterface::VirtualsCollection::ConstPointer pp(
                (*r)->virtuals_interface->virtual_packages());

        for (RepositoryVirtualsInterface::VirtualsCollection::Iterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            VersionSpecCollection::ConstPointer vv((*r)->version_specs(p->provided_by_name));

            /* following is debug, not warning, because of overlay style repos */
            if (vv->empty())
                Log::get_instance()->message(ll_debug, lc_context, "No packages matching '"
                        + stringify(p->provided_by_name) + "' for virtual '"
                        + stringify(p->virtual_name) + " ' in repository '"
                        + stringify((*r)->name()) + "'");

            for (VersionSpecCollection::Iterator v(vv->begin()), v_end(vv->end()) ;
                    v != v_end ; ++v)
                _imp->entries.push_back(VREntry::create()
                        .virtual_name(p->virtual_name)
                        .version(*v)
                        .provided_by_name(p->provided_by_name)
                        .provided_by_repository((*r)->name()));
        }
    }

    _imp->has_entries = true;
}

CountedPtr<Repository>
VirtualsRepository::make_virtuals_repository(
        const Environment * const env,
        const PackageDatabase * const,
        AssociativeCollection<std::string, std::string>::ConstPointer)
{
    return CountedPtr<Repository>(new VirtualsRepository(env));
}

bool
VirtualsRepository::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    need_entries();

    /// \todo
    return false;
}

bool
VirtualsRepository::do_query_profile_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    need_entries();

    /// \todo
    return false;
}

VersionMetadata::ConstPointer
VirtualsRepository::do_version_metadata(
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
            p.first->provided_by_repository)->virtuals_interface->virtual_package_version_metadata(
                RepositoryVirtualsEntry::create()
                .virtual_name(p.first->virtual_name)
                .provided_by_name(p.first->provided_by_name), v);
}

bool
VirtualsRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, v, QualifiedPackageName("dummy/package"), RepositoryName("dummy_repository"))));

    return p.first != p.second;
}

VersionSpecCollection::ConstPointer
VirtualsRepository::do_version_specs(const QualifiedPackageName & q) const
{
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
VirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
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
VirtualsRepository::do_category_names() const
{
    need_entries();

    CategoryNamePartCollection::Pointer result(new CategoryNamePartCollection::Concrete);
#if 0
    /// \todo: in theory, this can be a lot lot lot faster
    for (std::vector<VREntry>::const_iterator p(_imp->entries.begin()), p_end(_imp->entries.end()) ;
            p != p_end ; ++p)
        result->insert(p->virtual_name.category);
#else
    fast_unique_copy(_imp->entries.begin(), _imp->entries.end(),
            transform_inserter(result->inserter(), EntriesCategoryExtractor()),
            EntriesCategoryComparator());
#endif

    return result;
}

bool
VirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    return p.first != p.second;
}

bool
VirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(c + PackageNamePart("dummy"), VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesCategoryComparator()));

    return p.first != p.second;
}

bool
VirtualsRepository::do_is_licence(const std::string &) const
{
    return false;
}

void
VirtualsRepository::invalidate() const
{
    _imp->has_entries = false;
    _imp->entries.clear();
}

void
VirtualsRepository::do_install(const QualifiedPackageName &, const VersionSpec &,
        const InstallOptions &) const
{
}

