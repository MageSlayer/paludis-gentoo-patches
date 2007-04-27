/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/match_package.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/log.hh>
#include <paludis/query.hh>
#include <vector>
#include "vr_entry.hh"

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<VirtualsRepository>
    {
        const Environment * const env;

        mutable std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > > names;
        mutable bool has_names;

        mutable std::vector<VREntry> entries;
        mutable bool has_entries;

        Implementation(const Environment * const e) :
            env(e),
            has_names(false),
            has_entries(false)
        {
        }
    };
}

namespace
{
    struct NamesCategoryComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first.category < b.first.category;
        }
    };

    struct NamesNameComparator
    {
        bool
        operator() (const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & a,
                const std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > & b) const
        {
            return a.first < b.first;
        }
    };
}

VirtualsRepository::VirtualsRepository(const Environment * const env) :
    Repository(RepositoryName("virtuals"), RepositoryCapabilities::create()
            .installable_interface(this)
            .mask_interface(this)
            .installed_interface(0)
            .use_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .destination_interface(0)
            .config_interface(0)
            .contents_interface(0)
            .licenses_interface(0)
            .portage_interface(0)
            .hook_interface(0),
            "virtuals"),
    PrivateImplementationPattern<VirtualsRepository>(
            new Implementation<VirtualsRepository>(env))
{
    std::tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "virtuals");
    _info->add_section(config_info);
}

VirtualsRepository::~VirtualsRepository()
{
}

void
VirtualsRepository::need_names() const
{
    if (_imp->has_names)
        return;

    Context context("When loading names for virtuals repository:");

    Log::get_instance()->message(ll_debug, lc_context, "VirtualsRepository need_names");

    /* Determine our virtual name -> package mappings. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->provides_interface)
            continue;

        std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesCollection> provides(
                (*r)->provides_interface->provided_packages());
        for (RepositoryProvidesInterface::ProvidesCollection::Iterator p(provides->begin()),
                p_end(provides->end()) ; p != p_end ; ++p)
            _imp->names.push_back(std::make_pair(p->virtual_name, std::tr1::shared_ptr<const PackageDepSpec>(
                            new PackageDepSpec(
                                std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(p->provided_by_name))))));
    }

    std::sort(_imp->names.begin(), _imp->names.end(), NamesNameComparator());

    std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > > new_names;

    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->virtuals_interface)
            continue;

        std::tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsCollection> virtuals(
                (*r)->virtuals_interface->virtual_packages());
        for (RepositoryVirtualsInterface::VirtualsCollection::Iterator v(virtuals->begin()),
                v_end(virtuals->end()) ; v != v_end ; ++v)
        {
            std::pair<
                std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
                std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
                        std::equal_range(_imp->names.begin(), _imp->names.end(),
                            std::make_pair(v->virtual_name, std::tr1::shared_ptr<const PackageDepSpec>()),
                            NamesNameComparator()));

            if (p.first == p.second)
                new_names.push_back(std::make_pair(v->virtual_name, v->provided_by_spec));
        }
    }

    std::copy(new_names.begin(), new_names.end(), std::back_inserter(_imp->names));
    std::sort(_imp->names.begin(), _imp->names.end(), NamesNameComparator());

    _imp->has_names = true;
}

void
VirtualsRepository::need_entries() const
{
    if (_imp->has_entries)
        return;

    Context context("When loading entries for virtuals repository:");
    need_names();

    Log::get_instance()->message(ll_debug, lc_context, "VirtualsRepository need_entries");

    /* Populate our _imp->entries. */
    for (std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator
            v(_imp->names.begin()), v_end(_imp->names.end()) ; v != v_end ; ++v)
    {
        std::tr1::shared_ptr<const PackageDatabaseEntryCollection> matches(_imp->env->package_database()->query(
                    query::Matches(*v->second) & query::RepositoryHasInstallableInterface(), qo_order_by_version));

        if (matches->empty())
            Log::get_instance()->message(ll_warning, lc_context, "No packages matching '"
                    + stringify(*v->second) + "' for virtual '"
                    + stringify(v->first) + "'");

        for (PackageDatabaseEntryCollection::Iterator m(matches->begin()), m_end(matches->end()) ;
                m != m_end ; ++m)
        {
            _imp->entries.push_back(VREntry::create()
                    .virtual_name(v->first)
                    .version(m->version)
                    .provided_by_name(m->name)
                    .provided_by_repository(m->repository));
        }
    }

    std::sort(_imp->entries.begin(), _imp->entries.end());

    _imp->has_entries = true;
}

std::tr1::shared_ptr<Repository>
VirtualsRepository::make_virtuals_repository(
        Environment * const env,
        std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >)
{
    return std::tr1::shared_ptr<Repository>(new VirtualsRepository(env));
}

bool
VirtualsRepository::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

bool
VirtualsRepository::do_query_profile_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

std::tr1::shared_ptr<const VersionMetadata>
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

    Log::get_instance()->message(ll_debug, lc_no_context, "VirtualsRepository::do_version_metadata("
            + stringify(q) + ", " + stringify(v) + ") lookup using '"
            + stringify(p.first->virtual_name) + "', '" + stringify(p.first->version) + "', '"
            + stringify(p.first->provided_by_name) + "', '" + stringify(p.first->provided_by_repository) + "'");

    const RepositoryVirtualsInterface * const vif(_imp->env->package_database()->fetch_repository(
                p.first->provided_by_repository)->virtuals_interface);
    if (! vif)
        throw InternalError(PALUDIS_HERE, "vif is 0 for do_version_metadata(" + stringify(q) + ", "
                + stringify(v) + ") using (" + stringify(p.first->virtual_name) + ", "
                + stringify(p.first->provided_by_name) + ", " + stringify(p.first->provided_by_repository) + ")");

    return vif->virtual_package_version_metadata(
                RepositoryVirtualsEntry::create()
                .virtual_name(p.first->virtual_name)
                .provided_by_spec(std::tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec(
                            std::tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(p.first->provided_by_name))))),
                v);
}

bool
VirtualsRepository::do_has_version(const QualifiedPackageName & q,
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

std::tr1::shared_ptr<const VersionSpecCollection>
VirtualsRepository::do_version_specs(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return std::tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    std::tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);
    for ( ; p.first != p.second ; ++p.first)
        result->insert(p.first->version);

    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
VirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return std::tr1::shared_ptr<QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    need_names();

    std::pair<
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
            std::equal_range(_imp->names.begin(), _imp->names.end(),
                std::make_pair(c + PackageNamePart("dummy"), std::tr1::shared_ptr<const PackageDepSpec>()),
                NamesCategoryComparator()));

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    std::copy(p.first, p.second, transform_inserter(result->inserter(),
                SelectFirst<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> >()));

    return result;
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
VirtualsRepository::do_category_names() const
{
    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("virtual"));
    return result;
}

bool
VirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return false;

    need_names();

    std::pair<
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator,
        std::vector<std::pair<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator> p(
            std::equal_range(_imp->names.begin(), _imp->names.end(),
                std::make_pair(q, std::tr1::shared_ptr<const PackageDepSpec>()),
                NamesNameComparator()));

    return p.first != p.second;
}

bool
VirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
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

void
VirtualsRepository::invalidate()
{
    _imp.reset(new Implementation<VirtualsRepository>(_imp->env));
}

void
VirtualsRepository::do_install(const QualifiedPackageName &, const VersionSpec &,
        const InstallOptions &) const
{
}

