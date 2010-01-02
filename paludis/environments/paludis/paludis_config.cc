/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/keywords_conf.hh>
#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/mirrors_conf.hh>
#include <paludis/environments/paludis/licenses_conf.hh>
#include <paludis/environments/paludis/package_mask_conf.hh>
#include <paludis/environments/paludis/output_conf.hh>
#include <paludis/environments/paludis/world.hh>
#include <paludis/environments/paludis/extra_distribution_data.hh>

#include <paludis/util/config_file.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/member_iterator-impl.hh>

#include <paludis/distribution.hh>
#include <paludis/repository_factory.hh>

#include <tr1/functional>
#include <tr1/unordered_map>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>
#include <vector>

#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>

#include "config.h"

/** \file
 * Implementation of paludis_config.hh classes.
 *
 * \ingroup grppaludisconfig
 */

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<std::tr1::function<std::string (const std::string &)> > Repos;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    std::string from_kv(const std::tr1::shared_ptr<const KeyValueConfigFile> & m,
            const std::string & k)
    {
        return m->get(k);
    }

    std::string predefined(
            const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const KeyValueConfigFile &,
            const std::string & k)
    {
        return from_keys(m, k);
    }

    std::string initial_conf_vars(const std::string & r, const std::string & k)
    {
        if (k == "root" || k == "ROOT")
            return r;
        return "";
    }

    std::string to_kv_func(
            const std::tr1::function<std::string (const std::string &)> & f,
            const KeyValueConfigFile &,
            const std::string & k)
    {
        return f(k);
    }

    std::string override(
            const std::string & o,
            const std::string & v,
            const std::tr1::function<std::string (const std::string &)> & f,
            const std::string & k)
    {
        if (k == o)
            return v;
        return f(k);
    }

    void parse_commandline_vars(
        const std::string & varstr,
        const std::tr1::shared_ptr<Map<std::string, std::string> > & varmap)
    {
        typedef std::list<std::string> SetsType;
        SetsType sets;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(varstr, ":", "", std::back_inserter(sets));

        for (SetsType::const_iterator it = sets.begin(), end = sets.end(); end != it; ++it)
        {
            const std::string::size_type assign(it->find("="));

            if (std::string::npos != assign)
            {
                const std::string var_name(it->substr(0, assign));
                const std::string var_val(it->substr(assign + 1));

                if (! var_name.empty()  && ! var_val.empty())
                    varmap->insert(var_name,  var_val);
            }
        }
    }
}

namespace paludis
{
    /**
     * Implementation data for PaludisConfig.
     *
     * \ingroup grppaludisconfig
     */
    template<>
    struct Implementation<PaludisConfig>
    {
        PaludisEnvironment * const env;

        std::string paludis_command;
        std::string root;
        std::string config_dir;
        mutable Mutex distribution_mutex;
        mutable std::string distribution;
        std::tr1::shared_ptr<FSEntrySequence> bashrc_files;

        Repos repos;

        std::tr1::shared_ptr<KeywordsConf> keywords_conf;
        std::tr1::shared_ptr<UseConf> use_conf;
        std::tr1::shared_ptr<LicensesConf> licenses_conf;
        std::tr1::shared_ptr<PackageMaskConf> package_mask_conf;
        std::tr1::shared_ptr<PackageMaskConf> package_unmask_conf;
        std::tr1::shared_ptr<MirrorsConf> mirrors_conf;
        std::tr1::shared_ptr<OutputConf> output_conf;
        mutable std::tr1::shared_ptr<World> world;

        mutable Mutex reduced_mutex;
        mutable std::tr1::shared_ptr<uid_t> reduced_uid;
        mutable std::tr1::shared_ptr<gid_t> reduced_gid;

        mutable Mutex environment_conf_mutex;
        mutable bool has_environment_conf;
        mutable bool accept_all_breaks_portage;
        mutable Set<std::string> accept_breaks_portage;
        mutable std::string reduced_username;

        std::tr1::shared_ptr<Map<std::string, std::string> > commandline_environment;

        Implementation(PaludisEnvironment * const);

        void need_environment_conf() const;
    };

