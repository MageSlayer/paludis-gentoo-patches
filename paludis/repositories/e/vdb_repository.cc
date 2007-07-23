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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/vdb_merger.hh>
#include <paludis/repositories/e/vdb_unmerger.hh>
#include <paludis/repositories/e/vdb_id.hh>
#include <paludis/repositories/e/eapi_phase.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>

#include <paludis/action.hh>
#include <paludis/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/dep_tag.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/hook.hh>
#include <paludis/match_package.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repository_info.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/set_file.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_requirements.hh>

#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>

/** \file
 * Implementation for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/vdb_repository-sr.cc>

typedef MakeHashedMap<CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::Type CategoryMap;
typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    /**
     * Implementation data for VDBRepository.
     *
     * \ingroup grpvdbrepository
     */
    template <>
    struct Implementation<VDBRepository>
    {
        VDBRepositoryParams params;

        mutable Mutex big_nasty_mutex;

        mutable CategoryMap categories;
        mutable bool has_category_names;
        mutable IDMap ids;

        mutable tr1::shared_ptr<RepositoryProvidesInterface::ProvidesSequence> provides;
        tr1::shared_ptr<RepositoryNameCache> names_cache;

        Implementation(const VDBRepository * const, const VDBRepositoryParams &);
        ~Implementation();
    };

    Implementation<VDBRepository>::Implementation(const VDBRepository * const r,
            const VDBRepositoryParams & p) :
        params(p),
        has_category_names(false),
        names_cache(new RepositoryNameCache(p.names_cache, r))
    {
    }

    Implementation<VDBRepository>::~Implementation()
    {
    }
}

VDBRepository::VDBRepository(const VDBRepositoryParams & p) :
    Repository(RepositoryName("installed"),
            RepositoryCapabilities::create()
            .installed_interface(this)
            .sets_interface(this)
            .syncable_interface(0)
            .use_interface(this)
            .world_interface(this)
            .environment_variable_interface(this)
            .mirrors_interface(0)
            .provides_interface(this)
            .virtuals_interface(0)
            .destination_interface(this)
            .licenses_interface(0)
            .e_interface(0)
            .make_virtuals_interface(0)
            .qa_interface(0)
            .hook_interface(this)
            .manifest_interface(0),
            "vdb"),
    PrivateImplementationPattern<VDBRepository>(new Implementation<VDBRepository>(this, p))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("root", stringify(_imp->params.root));
    config_info->add_kv("format", "vdb");
    config_info->add_kv("world", stringify(_imp->params.world));
    config_info->add_kv("provides_cache", stringify(_imp->params.provides_cache));
    config_info->add_kv("names_cache", stringify(_imp->params.names_cache));
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));

    _info->add_section(config_info);
}

VDBRepository::~VDBRepository()
{
}

bool
VDBRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _imp->categories.end() != _imp->categories.find(c);
}

