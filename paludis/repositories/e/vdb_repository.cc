/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/vdb_merger.hh>
#include <paludis/repositories/e/vdb_unmerger.hh>
#include <paludis/repositories/e/vdb_id.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository.hh>

#include <paludis/action.hh>
#include <paludis/util/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_tag.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/set_file.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>
#include <paludis/stringify_formatter.hh>

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/kc.hh>

#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/vdb_repository-sr.cc>

typedef MakeHashedMap<CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::Type CategoryMap;
typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    template <>
    struct Implementation<VDBRepository>
    {
        VDBRepositoryParams params;

        const tr1::shared_ptr<Mutex> big_nasty_mutex;

        mutable CategoryMap categories;
        mutable bool has_category_names;
        mutable IDMap ids;

        mutable tr1::shared_ptr<RepositoryProvidesInterface::ProvidesSequence> provides;
        tr1::shared_ptr<RepositoryNameCache> names_cache;

        Implementation(const VDBRepository * const, const VDBRepositoryParams &, tr1::shared_ptr<Mutex> = make_shared_ptr(new Mutex));
        ~Implementation();

        tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > root_key;
        tr1::shared_ptr<const MetadataValueKey<std::string> > format_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > world_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > provides_cache_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > names_cache_key;
        tr1::shared_ptr<const MetadataValueKey<FSEntry> > builddir_key;
    };

    Implementation<VDBRepository>::Implementation(const VDBRepository * const r,
            const VDBRepositoryParams & p, tr1::shared_ptr<Mutex> m) :
        params(p),
        big_nasty_mutex(m),
        has_category_names(false),
        names_cache(new RepositoryNameCache(p.names_cache, r)),
        location_key(new LiteralMetadataValueKey<FSEntry> ("location", "location",
                    mkt_significant, params.location)),
        root_key(new LiteralMetadataValueKey<FSEntry> ("root", "root",
                    mkt_normal, params.root)),
        format_key(new LiteralMetadataValueKey<std::string> ("format", "format",
                    mkt_significant, "vdb")),
        world_key(new LiteralMetadataValueKey<FSEntry> ("world", "world",
                    mkt_normal, params.world)),
        provides_cache_key(new LiteralMetadataValueKey<FSEntry> ("provides_cache", "provides_cache",
                    mkt_normal, params.provides_cache)),
        names_cache_key(new LiteralMetadataValueKey<FSEntry> ("names_cache", "names_cache",
                    mkt_normal, params.names_cache)),
        builddir_key(new LiteralMetadataValueKey<FSEntry> ("builddir", "builddir",
                    mkt_normal, params.builddir))
    {
    }

    Implementation<VDBRepository>::~Implementation()
    {
    }
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    EInstalledRepository(
            EInstalledRepositoryParams::create()
            .environment(p.environment)
            .root(p.root)
            .builddir(p.builddir)
            .world(p.world),
            p.name,
            RepositoryCapabilities::named_create()
            (k::sets_interface(), this)
            (k::syncable_interface(), static_cast<RepositorySyncableInterface *>(0))
            (k::use_interface(), this)
            (k::world_interface(), this)
            (k::environment_variable_interface(), this)
            (k::mirrors_interface(), static_cast<RepositoryMirrorsInterface *>(0))
            (k::provides_interface(), this)
            (k::virtuals_interface(), static_cast<RepositoryVirtualsInterface *>(0))
            (k::destination_interface(), this)
            (k::e_interface(), static_cast<RepositoryEInterface *>(0))
            (k::make_virtuals_interface(), static_cast<RepositoryMakeVirtualsInterface *>(0))
            (k::qa_interface(), static_cast<RepositoryQAInterface *>(0))
            (k::hook_interface(), this)
            (k::manifest_interface(), static_cast<RepositoryManifestInterface *>(0))),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(this, p)),
    _imp(PrivateImplementationPattern<VDBRepository>::_imp)
{
    _add_metadata_keys();
}

VDBRepository::~VDBRepository()
{
}

