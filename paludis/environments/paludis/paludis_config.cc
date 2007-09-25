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

#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/keywords_conf.hh>
#include <paludis/environments/paludis/use_conf.hh>
#include <paludis/environments/paludis/mirrors_conf.hh>
#include <paludis/environments/paludis/licenses_conf.hh>
#include <paludis/environments/paludis/package_mask_conf.hh>

#include <paludis/config_file.hh>
#include <paludis/distribution.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/mutex.hh>

#include <paludis/util/tr1_functional.hh>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>
#include <vector>

#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include "config.h"

/** \file
 * Implementation of paludis_config.hh classes.
 *
 * \ingroup grppaludisconfig
 */

using namespace paludis;
using namespace paludis::paludis_environment;

#include <paludis/environments/paludis/use_config_entry-sr.cc>
#include <paludis/environments/paludis/repository_config_entry-sr.cc>

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
        tr1::shared_ptr<FSEntrySequence> bashrc_files;

        std::list<RepositoryConfigEntry> repos;

        tr1::shared_ptr<KeywordsConf> keywords_conf;
        tr1::shared_ptr<UseConf> use_conf;
        tr1::shared_ptr<LicensesConf> licenses_conf;
        tr1::shared_ptr<PackageMaskConf> package_mask_conf;
        tr1::shared_ptr<PackageMaskConf> package_unmask_conf;
        tr1::shared_ptr<MirrorsConf> mirrors_conf;

        mutable Mutex reduced_mutex;
        mutable tr1::shared_ptr<uid_t> reduced_uid;
        mutable tr1::shared_ptr<gid_t> reduced_gid;

        mutable Mutex environment_conf_mutex;
        mutable bool has_environment_conf;
        mutable bool accept_breaks_portage;
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
        accept_breaks_portage(true),
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

        tr1::shared_ptr<KeyValueConfigFile> kv;

        if ((FSEntry(config_dir) / "environment.conf").exists())
            kv.reset(new KeyValueConfigFile(FSEntry(config_dir) / "environment.conf", KeyValueConfigFileOptions()));
        else if ((FSEntry(config_dir) / "environment.bash").exists())
        {
            Command cmd(Command("bash '" + stringify(FSEntry(config_dir) / "environment.bash") + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix("environment.bash> "));
            PStream s(cmd);
            kv.reset(new KeyValueConfigFile(s, KeyValueConfigFileOptions()));

            if (s.exit_status() != 0)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Script '" + stringify(FSEntry(config_dir) / "environment.bash")
                        + "' returned non-zero exit status '" + stringify(s.exit_status()) + "'");
                kv.reset();
            }
        }
        else
            Log::get_instance()->message(ll_debug, lc_context, "No environment.conf or environment.bash in '"
                    + config_dir + "'");

        if (kv)
        {
            if (! kv->get("reduced_username").empty())
            {
                reduced_username = kv->get("reduced_username");
                Log::get_instance()->message(ll_debug, lc_context,
                        "loaded key 'reduced_username' = '" + reduced_username + "'");
            }
            else
                Log::get_instance()->message(ll_debug, lc_context,
                        "Key 'reduced_username' is unset, using '" + reduced_username + "'");

            accept_breaks_portage = kv->get("portage_compatible").empty();
            distribution = kv->get("distribution");
        }

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
    using namespace tr1::placeholders;

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

    Log::get_instance()->message(ll_debug, lc_no_context, "PaludisConfig initial directory is '"
            + stringify(local_config_dir) + "'");

    // Prefer specpath.conf over specpath. Warn if specpath is used.
    if ((local_config_dir / "specpath.conf").exists() || (local_config_dir / "specpath").exists())
    {
        KeyValueConfigFile* specpath;
        if ((local_config_dir / "specpath.conf").exists())
            specpath = new KeyValueConfigFile(local_config_dir / "specpath.conf", KeyValueConfigFileOptions());
        else
        {
            specpath = new KeyValueConfigFile(local_config_dir / "specpath", KeyValueConfigFileOptions());
            Log::get_instance()->message(ll_warning, lc_no_context, "Using specpath is deprecated, use specpath.conf instead");
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

    _imp->root = root_prefix;
    _imp->config_dir = stringify(local_config_dir);
    /* check that we can safely use userpriv */
    {
        Command cmd(Command("ls -ld '" + stringify(local_config_dir) + "'/* >/dev/null 2>/dev/null")
                .with_uid_gid(reduced_uid(), reduced_gid()));
        if (0 != run_command(cmd))
        {
            Log::get_instance()->message(ll_warning, lc_context, "Cannot access configuration directory '"
                    + stringify(local_config_dir) + "' using userpriv, so userpriv will be disabled");
            _imp->reduced_uid.reset(new uid_t(getuid()));
            _imp->reduced_gid.reset(new gid_t(getgid()));
        }
    }

    tr1::shared_ptr<Map<std::string, std::string> > conf_vars(
            new Map<std::string, std::string>);
    conf_vars->insert("ROOT", root_prefix);

    Log::get_instance()->message(ll_debug, lc_no_context, "PaludisConfig real directory is '"
            + stringify(local_config_dir) + "', root prefix is '" + root_prefix +
            "', config suffix is '" + local_config_suffix + "'");

    /* repositories */
    {
        /* add virtuals repositories */

        if (DistributionData::get_instance()->distribution_from_string(distribution())->support_old_style_virtuals)
        {
            tr1::shared_ptr<Map<std::string, std::string> > iv_keys(
                    new Map<std::string, std::string>);
            iv_keys->insert("root", root_prefix.empty() ? "/" : root_prefix);
            _imp->repos.push_back(RepositoryConfigEntry("installed_virtuals", -1, iv_keys));

            _imp->repos.push_back(RepositoryConfigEntry("virtuals", -2,
                        tr1::shared_ptr<Map<std::string, std::string> >()));
        }

        /* add normal repositories */

        if ((local_config_dir / "repository_defaults.conf").exists())
        {
            KeyValueConfigFile defaults_file(local_config_dir / "repository_defaults.conf", KeyValueConfigFileOptions(),
                    KeyValueConfigFile::Defaults(conf_vars));
            std::copy(defaults_file.begin(), defaults_file.end(), conf_vars->inserter());
        }
        else if ((local_config_dir / "repository_defaults.bash").exists())
        {
            Command cmd(Command("bash '" + stringify(local_config_dir / "repository_defaults.bash") + "'")
                    .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_stderr_prefix("repository_defaults.bash> "));
            PStream s(cmd);
            KeyValueConfigFile defaults_file(s, KeyValueConfigFileOptions(), KeyValueConfigFile::Defaults(conf_vars));
            std::copy(defaults_file.begin(), defaults_file.end(), conf_vars->inserter());
        }

        std::list<FSEntry> dirs;
        dirs.push_back(local_config_dir / "repositories");

        std::list<FSEntry> repo_files;
        for (std::list<FSEntry>::const_iterator dir(dirs.begin()), dir_end(dirs.end()) ;
                dir != dir_end ; ++dir)
        {
            if (! dir->exists())
                continue;

            std::copy(DirIterator(*dir), DirIterator(),
                    filter_inserter(std::back_inserter(repo_files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(*dir), DirIterator(),
                    filter_inserter(std::back_inserter(repo_files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
        }

        std::list<tr1::shared_ptr<Map<std::string, std::string> > > later_keys;
        for (std::list<FSEntry>::const_iterator repo_file(repo_files.begin()), repo_file_end(repo_files.end()) ;
                repo_file != repo_file_end ; ++repo_file)
        {
            Context local_context("When reading repository file '" + stringify(*repo_file) + "':");

            tr1::shared_ptr<KeyValueConfigFile> kv;
            if (is_file_with_extension(*repo_file, ".bash", IsFileWithOptions()))
            {
                Command cmd(Command("bash '" + stringify(*repo_file) + "'")
                        .with_setenv("PALUDIS_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                        .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                        .with_stderr_prefix(repo_file->basename() + "> "));
                PStream s(cmd);
                kv.reset(new KeyValueConfigFile(s, KeyValueConfigFileOptions(), KeyValueConfigFile::Defaults(conf_vars)));

                if (s.exit_status() != 0)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Script '" + stringify(*repo_file)
                            + "' returned non-zero exit status '" + stringify(s.exit_status()) + "'");
                    kv.reset();
                }
            }
            else
                kv.reset(new KeyValueConfigFile(*repo_file, KeyValueConfigFileOptions(), KeyValueConfigFile::Defaults(conf_vars)));

            if (! kv)
                continue;

            std::string format(kv->get("format"));
            if (format.empty())
                throw PaludisConfigError("Key 'format' not specified or empty");

            int importance(kv->get("master_repository").empty() ? 0 : 10);
            if (! kv->get("importance").empty())
                importance = destringify<int>(kv->get("importance"));

            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            std::copy(kv->begin(), kv->end(), keys->inserter());

            keys->erase("importance");
            keys->insert("importance", stringify(importance));

            keys->erase("repo_file");
            keys->insert("repo_file", stringify(*repo_file));

            keys->erase("root");
            keys->insert("root", root_prefix.empty() ? "/" : root_prefix);

            if (! kv->get("master_repository").empty())
            {
                Log::get_instance()->message(ll_debug, lc_context, "Delaying '" + stringify(*repo_file) +
                        "' because it uses master_repository");
                later_keys.push_back(keys);
            }
            else
            {
                Log::get_instance()->message(ll_debug, lc_context, "Not delaying '" + stringify(*repo_file) + "'");
                _imp->repos.push_back(RepositoryConfigEntry(format, importance, keys));
            }
        }

        for (std::list<tr1::shared_ptr<Map<std::string, std::string> > >::const_iterator
                k(later_keys.begin()), k_end(later_keys.end()) ; k != k_end ; ++k)
            _imp->repos.push_back(RepositoryConfigEntry((*k)->find("format")->second,
                        destringify<int>((*k)->find("importance")->second), *k));

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
            std::copy(DirIterator(local_config_dir / "keywords.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "keywords.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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
            std::copy(DirIterator(local_config_dir / "use.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "use.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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
            std::copy(DirIterator(local_config_dir / "licenses.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "licenses.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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
            std::copy(DirIterator(local_config_dir / "package_mask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "package_mask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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
            std::copy(DirIterator(local_config_dir / "package_unmask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "package_unmask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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
            std::copy(DirIterator(local_config_dir / "mirrors.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".conf", IsFileWithOptions())));
            std::copy(DirIterator(local_config_dir / "mirrors.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), tr1::bind(&is_file_with_extension, _1, ".bash", IsFileWithOptions())));
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

tr1::shared_ptr<const FSEntrySequence>
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
                Log::get_instance()->message(ll_warning, lc_no_context,
                        "Couldn't determine uid for user '" + reduced_username() + "'");
                _imp->reduced_uid.reset(new uid_t(getuid()));
            }
            else
                _imp->reduced_uid.reset(new uid_t(p->pw_uid));
        }

        Log::get_instance()->message(ll_debug, lc_context, "Reduced uid is '"
                + stringify(*_imp->reduced_uid) + "'");
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
                Log::get_instance()->message(ll_warning, lc_no_context,
                        "Couldn't determine gid for user '" + reduced_username() + "'");
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

    Log::get_instance()->message(ll_debug, lc_context,
            "Reduced username is '" + _imp->reduced_username + "'");

    return _imp->reduced_username;
}

bool
PaludisConfig::accept_breaks_portage() const
{
    _imp->need_environment_conf();

    return _imp->accept_breaks_portage;
}

tr1::shared_ptr<const KeywordsConf>
PaludisConfig::keywords_conf() const
{
    return _imp->keywords_conf;
}

tr1::shared_ptr<const UseConf>
PaludisConfig::use_conf() const
{
    return _imp->use_conf;
}

tr1::shared_ptr<const LicensesConf>
PaludisConfig::licenses_conf() const
{
    return _imp->licenses_conf;
}

tr1::shared_ptr<const PackageMaskConf>
PaludisConfig::package_mask_conf() const
{
    return _imp->package_mask_conf;
}

tr1::shared_ptr<const PackageMaskConf>
PaludisConfig::package_unmask_conf() const
{
    return _imp->package_unmask_conf;
}

tr1::shared_ptr<const MirrorsConf>
PaludisConfig::mirrors_conf() const
{
    return _imp->mirrors_conf;
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