bool
VDBRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Lock l(_imp->big_nasty_mutex);

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
VDBRepository::do_category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);

    std::copy(_imp->categories.begin(), _imp->categories.end(),
            transform_inserter(result->inserter(),
                tr1::mem_fn(&std::pair<const CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::first)));

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
VDBRepository::do_package_names(const CategoryNamePart & c) const
{
    Lock l(_imp->big_nasty_mutex);

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
VDBRepository::do_package_ids(const QualifiedPackageName & n) const
{
    Lock l(_imp->big_nasty_mutex);

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

UseFlagState
VDBRepository::do_query_use(const UseFlagName & f, const PackageID & e) const
{
    Lock l(_imp->big_nasty_mutex);

    if (! e.use_key())
        return use_unspecified;

    if (e.use_key()->value()->end() != e.use_key()->value()->find(f))
        return use_enabled;
    else
        return use_disabled;
}

bool
VDBRepository::do_query_use_mask(const UseFlagName & u, const PackageID & e) const
{
    return use_disabled == do_query_use(u, e);
}

bool
VDBRepository::do_query_use_force(const UseFlagName & u, const PackageID & e) const
{
    return use_enabled == do_query_use(u, e);
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
        Log::get_instance()->message(ll_warning, lc_no_context, "The provides_cache key is not set in '"
                + repo_file + "'. You should read http://paludis.pioto.org/cachefiles.html and select an "
                "appropriate value.");
        provides_cache = "/var/empty";
    }

    std::string names_cache;
    if (m->end() == m->find("names_cache") || ((names_cache = m->find("names_cache")->second)).empty())
    {
        Log::get_instance()->message(ll_warning, lc_no_context, "The names_cache key is not set in '"
                + repo_file + "'. You should read http://paludis.pioto.org/cachefiles.html and select an "
                "appropriate value.");
        names_cache = "/var/empty";
    }

    std::string buildroot;
    if (m->end() == m->find("buildroot") || ((buildroot = m->find("buildroot")->second)).empty())
        buildroot = "/var/tmp/paludis";

    return tr1::shared_ptr<Repository>(new VDBRepository(VDBRepositoryParams::create()
                .environment(env)
                .location(location)
                .root(root)
                .world(world)
                .buildroot(buildroot)
                .provides_cache(provides_cache)
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
        throw PackageInstallActionError("Couldn't uninstall '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

    std::string reinstalling_str(reinstalling ? "-reinstalling-" : "");

    tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(FSEntry(_imp->params.location / stringify(id->name().category) /
                (reinstalling_str + stringify(id->name().package) + "-" + stringify(id->version()))));

    FSEntry pkg_dir(_imp->params.location / stringify(id->name().category) / (reinstalling_str +
                stringify(id->name().package) + "-" + stringify(id->version())));

    tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));

    EAPIPhases phases(id->eapi()->supported->ebuild_phases->ebuild_uninstall);
    for (EAPIPhases::Iterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
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
                    VDBUnmergerOptions::create()
                    .environment(_imp->params.environment)
                    .root(root())
                    .contents_file(pkg_dir / "CONTENTS")
                    .config_protect(config_protect)
                    .config_protect_mask(config_protect_mask)
                    .package_id(id));

            unmerger.unmerge();
        }
        else
        {
            EbuildCommandParams params(EbuildCommandParams::create()
                    .environment(_imp->params.environment)
                    .package_id(id)
                    .ebuild_dir(pkg_dir)
                    .ebuild_file(pkg_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild"))
                    .files_dir(pkg_dir)
                    .eclassdirs(eclassdirs)
                    .exlibsdirs(make_shared_ptr(new FSEntrySequence))
                    .portdir(_imp->params.location)
                    .distdir(pkg_dir)
                    .sandbox(phase->option("sandbox"))
                    .userpriv(phase->option("userpriv"))
                    .commands(join(phase->begin_commands(), phase->end_commands(), " "))
                    .buildroot(_imp->params.buildroot));

            EbuildUninstallCommandParams uninstall_params(EbuildUninstallCommandParams::create()
                    .root(stringify(_imp->params.root) + "/")
                    .disable_cfgpro(o.no_config_protect)
                    .unmerge_only(false)
                    .loadsaveenv_dir(pkg_dir)
                    .load_environment(load_env.get()));

            EbuildUninstallCommand uninstall_cmd_pre(params, uninstall_params);
            uninstall_cmd_pre();
        }
    }

    /* remove vdb entry */
    for (DirIterator d(pkg_dir, false), d_end ; d != d_end ; ++d)
        FSEntry(*d).unlink();
    pkg_dir.rmdir();
}

void
VDBRepository::perform_config(const tr1::shared_ptr<const ERepositoryID> & id) const
{
    Context context("When configuring '" + stringify(*id) + "':");

    if (! _imp->params.root.is_directory())
        throw PackageInstallActionError("Couldn't configure '" + stringify(*id) +
                "' because root ('" + stringify(_imp->params.root) + "') is not a directory");

    tr1::shared_ptr<FSEntrySequence> eclassdirs(new FSEntrySequence);
    eclassdirs->push_back(FSEntry(_imp->params.location / stringify(id->name().category) /
                (stringify(id->name().package) + "-" + stringify(id->version()))));

    FSEntry pkg_dir(_imp->params.location / stringify(id->name().category) /
            (stringify(id->name().package) + "-" + stringify(id->version())));

    tr1::shared_ptr<FSEntry> load_env(new FSEntry(pkg_dir / "environment.bz2"));
    EAPIPhases phases(id->eapi()->supported->ebuild_phases->ebuild_config);

    for (EAPIPhases::Iterator phase(phases.begin_phases()), phase_end(phases.end_phases()) ;
            phase != phase_end ; ++phase)
    {
        EbuildConfigCommand config_cmd(EbuildCommandParams::create()
                .environment(_imp->params.environment)
                .package_id(id)
                .ebuild_dir(pkg_dir)
                .ebuild_file(pkg_dir / (stringify(id->name().package) + "-" + stringify(id->version()) + ".ebuild"))
                .files_dir(pkg_dir)
                .eclassdirs(eclassdirs)
                .exlibsdirs(make_shared_ptr(new FSEntrySequence))
                .portdir(_imp->params.location)
                .distdir(pkg_dir)
                .sandbox(phase->option("sandbox"))
                .userpriv(phase->option("userpriv"))
                .commands(join(phase->begin_commands(), phase->end_commands(), " "))
                .buildroot(_imp->params.buildroot),

                EbuildConfigCommandParams::create()
                .root(stringify(_imp->params.root) + "/")
                .load_environment(load_env.get()));

        config_cmd();
    }
}

tr1::shared_ptr<SetSpecTree::ConstItem>
VDBRepository::do_package_set(const SetName & s) const
{
    using namespace tr1::placeholders;

    Context context("When fetching package set '" + stringify(s) + "' from '" +
            stringify(name()) + "':");

    if ("everything" == s.data())
    {
        tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > result(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
        tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("everything"), stringify(name())));

        need_category_names();
        std::for_each(_imp->categories.begin(), _imp->categories.end(),
                tr1::bind(tr1::mem_fn(&VDBRepository::need_package_ids), this,
                    tr1::bind<CategoryNamePart>(tr1::mem_fn(
                            &std::pair<const CategoryNamePart, tr1::shared_ptr<QualifiedPackageNameSet> >::first), _1)));

        for (CategoryMap::const_iterator i(_imp->categories.begin()), i_end(_imp->categories.end()) ;
                i != i_end ; ++i)
            for (QualifiedPackageNameSet::Iterator e(i->second->begin()), e_end(i->second->end()) ;
                    e != e_end ; ++e)
            {
                tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                            tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(*e))));
                spec->set_tag(tag);
                result->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(spec)));
            }

        return result;
    }
    else if ("world" == s.data())
    {
        tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(SetName("world"), stringify(name())));

        if (_imp->params.world.exists())
        {
            SetFile world(SetFileParams::create()
                    .file_name(_imp->params.world)
                    .type(sft_simple)
                    .parse_mode(pds_pm_unspecific)
                    .tag(tag)
                    .environment(_imp->params.environment));
            return world.contents();
        }
        else
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "World file '" + stringify(_imp->params.world) +
                    "' doesn't exist");

        return tr1::shared_ptr<SetSpecTree::ConstItem>(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
    else
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

tr1::shared_ptr<const SetNameSet>
VDBRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("everything"));
    result->insert(SetName("world"));
    return result;
}