    Implementation<PaludisConfig>::Implementation(PaludisEnvironment * e) :
        env(e),
        paludis_command("paludis"),
        config_dir("(unset)"),
        bashrc_files(new FSEntrySequence),
        keywords_conf(new KeywordsConf(e)),
        use_conf(new UseConf(e)),
        licenses_conf(new LicensesConf(e)),
        package_mask_conf(new PackageMaskConf(e)),
        package_unmask_conf(new PackageMaskConf(e)),
        mirrors_conf(new MirrorsConf(e)),
        output_conf(new OutputConf(e)),
        has_environment_conf(false),
        accept_all_breaks_portage(false),
        reduced_username(getenv_with_default("PALUDIS_REDUCED_USERNAME", "paludisbuild")),
        commandline_environment(new Map<std::string, std::string>)
    {
    }

    void
    Implementation<PaludisConfig>::need_environment_conf() const
    {
        Lock lock(environment_conf_mutex);

        if (has_environment_conf)
            return;

        Context context("When loading environment.conf:");

        std::tr1::shared_ptr<KeyValueConfigFile> kv;
        std::tr1::shared_ptr<FSEntry> world_file;

        commandline_environment->insert("root", root);
        commandline_environment->insert("ROOT", root);
        commandline_environment->insert("accept_breaks_portage", "*");

        const KeyValueConfigFile::DefaultFunction def_predefined =
            std::tr1::bind(
                &predefined,
                commandline_environment,
                std::tr1::placeholders::_1,
                std::tr1::placeholders::_2);

        if ((FSEntry(config_dir) / "environment.conf").exists())
        {
            kv.reset(new KeyValueConfigFile(
                FSEntry(config_dir) / "environment.conf",
                KeyValueConfigFileOptions(),
                def_predefined,
                &KeyValueConfigFile::no_transformation));
        }
        else if ((FSEntry(config_dir) / "environment.bash").exists())
        {
            std::stringstream s;
            Command cmd(Command("bash '" + stringify(FSEntry(config_dir) / "environment.bash") + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix("environment.bash> ")
                    .with_captured_stdout_stream(&s));
            int exit_status(run_command(cmd));
            kv.reset(new KeyValueConfigFile(
                s,
                KeyValueConfigFileOptions(),
                def_predefined,
                &KeyValueConfigFile::no_transformation));

            if (exit_status != 0)
            {
                Log::get_instance()->message("paludis_environment.environment_bash.failure", ll_warning, lc_context)
                    << "Script '" << (FSEntry(config_dir) / "environment.bash") <<
                    "' returned non-zero exit status '" << exit_status << "'";
                kv.reset();
            }
        }
        else
        {
            Log::get_instance()->message("paludis_environment.no_environment_conf", ll_debug, lc_context)
                << "No environment.conf or environment.bash in '" << config_dir << "'";
            std::stringstream str;
            kv.reset(new KeyValueConfigFile(
                str,
                KeyValueConfigFileOptions(),
                def_predefined,
                &KeyValueConfigFile::no_transformation));
        }

        if (! kv->get("PALUDIS_NO_WRITE_CACHE_CLEAN").empty())
        {
            Log::get_instance()->message("paludis_environment.dodgy_script_config", ll_warning, lc_context)
                << "It looks like you configured Paludis using a dodgy broken script you found on the "
                "forums. Unfortunately, your system is probably now broken beyond repair. Please start again, "
                "and have more care when deciding how to set things up.";
        }

        if (! kv->get("reduced_username").empty())
            reduced_username = kv->get("reduced_username");

        std::list<std::string> breakages;
        tokenise_whitespace(kv->get("accept_breaks_portage"), std::back_inserter(breakages));
        for (std::list<std::string>::const_iterator it(breakages.begin()),
                 it_end(breakages.end()); it_end != it; ++it)
            if ("*" == *it)
            {
                accept_all_breaks_portage = true;
                break;
            }
            else
                accept_breaks_portage.insert(*it);

        distribution = kv->get("distribution");

        if (! kv->get("world").empty())
            world_file.reset(new FSEntry(kv->get("world")));

        if (! world_file)
            Log::get_instance()->message("paludis_environment.world.no_world", ll_warning, lc_context)
                << "No world file specified. You should specify 'world = /path/to/world/file' in "
                << (FSEntry(config_dir) / "environment.conf")
                << ". Any attempted updates to world will not be saved.";
        world.reset(new World(env, world_file));

        has_environment_conf = true;
    }

    template <>
    struct WrappedForwardIteratorTraits<PaludisConfig::RepositoryConstIteratorTag>
    {
        typedef Repos::const_iterator UnderlyingIterator;
    };
}

PaludisConfigError::PaludisConfigError(const std::string & msg) throw () :
    ConfigurationError("Paludis configuration error: " + msg)
{
}

PaludisConfigNoDirectoryError::PaludisConfigNoDirectoryError(const std::string & msg) throw () :
    PaludisConfigError("Paludis configuration error: " + msg)
{
}

PaludisConfig::PaludisConfig(PaludisEnvironment * const e, const std::string & suffix) :
    PrivateImplementationPattern<PaludisConfig>(new Implementation<PaludisConfig>(e))
{
    using namespace std::tr1::placeholders;

    Context context("When loading paludis configuration:");

    /* indirection */
    std::string local_config_suffix;

    if (! suffix.empty())
    {
        const std::string::size_type delim(suffix.find(":"));

        if (std::string::npos != delim)
        {
            local_config_suffix = "-" + suffix.substr(0, delim);
            parse_commandline_vars(suffix.substr(delim + 1), _imp->commandline_environment);
        }
        else
        {
            local_config_suffix = "-" + suffix;
        }
    }

    FSEntry local_config_dir(FSEntry(getenv_with_default("PALUDIS_HOME", getenv_or_error("HOME"))) /
            (".paludis" + local_config_suffix)), old_config_dir(local_config_dir);

    try
    {
        if (! local_config_dir.exists())
            local_config_dir = (FSEntry(SYSCONFDIR) / ("paludis" + local_config_suffix));
    }
    catch (const FSError &)
    {
        local_config_dir = (FSEntry(SYSCONFDIR) / ("paludis" + local_config_suffix));
    }

    if (_imp->commandline_environment->end() == _imp->commandline_environment->find("root"))
    {
        if (! local_config_dir.exists())
            throw PaludisConfigNoDirectoryError("Can't find configuration directory (tried '"
                + stringify(old_config_dir) + "', '" + stringify(local_config_dir) + "')");

        Log::get_instance()->message("paludis_environment.paludis_config.initial_dir", ll_debug, lc_no_context)
            << "PaludisConfig initial directory is '" << local_config_dir << "'";
    }

    std::tr1::shared_ptr<KeyValueConfigFile> specpath;
    const KeyValueConfigFile::DefaultFunction def_predefined =
        std::tr1::bind(
            &predefined,
            _imp->commandline_environment,
            std::tr1::placeholders::_1,
            std::tr1::placeholders::_2);

    if ((local_config_dir / "specpath.conf").exists())
    {
        specpath.reset(new KeyValueConfigFile(
            local_config_dir / "specpath.conf",
            KeyValueConfigFileOptions(),
            def_predefined,
            &KeyValueConfigFile::no_transformation));
    }
    else if (_imp->commandline_environment->end() != _imp->commandline_environment->find("root"))
    {
        std::istringstream strm;
        specpath.reset(new KeyValueConfigFile(
            strm,
            KeyValueConfigFileOptions(),
            def_predefined,
            &KeyValueConfigFile::no_transformation));
    }

    std::string root_prefix;

    if (specpath)
    {
        root_prefix = specpath->get("root");
        local_config_suffix = specpath->get("config-suffix");

        if (! local_config_suffix.empty())
            local_config_suffix.insert(0, "-");
    }

    if (! root_prefix.empty() && stringify(FSEntry(root_prefix).realpath()) != "/")
    {
        local_config_dir = FSEntry(root_prefix) / SYSCONFDIR / ("paludis" + local_config_suffix);
        if (! local_config_dir.exists())
            throw PaludisConfigError("Can't find configuration directory under root ("
                    "tried '" + stringify(local_config_dir) + "' and couldn't find any "
                    "specpath variables on the commandline");
    }

    _imp->root = root_prefix.empty() ? "/" : root_prefix;
    _imp->config_dir = stringify(local_config_dir);

    const std::tr1::shared_ptr<const PaludisDistribution> dist(
            PaludisExtraDistributionData::get_instance()->data_from_distribution(
                *DistributionData::get_instance()->distribution_from_string(distribution())));

    /* check that we can safely use userpriv */
    {
        Command cmd(Command("ls -ld '" + stringify(local_config_dir) + "'/* >/dev/null 2>/dev/null")
                .with_uid_gid(reduced_uid(), reduced_gid()));
        if (0 != run_command(cmd))
        {
            Log::get_instance()->message("paludis_environment.userpriv.disabled", ll_warning, lc_context)
                << "Cannot access configuration directory '" << local_config_dir
                << "' using userpriv, so userpriv will be disabled. Generally Paludis "
                "configuration directories and files should be world readable.";
            _imp->reduced_uid.reset(new uid_t(getuid()));
            _imp->reduced_gid.reset(new gid_t(getgid()));
        }

        if (dist->mandatory_userpriv() && ((0 == *_imp->reduced_uid || 0 == *_imp->reduced_gid)))
        {
            std::string s;
            if (0 == *_imp->reduced_uid)
                s = "uid " + stringify(*_imp->reduced_uid);
            if (0 == *_imp->reduced_gid)
            {
                if (! s.empty())
                    s.append(" or ");
                s.append("gid " + stringify(*_imp->reduced_gid));
            }
            throw PaludisConfigError("Cannot use " + s + " for userpriv");
        }
    }

    std::tr1::function<std::string (const std::string &)> predefined_conf_vars_func(
            std::tr1::bind(&initial_conf_vars, root_prefix, std::tr1::placeholders::_1));

    Log::get_instance()->message("paludis_environment.paludis_config.real_dir", ll_debug, lc_no_context)
        << "PaludisConfig real directory is '" << local_config_dir << "', root prefix is '" << root_prefix
        << "', config suffix is '" << local_config_suffix << "'";

    /* repositories */
    {
        /* add normal repositories. start by getting defaults for config files... */

        if ((local_config_dir / (dist->repository_defaults_filename_part() + ".conf")).exists())
        {
            predefined_conf_vars_func = std::tr1::bind(&from_kv, make_shared_ptr(new KeyValueConfigFile(
                            local_config_dir / (dist->repository_defaults_filename_part() + ".conf"), KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation)),
                    std::tr1::placeholders::_1);
        }
        else if ((local_config_dir / (dist->repository_defaults_filename_part() + ".bash")).exists())
        {
            std::stringstream s;
            Command cmd(Command("bash '" + stringify(local_config_dir / (dist->repository_defaults_filename_part() + ".bash")) + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix(dist->repository_defaults_filename_part() + ".bash> ")
                    .with_captured_stdout_stream(&s));
            int exit_status(run_command(cmd));
            predefined_conf_vars_func = std::tr1::bind(&from_kv, make_shared_ptr(new KeyValueConfigFile(
                            s, KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation)),
                    std::tr1::placeholders::_1);
            if (exit_status != 0)
                Log::get_instance()->message("paludis_environment.repository_defaults.failure", ll_warning, lc_context)
                    << "Script '" << (local_config_dir / (dist->repository_defaults_filename_part() + ".bash"))
                    << "' returned non-zero exit status '" << exit_status << "'";
        }

        /* find candidate config directories */
        std::list<FSEntry> dirs;
        dirs.push_back(local_config_dir / dist->repositories_directory());

        /* find repo config files */
        std::list<FSEntry> repo_files;
        for (std::list<FSEntry>::const_iterator dir(dirs.begin()), dir_end(dirs.end()) ;
                dir != dir_end ; ++dir)
        {
            if (! dir->exists())
                continue;

            std::remove_copy_if(DirIterator(*dir), DirIterator(), std::back_inserter(repo_files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(*dir), DirIterator(), std::back_inserter(repo_files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        /* get a mapping from repository name to key function, so we can work out the order */
        std::tr1::unordered_map<RepositoryName, std::tr1::function<std::string (const std::string &)>, Hash<RepositoryName> > repo_configs;
        for (std::list<FSEntry>::const_iterator repo_file(repo_files.begin()), repo_file_end(repo_files.end()) ;
                repo_file != repo_file_end ; ++repo_file)
        {
            Context local_context("When reading repository file '" + stringify(*repo_file) + "':");

            std::tr1::shared_ptr<KeyValueConfigFile> kv;
            if (is_file_with_extension(*repo_file, ".bash", IsFileWithOptions()))
            {
                std::stringstream s;
                Command cmd(Command("bash '" + stringify(*repo_file) + "'")
                        .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                        .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                        .with_stderr_prefix(repo_file->basename() + "> ")
                        .with_captured_stdout_stream(&s));
                int exit_status(run_command(cmd));
                kv.reset(new KeyValueConfigFile(s, KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation));

                if (exit_status != 0)
                {
                    Log::get_instance()->message("paludis_environment.repositories.failure", ll_warning, lc_context)
                        << "Script '" << *repo_file << "' returned non-zero exit status '" << exit_status << "'";
                    kv.reset();
                }
            }
            else
                kv.reset(new KeyValueConfigFile(*repo_file, KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation));

            if (! kv)
                continue;

            std::tr1::function<std::string (const std::string &)> repo_func(std::tr1::bind(&from_kv, kv, std::tr1::placeholders::_1));

            repo_func = std::tr1::bind(&override, "repo_file", stringify(*repo_file), repo_func, std::tr1::placeholders::_1);
            repo_func = std::tr1::bind(&override, "root", root_prefix.empty() ? "/" : root_prefix, repo_func, std::tr1::placeholders::_1);

            RepositoryName name(RepositoryFactory::get_instance()->name(_imp->env, repo_func));
            if (! repo_configs.insert(std::make_pair(name, repo_func)).second)
            {
                Log::get_instance()->message("paludis_environment.repositories.duplicate", ll_warning, lc_context)
                    << "Duplicate repository name '" << name << "' from config file '" << *repo_file << "', skipping";
                continue;
            }
        }

        /* work out order for repository creation */
        DirectedGraph<RepositoryName, bool, RepositoryNameComparator> repository_deps;
        std::for_each(first_iterator(repo_configs.begin()), first_iterator(repo_configs.end()),
                std::tr1::bind(std::tr1::mem_fn(&DirectedGraph<RepositoryName, bool, RepositoryNameComparator>::add_node), &repository_deps, _1));

        for (std::tr1::unordered_map<RepositoryName, std::tr1::function<std::string (const std::string &)>, Hash<RepositoryName> >::const_iterator
                r(repo_configs.begin()), r_end(repo_configs.end()) ; r != r_end ; ++r)
        {
            std::tr1::shared_ptr<const RepositoryNameSet> deps(RepositoryFactory::get_instance()->dependencies(_imp->env, r->second));
            for (RepositoryNameSet::ConstIterator d(deps->begin()), d_end(deps->end()) ;
                    d != d_end ; ++d)
            {
                if (*d == r->first)
                    throw ConfigurationError("Repository '" + stringify(r->first) + "' depends upon itself");
                try
                {
                    repository_deps.add_edge(r->first, *d, true);
                }
                catch (const NoSuchGraphNodeError &)
                {
                    throw ConfigurationError("Repository '" + stringify(r->first) + "' depends upon '" +
                            stringify(*d) + "', which is not configured");
                }
            }
        }

        try
        {
            std::list<RepositoryName> ordered_repos;
            repository_deps.topological_sort(std::back_inserter(ordered_repos));

            for (std::list<RepositoryName>::const_iterator o(ordered_repos.begin()), o_end(ordered_repos.end()) ;
                    o != o_end ; ++o)
            {
                std::tr1::unordered_map<RepositoryName, std::tr1::function<std::string (const std::string &)>, Hash<RepositoryName> >::const_iterator
                    c(repo_configs.find(*o));
                if (c == repo_configs.end())
                    throw InternalError(PALUDIS_HERE, "*o not in repo_configs");

                _imp->repos.push_back(c->second);
            }
        }
        catch (const NoGraphTopologicalOrderExistsError & x)
        {
            throw ConfigurationError("Repositories have circular dependencies. Unresolvable repositories are '"
                    + join(x.remaining_nodes()->begin(), x.remaining_nodes()->end(), "', '") + "'");
        }

        if (_imp->repos.empty())
            throw PaludisConfigError("No repositories specified");

#ifdef ENABLE_VIRTUALS_REPOSITORY
        /* add virtuals repositories */
        if ((*DistributionData::get_instance()->distribution_from_string(distribution())).support_old_style_virtuals())
        {
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", root_prefix.empty() ? "/" : root_prefix);
            iv_keys->insert("format", "installed_virtuals");
            _imp->repos.push_back(std::tr1::bind(&from_keys, iv_keys, std::tr1::placeholders::_1));

            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "virtuals");
            _imp->repos.push_back(std::tr1::bind(&from_keys, v_keys, std::tr1::placeholders::_1));
        }
#endif
    }

    /* keywords */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->keywords_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->keywords_filename_part() + ".bash"));
        if ((local_config_dir / (dist->keywords_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->keywords_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->keywords_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading keywords file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->keywords_conf->add(*file);
        }
    }

    /* output */
    {
        std::list<FSEntry> files;
        files.push_back(getenv_with_default("PALUDIS_DEFAULT_OUTPUT_CONF",
                    SHAREDIR "/paludis/environments/paludis/default_output.conf"));
        files.push_back(local_config_dir / (dist->output_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->output_filename_part() + ".bash"));
        if ((local_config_dir / (dist->output_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->output_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->output_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        bool any(false);
        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading output file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->output_conf->add(*file);
            any = true;
        }

        if (! any)
            throw PaludisConfigError("No output confs found");
    }

    /* use */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->use_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->use_filename_part() + ".bash"));
        if ((local_config_dir / (dist->use_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->use_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->use_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading use file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->use_conf->add(*file);
        }
    }

    /* licenses */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->licenses_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->licenses_filename_part() + ".bash"));
        if ((local_config_dir / (dist->licenses_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->licenses_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->licenses_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading licenses file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->licenses_conf->add(*file);
        }
    }

    /* user mask */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->package_mask_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->package_mask_filename_part() + ".bash"));
        if ((local_config_dir / (dist->package_mask_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->package_mask_filename_part() + ".conf.d")),
                    DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->package_mask_filename_part() + ".conf.d")),
                    DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_mask file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->package_mask_conf->add(*file);
        }
    }

    /* user unmask */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->package_unmask_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->package_unmask_filename_part() + ".bash"));
        if ((local_config_dir / (dist->package_unmask_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->package_unmask_filename_part() + ".conf.d")),
                    DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->package_unmask_filename_part() + ".conf.d")),
                    DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_unmask file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->package_unmask_conf->add(*file);
        }
    }

    /* mirrors */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / (dist->mirrors_filename_part() + ".conf"));
        files.push_back(local_config_dir / (dist->mirrors_filename_part() + ".bash"));
        if ((local_config_dir / (dist->mirrors_filename_part() + ".conf.d")).exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / (dist->mirrors_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / (dist->mirrors_filename_part() + ".conf.d")), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading mirrors file '" + stringify(*file) + "':");

            if (! file->exists())
                continue;

            _imp->mirrors_conf->add(*file);
        }
    }

    _imp->bashrc_files->push_back(local_config_dir / dist->bashrc_filename());
}

