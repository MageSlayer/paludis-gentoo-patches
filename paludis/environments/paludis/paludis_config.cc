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

#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/keywords_conf.hh>
#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/mirrors_conf.hh>
#include <paludis/environments/paludis/licenses_conf.hh>
#include <paludis/environments/paludis/package_mask_conf.hh>
#include <paludis/environments/paludis/world.hh>

#include <paludis/util/config_file.hh>
#include <paludis/distribution.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/make_shared_ptr.hh>

#include <tr1/functional>
#include <fstream>
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

template class WrappedForwardIterator<PaludisConfig::RepositoryConstIteratorTag, const RepositoryConfigEntry>;

#include <paludis/environments/paludis/use_config_entry-sr.cc>

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

        std::list<RepositoryConfigEntry> repos;

        std::tr1::shared_ptr<KeywordsConf> keywords_conf;
        std::tr1::shared_ptr<UseConf> use_conf;
        std::tr1::shared_ptr<LicensesConf> licenses_conf;
        std::tr1::shared_ptr<PackageMaskConf> package_mask_conf;
        std::tr1::shared_ptr<PackageMaskConf> package_unmask_conf;
        std::tr1::shared_ptr<MirrorsConf> mirrors_conf;
        mutable std::tr1::shared_ptr<World> world;

        mutable Mutex reduced_mutex;
        mutable std::tr1::shared_ptr<uid_t> reduced_uid;
        mutable std::tr1::shared_ptr<gid_t> reduced_gid;

        mutable Mutex environment_conf_mutex;
        mutable bool has_environment_conf;
        mutable bool accept_all_breaks_portage;
        mutable Set<std::string> accept_breaks_portage;
        mutable std::string reduced_username;

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
        has_environment_conf(false),
        accept_all_breaks_portage(false),
        reduced_username(getenv_with_default("PALUDIS_REDUCED_USERNAME", "paludisbuild"))
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
        std::tr1::shared_ptr<Map<std::string, std::string> > conf_vars(
                new Map<std::string, std::string>);
        conf_vars->insert("ROOT", root);
        conf_vars->insert("root", root);
        conf_vars->insert("accept_breaks_portage", "*");
        std::tr1::shared_ptr<FSEntry> world_file;

        if ((FSEntry(config_dir) / "environment.conf").exists())
            kv.reset(new KeyValueConfigFile(FSEntry(config_dir) / "environment.conf", KeyValueConfigFileOptions(),
                        std::tr1::bind(&predefined, conf_vars, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                        &KeyValueConfigFile::no_transformation));
        else if ((FSEntry(config_dir) / "environment.bash").exists())
        {
            std::stringstream s;
            Command cmd(Command("bash '" + stringify(FSEntry(config_dir) / "environment.bash") + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix("environment.bash> ")
                    .with_captured_stdout_stream(&s));
            int exit_status(run_command(cmd));
            kv.reset(new KeyValueConfigFile(s, KeyValueConfigFileOptions(),
                        std::tr1::bind(&predefined, conf_vars, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
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
            kv.reset(new KeyValueConfigFile(str, KeyValueConfigFileOptions(),
                        std::tr1::bind(&predefined, conf_vars, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                        &KeyValueConfigFile::no_transformation));
        }

        if (! kv->get("reduced_username").empty())
            reduced_username = kv->get("reduced_username");

        if (! kv->get("portage_compatible").empty())
            Log::get_instance()->message("paludis_environment.portage_compatible.deprecated", ll_warning, lc_context)
                << "The 'portage_compatible' variable in environment.conf is deprecated,"
                << " set 'accept_breaks_portage' to empty instead.";
        else
        {
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
        }

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
    std::string root_prefix;
    std::string local_config_suffix;
    if (! suffix.empty())
        local_config_suffix = "-" + suffix;

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

    if (! local_config_dir.exists())
        throw PaludisConfigNoDirectoryError("Can't find configuration directory (tried '"
                + stringify(old_config_dir) + "', '" + stringify(local_config_dir) + "')");

    Log::get_instance()->message("paludis_environment.paludis_config.initial_dir", ll_debug, lc_no_context)
        << "PaludisConfig initial directory is '" << local_config_dir << "'";

    // Prefer specpath.conf over specpath. Warn if specpath is used.
    if ((local_config_dir / "specpath.conf").exists() || (local_config_dir / "specpath").exists())
    {
        KeyValueConfigFile* specpath;
        if ((local_config_dir / "specpath.conf").exists())
            specpath = new KeyValueConfigFile(local_config_dir / "specpath.conf", KeyValueConfigFileOptions(),
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);
        else
        {
            specpath = new KeyValueConfigFile(local_config_dir / "specpath", KeyValueConfigFileOptions(),
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);
            Log::get_instance()->message("paludis_environment.paludis_config.specpath.deprecated", ll_warning, lc_no_context)
                << "Using specpath is deprecated, use specpath.conf instead";
        }
        root_prefix = specpath->get("root");
        local_config_suffix = specpath->get("config-suffix");

        if (! root_prefix.empty() && stringify(FSEntry(root_prefix).realpath()) != "/")
        {
            local_config_dir = FSEntry(root_prefix) / SYSCONFDIR / ("paludis" + local_config_suffix);
            if (! local_config_dir.exists())
                throw PaludisConfigError("Can't find configuration directory under root ("
                        "tried '" + stringify(local_config_dir) + "'");
        }
    }

    _imp->root = root_prefix.empty() ? "/" : root_prefix;
    _imp->config_dir = stringify(local_config_dir);
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
    }

    std::tr1::function<std::string (const std::string &)> predefined_conf_vars_func(
            std::tr1::bind(&initial_conf_vars, root_prefix, std::tr1::placeholders::_1));

    Log::get_instance()->message("paludis_environment.paludis_config.real_dir", ll_debug, lc_no_context)
        << "PaludisConfig real directory is '" << local_config_dir << "', root prefix is '" << root_prefix
        << "', config suffix is '" << local_config_suffix << "'";

    /* repositories */
    {

#ifdef ENABLE_VIRTUALS_REPOSITORY
        /* add virtuals repositories */
        if ((*DistributionData::get_instance()->distribution_from_string(distribution()))[k::support_old_style_virtuals()])
        {
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(
                    new Map<std::string, std::string>);
            iv_keys->insert("root", root_prefix.empty() ? "/" : root_prefix);
            _imp->repos.push_back(RepositoryConfigEntry::named_create()
                    (k::format(), "installed_virtuals")
                    (k::importance(), -1)
                    (k::keys(), std::tr1::bind(&from_keys, iv_keys, std::tr1::placeholders::_1))
                    );

            _imp->repos.push_back(RepositoryConfigEntry::named_create()
                    (k::format(), "virtuals")
                    (k::importance(), -2)
                    (k::keys(), std::tr1::bind(&from_keys, make_shared_ptr(new Map<std::string, std::string>), std::tr1::placeholders::_1))
                    );
        }
#endif

        /* add normal repositories */

        if ((local_config_dir / "repository_defaults.conf").exists())
        {
            predefined_conf_vars_func = std::tr1::bind(&from_kv, make_shared_ptr(new KeyValueConfigFile(
                            local_config_dir / "repository_defaults.conf", KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation)),
                    std::tr1::placeholders::_1);
        }
        else if ((local_config_dir / "repository_defaults.bash").exists())
        {
            std::stringstream s;
            Command cmd(Command("bash '" + stringify(local_config_dir / "repository_defaults.bash") + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix("repository_defaults.bash> ")
                    .with_captured_stdout_stream(&s));
            int exit_status(run_command(cmd));
            predefined_conf_vars_func = std::tr1::bind(&from_kv, make_shared_ptr(new KeyValueConfigFile(
                            s, KeyValueConfigFileOptions(),
                            std::tr1::bind(&to_kv_func, predefined_conf_vars_func, std::tr1::placeholders::_1, std::tr1::placeholders::_2),
                            &KeyValueConfigFile::no_transformation)),
                    std::tr1::placeholders::_1);
            if (exit_status != 0)
                Log::get_instance()->message("paludis_environment.repository_defaults.failure", ll_warning, lc_context)
                    << "Script '" << (local_config_dir / "repository_defaults.bash")
                    << "' returned non-zero exit status '" << exit_status << "'";
        }

        std::list<FSEntry> dirs;
        dirs.push_back(local_config_dir / "repositories");

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

        std::list<std::tr1::function<std::string (const std::string &)> > later_repo_files;
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

            std::string format(kv->get("format"));
            if (format.empty())
                throw PaludisConfigError("Key 'format' not specified or empty");

            int importance(kv->get("master_repository").empty() ? 0 : 10);
            if (! kv->get("importance").empty())
                importance = destringify<int>(kv->get("importance"));

            std::tr1::function<std::string (const std::string &)> repo_func(std::tr1::bind(&from_kv, kv, std::tr1::placeholders::_1));

            repo_func = std::tr1::bind(&override, "importance", stringify(importance), repo_func, std::tr1::placeholders::_1);
            repo_func = std::tr1::bind(&override, "repo_file", stringify(*repo_file), repo_func, std::tr1::placeholders::_1);
            repo_func = std::tr1::bind(&override, "root", root_prefix.empty() ? "/" : root_prefix, repo_func, std::tr1::placeholders::_1);

            if (! repo_func("master_repository").empty())
            {
                Log::get_instance()->message("paludis_environment.repositories.delaying", ll_debug, lc_context)
                    << "Delaying '" << *repo_file << "' because it uses master_repository";
                later_repo_files.push_back(repo_func);
            }
            else
            {
                Log::get_instance()->message("paludis_environment.repositories.not_delaying", ll_debug, lc_context)
                    << "Not delaying '" << *repo_file << "'";
                _imp->repos.push_back(RepositoryConfigEntry::named_create()
                        (k::format(), format)
                        (k::importance(), importance)
                        (k::keys(), repo_func)
                        );
            }
        }

        for (std::list<std::tr1::function<std::string (const std::string &)> >::const_iterator
                k(later_repo_files.begin()), k_end(later_repo_files.end()) ; k != k_end ; ++k)
            _imp->repos.push_back(RepositoryConfigEntry::named_create()
                    (k::format(), (*k)("format"))
                    (k::importance(), destringify<int>((*k)("importance")))
                    (k::keys(), *k)
                    );

        if (_imp->repos.empty())
            throw PaludisConfigError("No repositories specified");
    }

    /* keywords */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "keywords.conf");
        files.push_back(local_config_dir / "keywords.bash");
        if ((local_config_dir / "keywords.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "keywords.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "keywords.conf.d"), DirIterator(), std::back_inserter(files),
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

    /* use */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "use.conf");
        files.push_back(local_config_dir / "use.bash");
        if ((local_config_dir / "use.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "use.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "use.conf.d"), DirIterator(), std::back_inserter(files),
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
        files.push_back(local_config_dir / "licenses.conf");
        files.push_back(local_config_dir / "licenses.bash");
        if ((local_config_dir / "licenses.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "licenses.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "licenses.conf.d"), DirIterator(), std::back_inserter(files),
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
        files.push_back(local_config_dir / "package_mask.conf");
        files.push_back(local_config_dir / "package_mask.bash");
        if ((local_config_dir / "package_mask.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "package_mask.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "package_mask.conf.d"), DirIterator(), std::back_inserter(files),
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
        files.push_back(local_config_dir / "package_unmask.conf");
        files.push_back(local_config_dir / "package_unmask.bash");
        if ((local_config_dir / "package_unmask.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "package_unmask.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "package_unmask.conf.d"), DirIterator(), std::back_inserter(files),
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
        files.push_back(local_config_dir / "mirrors.conf");
        files.push_back(local_config_dir / "mirrors.bash");
        if ((local_config_dir / "mirrors.conf.d").exists())
        {
            std::remove_copy_if(DirIterator(local_config_dir / "mirrors.conf.d"), DirIterator(), std::back_inserter(files),
                    std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::remove_copy_if(DirIterator(local_config_dir / "mirrors.conf.d"), DirIterator(), std::back_inserter(files),
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

    _imp->bashrc_files->push_back(local_config_dir / "bashrc");
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