void
VDBRepository::invalidate()
{
    Lock l(_imp->big_nasty_mutex);

    _imp.reset(new Implementation<VDBRepository>(this, _imp->params));
}

void
VDBRepository::invalidate_masks()
{
}

void
VDBRepository::add_string_to_world(const std::string & n) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When adding '" + n + "' to world file '" + stringify(_imp->params.world) + "':");

    if (! _imp->params.world.exists())
    {
        std::ofstream f(stringify(_imp->params.world).c_str());
        if (! f)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot create world file '"
                    + stringify(_imp->params.world) + "'");
            return;
        }
    }

    SetFile world(SetFileParams::create()
            .file_name(_imp->params.world)
            .type(sft_simple)
            .parse_mode(pds_pm_unspecific)
            .tag(tr1::shared_ptr<DepTag>())
            .environment(_imp->params.environment));
    world.add(n);
    world.rewrite();
}

void
VDBRepository::remove_string_from_world(const std::string & n) const
{
    Lock l(_imp->big_nasty_mutex);

    Context context("When removing '" + n + "' from world file '" + stringify(_imp->params.world) + "':");

    if (_imp->params.world.exists())
    {
        SetFile world(SetFileParams::create()
                .file_name(_imp->params.world)
                .type(sft_simple)
                .parse_mode(pds_pm_unspecific)
                .tag(tr1::shared_ptr<DepTag>())
                .environment(_imp->params.environment));

        world.remove(n);
        world.rewrite();
    }
}