PaludisConfig::~PaludisConfig()
{
}

std::tr1::shared_ptr<const FSEntrySequence>
PaludisConfig::bashrc_files() const
{
    return _imp->bashrc_files;
}

PaludisConfig::RepositoryConstIterator
PaludisConfig::begin_repositories() const
{
    return RepositoryConstIterator(_imp->repos.begin());
}

PaludisConfig::RepositoryConstIterator
PaludisConfig::end_repositories() const
{
    return RepositoryConstIterator(_imp->repos.end());
}

std::string
PaludisConfig::root() const
{
    return _imp->root;
}

std::string
PaludisConfig::config_dir() const
{
    return _imp->config_dir;
}

uid_t
PaludisConfig::reduced_uid() const
{
    Lock lock(_imp->reduced_mutex);

    if (! _imp->reduced_uid)
    {
        Context context("When determining reduced UID:");

        if (0 != getuid())
            _imp->reduced_uid.reset(new uid_t(getuid()));
        else
        {
            struct passwd * p(getpwnam(reduced_username().c_str()));
            if (! p)
            {
                Log::get_instance()->message("paludis_environment.reduced_uid.unknown", ll_warning, lc_no_context)
                    << "Couldn't determine uid for user '" << reduced_username() << "'";
                _imp->reduced_uid.reset(new uid_t(getuid()));
            }
            else
                _imp->reduced_uid.reset(new uid_t(p->pw_uid));
        }

        Log::get_instance()->message("paludis_environment.reduced_uid.value", ll_debug, lc_context)
            << "Reduced uid is '" << *_imp->reduced_uid << "'";
    }

    return *_imp->reduced_uid;
}