void
VDBRepository::_add_metadata_keys() const
{
    clear_metadata_keys();
    add_metadata_key(_imp->location_key);
    add_metadata_key(_imp->root_key);
    add_metadata_key(_imp->format_key);
    add_metadata_key(_imp->world_key);
    add_metadata_key(_imp->provides_cache_key);
    add_metadata_key(_imp->names_cache_key);
    add_metadata_key(_imp->builddir_key);
}

bool
VDBRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _imp->categories.end() != _imp->categories.find(c);
}

bool
VDBRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When checking for package '" + stringify(q) +
            "' in " + stringify(name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->categories.find(q.category));
    if (_imp->categories.end() == cat_iter)
        return false;

    need_package_ids(q.category);

    return cat_iter->second->end() != cat_iter->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

    std::transform(_imp->categories.begin(), _imp->categories.end(), result->inserter(),
            tr1::mem_fn(&std::pair<const CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::first));

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
VDBRepository::package_names(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);

    need_category_names();
    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameSet);

    need_package_ids(c);

    return _imp->categories.find(c)->second;
}

tr1::shared_ptr<const PackageIDSequence>
VDBRepository::package_ids(const QualifiedPackageName & n) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");


    need_category_names();
    if (! has_category_named(n.category))
        return make_shared_ptr(new PackageIDSequence);

    need_package_ids(n.category);
    if (! has_package_named(n))
        return make_shared_ptr(new PackageIDSequence);

    return _imp->ids.find(n)->second;
}

tr1::shared_ptr<Repository>
VDBRepository::make_vdb_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    std::string repo_file(m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second);
    Context context("When making VDB repository from repo_file '" + repo_file + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw VDBRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string root;
    if (m->end() == m->find("root") || ((root = m->find("root")->second)).empty())
        root = "/";

    std::string world;
    if (m->end() == m->find("world") || ((world = m->find("world")->second)).empty())
        world = location + "/world";

    std::string provides_cache;
    if (m->end() == m->find("provides_cache") || ((provides_cache = m->find("provides_cache")->second)).empty())
    {
        provides_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->default_distribution()))[k::default_vdb_provides_cache()];
        if (provides_cache.empty())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "The provides_cache key is not set in '"
                    + repo_file + "'. You should read the Paludis documentation and select an "
                    "appropriate value.");
            provides_cache = "/var/empty";
        }
    }

    std::string names_cache;
    if (m->end() == m->find("names_cache") || ((names_cache = m->find("names_cache")->second)).empty())
    {
        names_cache = (*DistributionData::get_instance()->distribution_from_string(
                env->default_distribution()))[k::default_vdb_names_cache()];
        if (names_cache.empty())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "The names_cache key is not set in '"
                    + repo_file + "'. You should read the Paludis documentation and select an "
                    "appropriate value.");
            names_cache = "/var/empty";
        }
    }

    std::string builddir;
    if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
    {
        if (m->end() == m->find("buildroot") || ((builddir = m->find("buildroot")->second)).empty())
            builddir = (*DistributionData::get_instance()->distribution_from_string(
                    env->default_distribution()))[k::default_ebuild_builddir()];
        else
            Log::get_instance()->message(ll_warning, lc_context) << "Key 'buildroot' is deprecated, use 'builddir' instead";
    }

    std::string name;
    if (m->end() == m->find("name") || ((name = m->find("name")->second)).empty())
        name = "installed";

    return tr1::shared_ptr<Repository>(new VDBRepository(VDBRepositoryParams::create()
                .environment(env)
                .location(location)
                .root(root)
                .world(world)
                .builddir(builddir)
                .provides_cache(provides_cache)
                .name(RepositoryName(name))
                .names_cache(names_cache)));
}

VDBRepositoryConfigurationError::VDBRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository configuration error: " + msg)
{
}

VDBRepositoryKeyReadError::VDBRepositoryKeyReadError(
        const std::string & msg) throw () :
    ConfigurationError("VDB repository key read error: " + msg)
{
}