void
VDBRepository::add_to_world(const QualifiedPackageName & n) const
{
    add_string_to_world(stringify(n));
}

void
VDBRepository::add_to_world(const SetName & n) const
{
    add_string_to_world(stringify(n));
}

void
VDBRepository::remove_from_world(const QualifiedPackageName & n) const
{
    remove_string_from_world(stringify(n));
}

void
VDBRepository::remove_from_world(const SetName & n) const
{
    remove_string_from_world(stringify(n));
}

std::string
VDBRepository::get_environment_variable(
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' for '" +
            stringify(*id) + "':");

    FSEntry vdb_dir(_imp->params.location / stringify(id->name().category)
            / (stringify(id->name().package) + "-" +
                stringify(id->version())));

    if (! vdb_dir.is_directory_or_symlink_to_directory())
        throw EnvironmentVariableActionError("Could not find VDB entry for '"
                + stringify(*id) + "'");

    if ((vdb_dir / var).is_regular_file_or_symlink_to_regular_file())
    {
        std::ifstream f(stringify(vdb_dir / var).c_str());
        if (! f)
            throw EnvironmentVariableActionError("Could not read '" +
                    stringify(vdb_dir / var) + "'");
        return strip_trailing_string(
                std::string((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>()), "\n");
    }
    else if ((vdb_dir / "environment.bz2").is_regular_file_or_symlink_to_regular_file())
    {
        PStream p("bash -c '( bunzip2 < " + stringify(vdb_dir / "environment.bz2" ) +
                " ; echo echo \\$" + var + " ) | bash 2>/dev/null'");
        std::string result(strip_trailing_string(std::string(
                        (std::istreambuf_iterator<char>(p)),
                        std::istreambuf_iterator<char>()), "\n"));
        if (0 != p.exit_status())
            throw EnvironmentVariableActionError("Could not load environment.bz2");
        return result;
    }
    else
        throw EnvironmentVariableActionError("Could not get variable '" + var + "' for '"
                + stringify(*id) + "'");
}

tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence>
VDBRepository::provided_packages() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->provides)
        return _imp->provides;

    if (! load_provided_using_cache())
        load_provided_the_slow_way();

    return _imp->provides;
}

tr1::shared_ptr<const UseFlagNameSet>
VDBRepository::do_arch_flags() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

tr1::shared_ptr<const UseFlagNameSet>
VDBRepository::do_use_expand_flags() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

tr1::shared_ptr<const UseFlagNameSet>
VDBRepository::do_use_expand_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

tr1::shared_ptr<const UseFlagNameSet>
VDBRepository::do_use_expand_hidden_prefixes() const
{
    return tr1::shared_ptr<const UseFlagNameSet>(new UseFlagNameSet);
}

bool
VDBRepository::load_provided_using_cache() const
{
    Lock l(_imp->big_nasty_mutex);

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
        WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));
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

        DepSpecFlattener f(_imp->params.environment, id);
        tr1::shared_ptr<ProvideSpecTree::ConstItem> pp(DepParser::parse_provide(
                    join(next(next(tokens.begin())), tokens.end(), " "), *EAPIData::get_instance()->eapi_from_string("paludis-1")));
        pp->accept(f);

        for (DepSpecFlattener::Iterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
            result->push_back(RepositoryProvidesEntry::create()
                    .virtual_name(QualifiedPackageName((*p)->text()))
                    .provided_by(id));
    }

    _imp->provides = result;
    return true;
}