gid_t
PaludisConfig::reduced_gid() const
{
    Lock lock(_imp->reduced_mutex);

    if (! _imp->reduced_gid)
    {
        if (0 != getuid())
            _imp->reduced_gid.reset(new gid_t(getgid()));
        else
        {
            struct passwd * p(getpwnam(reduced_username().c_str()));
            if (! p)
            {
                Log::get_instance()->message("paludis_environment.reduced_gid.unknown", ll_warning, lc_no_context)
                    << "Couldn't determine gid for user '" << reduced_username() << "'";
                _imp->reduced_gid.reset(new gid_t(getgid()));
            }
            else
                _imp->reduced_gid.reset(new gid_t(p->pw_gid));
        }
    }

    return *_imp->reduced_gid;
}

std::string
PaludisConfig::reduced_username() const
{
    Context context("When determining reduced username:");
    _imp->need_environment_conf();

    Log::get_instance()->message("paludis_environment.reduced_username", ll_debug, lc_context)
        << "Reduced username is '" << _imp->reduced_username << "'";

    return _imp->reduced_username;
}

bool
PaludisConfig::accept_all_breaks_portage() const
{
    _imp->need_environment_conf();

    return _imp->accept_all_breaks_portage;
}

const Set<std::string> &
PaludisConfig::accept_breaks_portage() const
{
    _imp->need_environment_conf();

    return _imp->accept_breaks_portage;
}