void
VDBRepository::perform_uninstall(const tr1::shared_ptr<const ERepositoryID> & id,
        const UninstallActionOptions & o, bool reinstalling) const
{
    Context context("When uninstalling '" + stringify(*id) + (reinstalling ? "' for a reinstall:" : "':"));

    if (! _imp->params.root.is_directory())
        throw InstallActionError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

    std::string reinstalling_str(reinstalling ? "-reinstalling-" : "");

    tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(FSEntry(_imp->params.location / stringify(id->name().category) /
                (reinstalling_str + stringify(id->name().package) + "-" + stringify(id->version()))));

    FSEntry pkg_dir(_imp->params.location / stringify(id->name().category) / (reinstalling_str +
                stringify(id->name().package) + "-" + stringify(id->version())));

    tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));

    EAPIPhases phases((*(*id->eapi())[k::supported()])[k::ebuild_phases()].ebuild_uninstall);
    for (EAPIPhases::ConstIterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        if (phase->option("unmerge"))
        {
            /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb, supplement with env */
            std::string config_protect, config_protect_mask;
            {
                std::ifstream c(stringify(pkg_dir / "CONFIG_PROTECT").c_str());
                config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT", "");

                std::ifstream c_m(stringify(pkg_dir / "CONFIG_PROTECT_MASK").c_str());
                config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>()) +
                    " " + getenv_with_default("CONFIG_PROTECT_MASK", "");
            }

            /* unmerge */
            VDBUnmerger unmerger(
                    VDBUnmergerOptions::named_create()
                    (k::environment(), _imp->params.environment)
                    (k::root(), installed_root_key()->value())
                    (k::contents_file(), pkg_dir / "CONTENTS")
                    (k::config_protect(), config_protect)
                    (k::config_protect_mask(), config_protect_mask)
                    (k::package_id(), id));

            unmerger.unmerge();
        }
        else
        {
            EbuildCommandParams params(EbuildCommandParams::named_create()
                    (k::environment(), _imp->params.environment)
                    (k::package_id(), id)
                    (k::ebuild_dir(), pkg_dir)
                    (k::ebuild_file(), pkg_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild"))
                    (k::files_dir(), pkg_dir)
                    (k::eclassdirs(), eclassdirs)
                    (k::exlibsdirs(), make_shared_ptr(new FSEntrySequence))
                    (k::portdir(), _imp->params.location)
                    (k::distdir(), pkg_dir)
                    (k::sandbox(), phase->option("sandbox"))
                    (k::userpriv(), phase->option("userpriv"))
                    (k::commands(), join(phase->begin_commands(), phase->end_commands(), " "))
                    (k::builddir(), _imp->params.builddir));

            EbuildUninstallCommandParams uninstall_params(EbuildUninstallCommandParams::named_create()
                    (k::root(), stringify(_imp->params.root))
                    (k::disable_cfgpro(), o[k::no_config_protect()])
                    (k::unmerge_only(), false)
                    (k::loadsaveenv_dir(), pkg_dir)
                    (k::load_environment(), load_env.get()));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    /* remove vdb entry */
    for (DirIterator d(pkg_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    pkg_dir.rmdir();
}

void
VDBRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);
    _imp.reset(new Implementation<VDBRepository>(this, _imp->params, _imp->big_nasty_mutex));
    _add_metadata_keys();
}

void
VDBRepository::invalidate_masks()
{
}

tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence>
VDBRepository::provided_packages() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->provides)
        return _imp->provides;

    if (! load_provided_using_cache())
        load_provided_the_slow_way();

    return _imp->provides;
}