void
VDBRepository::load_provided_the_slow_way() const
{
    Lock l(_imp->big_nasty_mutex);

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
        for (PackageIDSequence::Iterator e(i->second->begin()), e_end(i->second->end()) ;
                e != e_end ; ++e)
        {
            Context loop_context("When loading VDB PROVIDEs entry for '" + stringify(**e) + "':");

            try
            {
                if (! (*e)->provide_key())
                    continue;

                tr1::shared_ptr<const ProvideSpecTree::ConstItem> provide((*e)->provide_key()->value());;
                DepSpecFlattener f(_imp->params.environment, *e);
                provide->accept(f);

                for (DepSpecFlattener::Iterator p(f.begin()), p_end(f.end()) ; p != p_end ; ++p)
                {
                    QualifiedPackageName pp((*p)->text());

                    if (pp.category != CategoryNamePart("virtual"))
                        Log::get_instance()->message(ll_warning, lc_no_context, "PROVIDE of non-virtual '"
                                + stringify(pp) + "' from '" + stringify(**e) + "' will not work as expected");

                    result->push_back(RepositoryProvidesEntry::create()
                            .virtual_name(pp)
                            .provided_by(*e));
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
    Lock l(_imp->big_nasty_mutex);

    regenerate_provides_cache();
    _imp->names_cache->regenerate_cache();
}

void
VDBRepository::regenerate_provides_cache() const
{
    Lock l(_imp->big_nasty_mutex);

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
        for (PackageIDSequence::Iterator e(i->second->begin()), e_end(i->second->end()) ;
                e != e_end ; ++e)
        {
            if (! (*e)->provide_key())
                continue;

            tr1::shared_ptr<const ProvideSpecTree::ConstItem> provide((*e)->provide_key()->value());;
            DepSpecPrettyPrinter p(0, false);
            provide->accept(p);
            std::string provide_str(strip_leading(strip_trailing(stringify(p), " \t\r\n"), " \t\r\n"));
            if (provide_str.empty())
                continue;

            f << (*e)->name() << " " << (*e)->version() << " " << provide_str << std::endl;
        }
    }
}

tr1::shared_ptr<const CategoryNamePartSet>
VDBRepository::do_category_names_containing_package(const PackageNamePart & p) const
{
    Lock l(_imp->big_nasty_mutex);

    if (! _imp->names_cache->usable())
        return Repository::do_category_names_containing_package(p);

    tr1::shared_ptr<const CategoryNamePartSet> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::do_category_names_containing_package(p);
}

bool
VDBRepository::is_suitable_destination_for(const PackageID & e) const
{
    std::string f(e.repository()->format());
    return f == "ebuild" || f == "ebin";
}

bool
VDBRepository::is_default_destination() const
{
    return _imp->params.environment->root() == root();
}

std::string
VDBRepository::do_describe_use_flag(const UseFlagName &, const PackageID &) const
{
    return "";
}

FSEntry
VDBRepository::root() const
{
    return _imp->params.root;
}

bool
VDBRepository::want_pre_post_phases() const
{
    return true;
}

void
VDBRepository::merge(const MergeOptions & m)
{
    Context context("When merging '" + stringify(*m.package_id) + "' at '" + stringify(m.image_dir)
            + "' to VDB repository '" + stringify(name()) + "':");

    if (! is_suitable_destination_for(*m.package_id))
        throw PackageInstallActionError("Not a suitable destination for '" + stringify(*m.package_id) + "'");

    bool is_replace(package_id_if_exists(m.package_id->name(), m.package_id->version()));

    FSEntry tmp_vdb_dir(_imp->params.location);
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= stringify(m.package_id->name().category);
    if (! tmp_vdb_dir.exists())
        tmp_vdb_dir.mkdir();
    tmp_vdb_dir /= ("-checking-" + stringify(m.package_id->name().package) + "-" + stringify(m.package_id->version()));
    tmp_vdb_dir.mkdir();

    WriteVDBEntryCommand write_vdb_entry_command(
            WriteVDBEntryParams::create()
            .environment(_imp->params.environment)
            .package_id(tr1::static_pointer_cast<const ERepositoryID>(m.package_id))
            .output_directory(tmp_vdb_dir)
            .environment_file(m.environment_file));

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
    vdb_dir /= stringify(m.package_id->name().category);
    vdb_dir /= (stringify(m.package_id->name().package) + "-" + stringify(m.package_id->version()));

    VDBMerger merger(
            VDBMergerOptions::create()
            .environment(_imp->params.environment)
            .image(m.image_dir)
            .root(root())
            .contents_file(vdb_dir / "CONTENTS")
            .config_protect(config_protect)
            .config_protect_mask(config_protect_mask)
            .package_id(m.package_id));

    if (! merger.check())
    {
        for (DirIterator d(tmp_vdb_dir, false), d_end ; d != d_end ; ++d)
            FSEntry(*d).unlink();
        tmp_vdb_dir.rmdir();
        throw PackageInstallActionError("Not proceeding with install due to merge sanity check failing");
    }

    if (is_replace)
    {
        if ((vdb_dir.dirname() / ("-reinstalling-" + vdb_dir.basename())).exists())
            throw PackageInstallActionError("Directory '" + stringify(vdb_dir.dirname() /
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
        perform_uninstall(tr1::static_pointer_cast<const ERepositoryID>(m.package_id), uninstall_options, true);
    }

    VDBPostMergeCommand post_merge_command(
            VDBPostMergeCommandParams::create()
            .root(root()));

    post_merge_command();
}

HookResult
VDBRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return HookResult(0, "");
}

void
VDBRepository::need_category_names() const
{
    Lock l(_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    Context context("When loading category names from '" + stringify(_imp->params.location) + "':");

    for (DirIterator d(_imp->params.location), d_end ; d != d_end ; ++d)
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
    Lock l(_imp->big_nasty_mutex);

    if (_imp->categories[c])
        return;

    Context context("When loading package names from '" + stringify(_imp->params.location) +
            "' in category '" + stringify(c) + "':");

    tr1::shared_ptr<QualifiedPackageNameSet> q(new QualifiedPackageNameSet);

    for (DirIterator d(_imp->params.location / stringify(c)), d_end ; d != d_end ; ++d)
        try
        {
            if (d->is_directory_or_symlink_to_directory())
            {
                std::string s(d->basename());
                if (std::string::npos == s.rfind('-'))
                    continue;

                PackageDepSpec p("=" + stringify(c) + "/" + s, pds_pm_permissive);
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
    Lock l(_imp->big_nasty_mutex);

    Context context("When creating ID for '" + stringify(q) + "-" + stringify(v) + "' from '" + stringify(f) + "':");

    tr1::shared_ptr<VDBID> result(new VDBID(q, v, _imp->params.environment, shared_from_this(), f));
    return result;
}

const tr1::shared_ptr<const ERepositoryID>
VDBRepository::package_id_if_exists(const QualifiedPackageName & q, const VersionSpec & v) const
{
    Lock l(_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return tr1::shared_ptr<const ERepositoryID>();

    need_package_ids(q.category);

    using namespace tr1::placeholders;

    PackageIDSequence::Iterator i(std::find_if(_imp->ids[q]->begin(), _imp->ids[q]->end(),
                tr1::bind(std::equal_to<VersionSpec>(), v, tr1::bind(tr1::mem_fn(&PackageID::version), _1))));
    if (_imp->ids[q]->end() == i)
        return tr1::shared_ptr<const ERepositoryID>();
    return tr1::static_pointer_cast<const ERepositoryID>(*i);
}

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }
    };
}

bool
VDBRepository::do_some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