std::tr1::shared_ptr<const KeywordsConf>
PaludisConfig::keywords_conf() const
{
    return _imp->keywords_conf;
}

std::tr1::shared_ptr<const OutputConf>
PaludisConfig::output_conf() const
{
    return _imp->output_conf;
}

std::tr1::shared_ptr<const UseConf>
PaludisConfig::use_conf() const
{
    return _imp->use_conf;
}

std::tr1::shared_ptr<const LicensesConf>
PaludisConfig::licenses_conf() const
{
    return _imp->licenses_conf;
}

std::tr1::shared_ptr<const PackageMaskConf>
PaludisConfig::package_mask_conf() const
{
    return _imp->package_mask_conf;
}

std::tr1::shared_ptr<const PackageMaskConf>
PaludisConfig::package_unmask_conf() const
{
    return _imp->package_unmask_conf;
}

std::tr1::shared_ptr<const MirrorsConf>
PaludisConfig::mirrors_conf() const
{
    return _imp->mirrors_conf;
}

std::tr1::shared_ptr<const World>
PaludisConfig::world() const
{
    _imp->need_environment_conf();
    return _imp->world;
}

std::string
PaludisConfig::distribution() const
{
    Lock lock(_imp->distribution_mutex);

    if (! _imp->distribution.empty())
        return _imp->distribution;

    _imp->need_environment_conf();

    if (_imp->distribution.empty())
        _imp->distribution = getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION);

    return _imp->distribution;
}

template class WrappedForwardIterator<PaludisConfig::RepositoryConstIteratorTag, const std::tr1::function<std::string (const std::string &)> >;