bool
VDBRepository::load_provided_using_cache() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->params.provides_cache == FSEntry("/var/empty"))
        return false;

    Context context("When loading VDB PROVIDEs map using '" + stringify(_imp->params.provides_cache) + "':");

    tr1::shared_ptr<ProvidesSequence> result(new ProvidesSequence);

    if (! _imp->params.provides_cache.is_regular_file())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Provides cache at '"
                + stringify(_imp->params.provides_cache) + "' is not a regular file.");
        return false;
    }

    std::ifstream provides_cache(stringify(_imp->params.provides_cache).c_str());

    std::string version;
    std::getline(provides_cache, version);

    if (version != "paludis-2")
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Can't use provides cache at '"
                + stringify(_imp->params.provides_cache) + "' because format '" + version + "' is not 'paludis-2'");
        return false;
    }

    std::string for_name;
    std::getline(provides_cache, for_name);
    if (for_name != stringify(name()))
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "Can't use provides cache at '"
                + stringify(_imp->params.provides_cache) + "' because it was generated for repository '"
                + for_name + "'. You must not have multiple name caches at the same location.");
        return false;
    }

    std::string line;
    while (std::getline(provides_cache, line))
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(line, std::back_inserter(tokens));
        if (tokens.size() < 3)
            continue;

        QualifiedPackageName q(tokens.at(0));
        VersionSpec v(tokens.at(1));

        tr1::shared_ptr<const PackageID> id(package_id_if_exists(q, v));
        if (! id)
        {
            Log::get_instance()->message(ll_warning, lc_context) << "No package available for line '"
                << line << "'";
            continue;
        }

        DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->params.environment);
        tr1::shared_ptr<ProvideSpecTree::ConstItem> pp(parse_provide(
                    join(next(next(tokens.begin())), tokens.end(), " "),
                    _imp->params.environment, id,
                    *EAPIData::get_instance()->eapi_from_string("paludis-1")));
        pp->accept(f);

        for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
            result->push_back(RepositoryProvidesEntry::named_create()
                    (k::virtual_name(), QualifiedPackageName((*p)->text()))
                    (k::provided_by(), id));
    }

    _imp->provides = result;
    return true;
}

void
VDBRepository::load_provided_the_slow_way() const
{
    Lock l(*_imp->big_nasty_mutex);

    using namespace tr1::placeholders;

    Context context("When loading VDB PROVIDEs map the slow way:");

    Log::get_instance()->message(ll_debug, lc_no_context, "Starting VDB PROVIDEs map creation");

    tr1::shared_ptr<ProvidesSequence> result(new ProvidesSequence);

    need_category_names();
    std::for_each(_imp->categories.begin(), _imp->categories.end(),
            tr1::bind(tr1::mem_fn(&VDBRepository::need_package_ids), this,
                tr1::bind<CategoryNamePart>(tr1::mem_fn(
                        &std::pair<const CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::first), _1)));


    for (IDMap::const_iterator i(_imp->ids.begin()), i_end(_imp->ids.end()) ;
            i != i_end ; ++i)
    {
        for (PackageIDSequence::ConstIterator e(i->second->begin()), e_end(i->second->end()) ;
                e != e_end ; ++e)
        {
            Context loop_context("When loading VDB PROVIDEs entry for '" + stringify(**e) + "':");

            try
            {
                if (! (*e)->provide_key())
                    continue;

                tr1::shared_ptr<const ProvideSpecTree::ConstItem> provide((*e)->provide_key()->value());;
                DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->params.environment);
                provide->accept(f);

                for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator
                        p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
                {
                    QualifiedPackageName pp((*p)->text());

                    if (pp.category != CategoryNamePart("virtual"))
                        Log::get_instance()->message(ll_warning, lc_no_context, "PROVIDE of non-virtual '"
                                + stringify(pp) + "' from '" + stringify(**e) + "' will not work as expected");

                    result->push_back(RepositoryProvidesEntry::named_create()
                            (k::virtual_name(), pp)
                            (k::provided_by(), *e));
                }
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & ee)
            {
                Log::get_instance()->message(ll_warning, lc_no_context, "Skipping VDB PROVIDE entry for '"
                        + stringify(**e) + "' due to exception '"
                        + stringify(ee.message()) + "' (" + stringify(ee.what()) + ")");
            }
        }
    }

    Log::get_instance()->message(ll_debug, lc_no_context) << "Done VDB PROVIDEs map creation";

    _imp->provides = result;
}

void
VDBRepository::regenerate_cache() const
{
    Lock l(*_imp->big_nasty_mutex);

    regenerate_provides_cache();
    _imp->names_cache->regenerate_cache();
}

