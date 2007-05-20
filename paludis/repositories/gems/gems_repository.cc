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

#include "gems_repository.hh"

using namespace paludis;

#include <paludis/repositories/gems/gems_repository-sr.cc>
#include <paludis/repositories/gems/gems_version_metadata.hh>
#include <paludis/repositories/gems/cache.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/syncer.hh>
#include <paludis/package_database.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/environment.hh>

#include <paludis/portage_dep_parser.hh>

namespace paludis
{
    typedef std::map<VersionSpec, GemsRepositoryEntry> Versions;
    typedef MakeHashedMap<PackageNamePart, Versions>::Type Packages;

    template<>
    struct Implementation<GemsRepository>
    {
        const GemsRepositoryParams params;

        void need_entries() const;

        mutable bool has_entries;
        mutable Packages packages;

        Implementation(const GemsRepositoryParams & p) :
            params(p),
            has_entries(false)
        {
        }
    };

    void
    Implementation<GemsRepository>::need_entries() const
    {
        if (has_entries)
            return;

        Context context("When loading Gems repository entries:");

        GemsCache cache(params.location / "yaml");
        for (GemsCache::Iterator g(cache.begin()), g_end(cache.end()) ;
                g != g_end ; ++g)
        {
            tr1::shared_ptr<VersionMetadata> m(new GemsVersionMetadata(g->version));
            m->set_homepage(g->homepage);
            if (g->description.empty())
                m->description = g->summary;
            else
                m->description = g->description;

            Packages::iterator v(packages.insert(std::make_pair(g->name, Versions())).first);
            v->second.insert(std::make_pair(g->version, m));
        }

        has_entries = true;
    }
}

bool
GemsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return stringify(c) == "gems";
}

bool
GemsRepository::do_has_package_named(const QualifiedPackageName & c) const
{
    if (! do_has_category_named(c.category))
        return false;

    _imp->need_entries();
    return _imp->packages.end() != _imp->packages.find(c.package);

    return false;
}

tr1::shared_ptr<const CategoryNamePartCollection>
GemsRepository::do_category_names() const
{
    static tr1::shared_ptr<CategoryNamePartCollection> names(new CategoryNamePartCollection::Concrete);
    if (names->empty())
        names->insert(CategoryNamePart("gems"));

    return names;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
GemsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (! has_category_named(c))
        return tr1::shared_ptr<const QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    _imp->need_entries();

    tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    for (Packages::const_iterator i(_imp->packages.begin()), i_end(_imp->packages.end()) ;
            i != i_end ; ++i)
        result->insert(c + i->first);

    return result;
}

tr1::shared_ptr<const VersionSpecCollection>
GemsRepository::do_version_specs(const QualifiedPackageName & p) const
{
    if (! has_category_named(p.category))
        return tr1::shared_ptr<const VersionSpecCollection>(new VersionSpecCollection::Concrete);

    _imp->need_entries();

    tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);
    Packages::const_iterator i(_imp->packages.find(p.package));
    if (i != _imp->packages.end())
        std::copy(i->second.begin(), i->second.end(), transform_inserter(
                    result->inserter(), SelectFirst<VersionSpec, GemsRepositoryEntry>()));

    return result;
}

bool
GemsRepository::do_has_version(const QualifiedPackageName & q, const VersionSpec &) const
{
    if (! has_category_named(q.category))
        return false;

    _imp->need_entries();

    return false;
}

tr1::shared_ptr<const VersionMetadata>
GemsRepository::do_version_metadata(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! has_category_named(q.category))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    _imp->need_entries();

    Packages::const_iterator i(_imp->packages.find(q.package));
    if (i == _imp->packages.end())
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    Versions::const_iterator j(i->second.find(v));
    if (j == i->second.end())
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    return j->second.metadata;
}

void
GemsRepository::do_install(const QualifiedPackageName &, const VersionSpec &, const InstallOptions &) const
{
}

tr1::shared_ptr<DepSpec>
GemsRepository::do_package_set(const SetName &) const
{
    return tr1::shared_ptr<DepSpec>();
}

tr1::shared_ptr<const SetNameCollection>
GemsRepository::sets_list() const
{
    return tr1::shared_ptr<SetNameCollection>(new SetNameCollection::Concrete);
}

bool
GemsRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.yaml_uri.empty())
        return false;

    std::string::size_type p;
    if (std::string::npos == ((p = _imp->params.yaml_uri.find(':'))))
        throw ConfigurationError("Don't recognise URI '" + _imp->params.yaml_uri + "'");

    std::string protocol(_imp->params.yaml_uri.substr(0, p));

    Log::get_instance()->message(ll_debug, lc_context) << "looking for syncer protocol '"
            + stringify(protocol) + "'";

    FSEntry fetcher("/var/empty");
    bool ok(false);
    tr1::shared_ptr<const FSEntryCollection> fetchers_dirs(_imp->params.environment->fetchers_dirs());
    for (FSEntryCollection::Iterator d(fetchers_dirs->begin()),
            d_end(fetchers_dirs->end()) ; d != d_end && ! ok; ++d)
    {
        fetcher = FSEntry(*d) / ("do" + protocol);
        if (fetcher.exists() && fetcher.has_permission(fs_ug_owner, fs_perm_execute))
            ok = true;

        Log::get_instance()->message(ll_debug, lc_no_context, "Trying '" + stringify(fetcher) + "': "
                + (ok ? "ok" : "not ok"));
    }

    if (! ok)
        throw ConfigurationError("Don't know how to fetch URI '" + _imp->params.yaml_uri + "'");

    (_imp->params.location / "yaml").unlink();
    if (run_command(stringify(fetcher) + " '" +_imp->params.yaml_uri + "' '" +
                stringify(_imp->params.location / "yaml") + "'"))
        throw SyncFailedError(stringify(_imp->params.location / "yaml"), _imp->params.yaml_uri);

    return true;
}

GemsRepository::GemsRepository(const GemsRepositoryParams & p) :
    Repository(RepositoryName("gems"),
            RepositoryCapabilities::create()
            .mask_interface(0)
            .contents_interface(0)
            .installable_interface(this)
            .installed_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .config_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .portage_interface(0)
            .pretend_interface(0)
            .hook_interface(0),
            "gems"),
    PrivateImplementationPattern<GemsRepository>(new Implementation<GemsRepository>(p))
{
}

GemsRepository::~GemsRepository()
{
}

void
GemsRepository::invalidate()
{
    _imp.reset(new Implementation<GemsRepository>(_imp->params));
}

void
GemsRepository::regenerate_cache() const
{
}

tr1::shared_ptr<const RepositoryInfo>
GemsRepository::info(bool) const
{
    return tr1::shared_ptr<RepositoryInfo>(new RepositoryInfo);
}