void
VDBRepository::regenerate_provides_cache() const
{
    Lock l(*_imp->big_nasty_mutex);

    using namespace tr1::placeholders;

    if (_imp->params.provides_cache == FSEntry("/var/empty"))
        return;

    Context context("When generating VDB repository provides cache at '"
            + stringify(_imp->params.provides_cache) + "':");

    FSEntry(_imp->params.provides_cache).unlink();
    _imp->params.provides_cache.dirname().mkdir();

    need_category_names();
    std::for_each(_imp->categories.begin(), _imp->categories.end(),
            tr1::bind(tr1::mem_fn(&VDBRepository::need_package_ids), this,
                tr1::bind<CategoryNamePart>(tr1::mem_fn(
                        &std::pair<const CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::first), _1)));

    std::ofstream f(stringify(_imp->params.provides_cache).c_str());
    if (! f)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Cannot write to '" <<
                _imp->params.provides_cache << "'";
        return;
    }

    f << "paludis-2" << std::endl;
    f << name() << std::endl;

    for (IDMap::const_iterator i(_imp->ids.begin()), i_end(_imp->ids.end()) ;
            i != i_end ; ++i)
    {
        for (PackageIDSequence::ConstIterator e(i->second->begin()), e_end(i->second->end()) ;
                e != e_end ; ++e)
        {
            if (! (*e)->provide_key())
                continue;

            tr1::shared_ptr<const ProvideSpecTree::ConstItem> provide((*e)->provide_key()->value());
            StringifyFormatter ff;
            DepSpecPrettyPrinter p(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            provide->accept(p);
            std::string provide_str(strip_leading(strip_trailing(stringify(p), " \t\r\n"), " \t\r\n"));
            if (provide_str.empty())
                continue;

            f << (*e)->name() << " " << (*e)->version() << " " << provide_str << std::endl;
        }
    }
}

tr1::shared_ptr<const CategoryNamePartSet>
VDBRepository::category_names_containing_package(const PackageNamePart & p) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! _imp->names_cache->usable())
        return Repository::category_names_containing_package(p);

    tr1::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::category_names_containing_package(p);
}

void
VDBRepository::merge(const MergeParams & m)
{
    Context context("When merging '" + stringify(*m[k::package_id()]) + "' at '" + stringify(m[k::image_dir()])
            + "' to VDB repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m[k::package_id()]))
        throw InstallActionError("Not a suitable destination for '" + stringify(*m[k::package_id()]) + "'");

    tr1::shared_ptr<const ERepositoryID> is_replace(package_id_if_exists(m[k::package_id()]->name(), m[k::package_id()]->version()));

    FSEntry tmp_vdb_dir(_imp->params.location);
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= stringify(m[k::package_id()]->name().category);
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= ("-checking-" + stringify(m[k::package_id()]->name().package) + "-" + stringify(m[k::package_id()]->version()));
    tmp_vdb_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            WriteVDBEntryParams::named_create()
            (k::environment(), _imp->params.environment)
            (k::package_id(), tr1::static_pointer_cast<const ERepositoryID>(m[k::package_id()]))
            (k::output_directory(), tmp_vdb_dir)
            (k::environment_file(), m[k::environment_file()]));

    write_vdb_entry_command();

    /* load CONFIG_PROTECT, CONFIG_PROTECT_MASK from vdb */
    std::string config_protect, config_protect_mask;
    {
        std::ifstream c(stringify(tmp_vdb_dir / "CONFIG_PROTECT").c_str());
        config_protect = std::string((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());

        std::ifstream c_m(stringify(tmp_vdb_dir / "CONFIG_PROTECT_MASK").c_str());
        config_protect_mask = std::string((std::istreambuf_iterator<char>(c_m)), std::istreambuf_iterator<char>());
    }

    FSEntry vdb_dir(_imp->params.location);
    vdb_dir /= stringify(m[k::package_id()]->name().category);
    vdb_dir /= (stringify(m[k::package_id()]->name().package) + "-" + stringify(m[k::package_id()]->version()));

    VDBMerger merger(
            VDBMergerParams::named_create()
            (k::environment(), _imp->params.environment)
            (k::image(), m[k::image_dir()])
            (k::root(), installed_root_key()->value())
            (k::contents_file(), vdb_dir / "CONTENTS")
            (k::config_protect(), config_protect)
            (k::config_protect_mask(), config_protect_mask)
            (k::package_id(), m[k::package_id()])
            (k::options(), m[k::options()]));

    if (! merger.check())
    {
        for (DirIterator d(tmp_vdb_dir, DirIteratorOptions() + dio_include_dotfiles), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        tmp_vdb_dir.rmdir();
        throw InstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    if (is_replace)
    {
        if ((vdb_dir.dirname() / ("-reinstalling-" + vdb_dir.basename())).exists())
            throw InstallActionError("Directory '" + stringify(vdb_dir.dirname() /
                        ("-reinstalling-" + vdb_dir.basename())) + "' already exists, probably due to "
                    "a previous failed upgrade. If it is safe to do so, remove this directory and try "
                    "again.");
        vdb_dir.rename(vdb_dir.dirname() / ("-reinstalling-" + vdb_dir.basename()));
    }

    tmp_vdb_dir.rename(vdb_dir);

    merger.merge();

    if (is_replace)
    {
        UninstallActionOptions uninstall_options(false);
        perform_uninstall(is_replace, uninstall_options, true);
    }

    VDBPostMergeCommand post_merge_command(
            VDBPostMergeCommandParams::named_create()
            (k::root(), installed_root_key()->value()));

    post_merge_command();
}

void
VDBRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names from '" + stringify(_imp->params.location) + "':");

    for (DirIterator d(_imp->params.location, DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        try
        {
            if (d->is_directory_or_symlink_to_directory())
                _imp->categories.insert(std::make_pair(CategoryNamePart(d->basename()),
                            tr1::shared_ptr<QualifiedPackageNameSet>()));
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context) << "Skipping VDB category dir '"
                << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }

    _imp->has_category_names = true;
}

void
VDBRepository::need_package_ids(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->categories[c])
        return;

    Context context("When loading package names from '" + stringify(_imp->params.location) +
            "' in category '" + stringify(c) + "':");

    tr1::shared_ptr<QualifiedPackageNameSet> q(new QualifiedPackageNameSet);

    for (DirIterator d(_imp->params.location / stringify(c), DirIteratorOptions() + dio_inode_sort), d_end ; d != d_end ; ++d)
        try
        {
            if (d->is_directory_or_symlink_to_directory())
            {
                std::string s(d->basename());
                if (std::string::npos == s.rfind('-'))
                    continue;

                PackageDepSpec p(parse_user_package_dep_spec("=" + stringify(c) + "/" + s, UserPackageDepSpecOptions()));
                q->insert(*p.package_ptr());
                IDMap::iterator i(_imp->ids.find(*p.package_ptr()));
                if (_imp->ids.end() == i)
                    i = _imp->ids.insert(std::make_pair(*p.package_ptr(), make_shared_ptr(new PackageIDSequence))).first;
                i->second->push_back(make_id(*p.package_ptr(), p.version_requirements_ptr()->begin()->version_spec, *d));
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context) << "Skipping VDB package dir '"
                << *d << "' due to exception '" << e.message() << "' (" << e.what() << ")";
        }

    _imp->categories[c] = q;
}

const tr1::shared_ptr<const ERepositoryID>
VDBRepository::make_id(const QualifiedPackageName & q, const VersionSpec & v, const FSEntry & f) const
{
    Lock l(*_imp->big_nasty_mutex);

    Context context("When creating ID for '" + stringify(q) + "-" + stringify(v) + "' from '" + stringify(f) + "':");

    tr1::shared_ptr<VDBID> result(new VDBID(q, v, _imp->params.environment, shared_from_this(), f));
    return result;
}

const tr1::shared_ptr<const ERepositoryID>
VDBRepository::package_id_if_exists(const QualifiedPackageName & q, const VersionSpec & v) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return tr1::shared_ptr<const ERepositoryID>();

    need_package_ids(q.category);

    using namespace tr1::placeholders;

    PackageIDSequence::ConstIterator i(_imp->ids[q]->begin()), i_end(_imp->ids[q]->end());
    for ( ; i != i_end ; ++i)
        if (v == (*i)->version())
            return tr1::static_pointer_cast<const ERepositoryID>(*i);
    return tr1::shared_ptr<const ERepositoryID>();
}

void
VDBRepository::need_keys_added() const
{
}

const tr1::shared_ptr<const MetadataValueKey<std::string> >
VDBRepository::format_key() const
{
    return _imp->format_key;
}

const tr1::shared_ptr<const MetadataValueKey<FSEntry> >
VDBRepository::installed_root_key() const
{
    return _imp->root_key;
}

