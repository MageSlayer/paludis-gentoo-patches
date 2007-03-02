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
#include <paludis/config_file.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>

#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>

/** \file
 * Implementation of paludis_config.hh classes.
 *
 * \ingroup grppaludisconfig
 */

using namespace paludis;

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
        std::string bashrc_files;

        mutable bool sets_expanded;

        std::list<RepositoryConfigEntry> repos;

        std::map<QualifiedPackageName, std::vector<
            std::pair<std::tr1::shared_ptr<const PackageDepSpec>, KeywordName> > > keywords;

        const std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, KeywordName> > empty_keywords;

        std::vector<KeywordName> default_keywords;

        mutable std::vector<SetKeywordConfigEntry> set_keywords;

        std::map<QualifiedPackageName, std::vector<
            std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > > licenses;

        const std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > empty_licenses;

        std::vector<std::string> default_licenses;

        mutable std::vector<SetLicenseConfigEntry> set_licenses;

        std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > > user_masks;

        std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > > user_unmasks;

        std::vector<std::tr1::shared_ptr<const PackageDepSpec> > empty_masks;

        mutable std::vector<SetMaskConfigEntry> set_masks;
        mutable std::vector<SetMaskConfigEntry> set_unmasks;

        std::map<QualifiedPackageName, std::vector<UseConfigEntry> > use;

        mutable std::vector<SetUseConfigEntry> set_use;

        mutable std::vector<SetUseConfigMinusStarEntry> set_use_prefixes_that_have_minus_star;

        std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > empty_use_prefixes;

        std::map<QualifiedPackageName, std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > >
            use_prefixes_that_have_minus_star;

        std::vector<UseConfigEntry> empty_use;

        std::vector<std::pair<UseFlagName, UseFlagState> > default_use;

        std::vector<std::string> default_use_prefixes_that_have_minus_star;

        std::multimap<std::string, std::string> mirrors;

        mutable std::tr1::shared_ptr<uid_t> reduced_uid;
        mutable std::tr1::shared_ptr<gid_t> reduced_gid;

        Implementation(PaludisEnvironment * const);

        void need_sets_expanded() const;
    };

    Implementation<PaludisConfig>::Implementation(PaludisEnvironment * e) :
        env(e),
        paludis_command("paludis"),
        config_dir("(unset)"),
        sets_expanded(false)
    {
    }

    void
    Implementation<PaludisConfig>::need_sets_expanded() const
    {
        if (sets_expanded)
            return;

        {
            Context context("When expanding set names from use.conf:");

            for (std::vector<SetUseConfigEntry>::iterator s(set_use.begin()), s_end(set_use.end()) ;
                    s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }

            for (std::vector<SetUseConfigMinusStarEntry>::iterator s(set_use_prefixes_that_have_minus_star.begin()),
                    s_end(set_use_prefixes_that_have_minus_star.end()) ; s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }
        }

        {
            Context context("When expanding set names from keywords.conf:");

            for (std::vector<SetKeywordConfigEntry>::iterator s(set_keywords.begin()), s_end(set_keywords.end()) ;
                    s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }
        }

        {
            Context context("When expanding set names from licenses.conf:");

            for (std::vector<SetLicenseConfigEntry>::iterator s(set_licenses.begin()), s_end(set_licenses.end()) ;
                    s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }
        }

        {
            Context context("When expanding set names from package_unmask.conf:");

            for (std::vector<SetMaskConfigEntry>::iterator s(set_unmasks.begin()), s_end(set_unmasks.end()) ;
                    s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }
        }

        {
            Context context("When expanding set names from package_mask.conf:");

            for (std::vector<SetMaskConfigEntry>::iterator s(set_masks.begin()), s_end(set_masks.end()) ;
                    s != s_end ; ++s)
                if (! s->dep_spec)
                {
                    s->dep_spec = env->package_set(s->set_name);
                    if (! s->dep_spec)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Set '" +
                                stringify(s->set_name) + "' doesn't exist");
                        s->dep_spec.reset(new AllDepSpec);
                    }
                }
        }

        sets_expanded = true;
    }
}

PaludisConfigError::PaludisConfigError(const std::string & msg) throw () :
    ConfigurationError("Paludis configuration error: " + msg)
{
}

PaludisConfig::PaludisConfig(PaludisEnvironment * const e, const std::string & suffix) :
    PrivateImplementationPattern<PaludisConfig>(new Implementation<PaludisConfig>(e))
{
    Context context("When loading paludis configuration:");

    /* indirection */
    std::string root_prefix;
    std::string local_config_suffix;
    if (! suffix.empty())
        local_config_suffix = "-" + suffix;

    FSEntry local_config_dir(FSEntry(getenv_with_default("PALUDIS_HOME", getenv_or_error("HOME"))) /
            (".paludis" + local_config_suffix)), old_config_dir(local_config_dir);
    if (! local_config_dir.exists())
        local_config_dir = (FSEntry(SYSCONFDIR) / ("paludis" + local_config_suffix));
    if (! local_config_dir.exists())
        throw PaludisConfigError("Can't find configuration directory (tried '"
                + stringify(old_config_dir) + "', '" + stringify(local_config_dir) + "')");

    Log::get_instance()->message(ll_debug, lc_no_context, "PaludisConfig initial directory is '"
            + stringify(local_config_dir) + "'");

    if ((local_config_dir / "specpath").exists())
    {
        KeyValueConfigFile specpath(local_config_dir / "specpath");
        root_prefix = specpath.get("root");
        local_config_suffix = specpath.get("config-suffix");

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

    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > conf_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    conf_vars->insert("ROOT", root_prefix);

    Log::get_instance()->message(ll_debug, lc_no_context, "PaludisConfig real directory is '"
            + stringify(local_config_dir) + "', root prefix is '" + root_prefix +
            "', config suffix is '" + local_config_suffix + "'");

    /* repositories */
    {
        if ((local_config_dir / "repository_defaults.conf").exists())
        {
            KeyValueConfigFile defaults_file(local_config_dir / "repository_defaults.conf");
            std::copy(defaults_file.begin(), defaults_file.end(),
                    conf_vars->inserter());
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
                    filter_inserter(std::back_inserter(repo_files), IsFileWithExtension(".conf")));
        }

        std::list<std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > > later_keys;
        for (std::list<FSEntry>::const_iterator repo_file(repo_files.begin()), repo_file_end(repo_files.end()) ;
                repo_file != repo_file_end ; ++repo_file)
        {
            Context local_context("When reading repository file '" + stringify(*repo_file) + "':");

            KeyValueConfigFile k(*repo_file, conf_vars);

            std::string format(k.get("format"));
            if (format.empty())
                throw PaludisConfigError("Key 'format' not specified or empty");

            int importance(k.get("master_repository").empty() ? 0 : 10);
            if (! k.get("importance").empty())
                importance = destringify<int>(k.get("importance"));

            std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete(k.begin(), k.end()));

            keys->erase("importance");
            keys->insert("importance", stringify(importance));

            keys->erase("repo_file");
            keys->insert("repo_file", stringify(*repo_file));

            keys->erase("root");
            keys->insert("root", root_prefix.empty() ? "/" : root_prefix);

            if (! k.get("master_repository").empty())
            {
                Log::get_instance()->message(ll_debug, lc_context, "Delaying '" + stringify(*repo_file) +
                        "' because it uses master_repository");
                later_keys.push_back(keys);
            }
            else
            {
                Log::get_instance()->message(ll_debug, lc_context, "Not delaying '" + stringify(*repo_file) + "'")
                _imp->repos.push_back(RepositoryConfigEntry(format, importance, keys));
            }
        }

        for (std::list<std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > >::const_iterator
                k(later_keys.begin()), k_end(later_keys.end()) ; k != k_end ; ++k)
            _imp->repos.push_back(RepositoryConfigEntry((*k)->find("format")->second,
                        destringify<int>((*k)->find("importance")->second), *k));

        if (_imp->repos.empty())
            throw PaludisConfigError("No repositories specified");

        /* add virtuals repositories */

        std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> > iv_keys(
                new AssociativeCollection<std::string, std::string>::Concrete);
        iv_keys->insert("root", root_prefix.empty() ? "/" : root_prefix);
        _imp->repos.push_back(RepositoryConfigEntry("installed_virtuals", -1, iv_keys));

        _imp->repos.push_back(RepositoryConfigEntry("virtuals", -2,
                    std::tr1::shared_ptr<AssociativeCollection<std::string, std::string> >()));

        _imp->repos.sort();
    }

    /* keywords */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "keywords.conf");
        if ((local_config_dir / "keywords.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "keywords.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading keywords file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
                if (tokens.empty())
                    continue;
                if ("*" == tokens.at(0))
                    std::copy(next(tokens.begin()), tokens.end(),
                            create_inserter<KeywordName>(std::back_inserter(_imp->default_keywords)));
                else if (std::string::npos == tokens.at(0).find('/'))
                {
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _imp->set_keywords.push_back(SetKeywordConfigEntry(
                                    SetName(tokens.at(0)), std::tr1::shared_ptr<DepSpec>(), KeywordName(*t)));
                }
                else
                {
                    std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _imp->keywords[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_imp->default_keywords.empty())
            throw PaludisConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    /* licenses */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "licenses.conf");
        if ((local_config_dir / "licenses.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "licenses.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading licenses file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
                if (tokens.empty())
                    continue;
                if ("*" == tokens.at(0))
                    std::copy(next(tokens.begin()), tokens.end(), std::back_inserter(_imp->default_licenses));
                else
                {
                    std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _imp->licenses[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_imp->default_licenses.empty())
            throw PaludisConfigError("No default licenses specified (a licenses.conf file should "
                    "contain an entry in the form '* license license', or '* *' if you don't want any "
                    "license filtering)");
    }

    /* user mask */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "package_mask.conf");
        if ((local_config_dir / "package_mask.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "package_mask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_mask file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                if (line->empty())
                    continue;

                if (std::string::npos == line->find('/'))
                    _imp->set_masks.push_back(SetMaskConfigEntry(SetName(*line),
                                std::tr1::shared_ptr<const DepSpec>()));
                else
                {
                    std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(*line));
                    _imp->user_masks[a->package()].push_back(a);
                }
            }
        }
    }

    /* user unmask */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "package_unmask.conf");
        if ((local_config_dir / "package_unmask.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "package_unmask.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_unmask file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                if (line->empty())
                    continue;

                if (std::string::npos == line->find('/'))
                    _imp->set_unmasks.push_back(SetMaskConfigEntry(SetName(*line),
                                std::tr1::shared_ptr<const DepSpec>()));
                else
                {
                    std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(*line));
                    _imp->user_unmasks[a->package()].push_back(a);
                }
            }
        }
    }

    /* use */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "use.conf");
        if ((local_config_dir / "use.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "use.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading use file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
                if (tokens.empty())
                    continue;

                std::string prefix;
                if ("*" == tokens.at(0))
                {
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                        {
                            if (*t == "-*")
                            {
                                _imp->default_use_prefixes_that_have_minus_star.push_back(prefix);
                                if (prefix.empty())
                                    Log::get_instance()->message(ll_warning, lc_no_context,
                                            "Using '* -*' in use.conf is dangerous. You have been warned.");
                            }
                            else
                                _imp->default_use.push_back(std::make_pair(UseFlagName(
                                                prefix + t->substr(1)), use_disabled));
                        }
                        else if (':' == t->at(t->length() - 1))
                        {
                            prefix.clear();
                            std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix),
                                    &::tolower);
                            prefix.append("_");
                        }
                        else
                            _imp->default_use.push_back(std::make_pair(UseFlagName(
                                            prefix + *t), use_enabled));
                    }
                }
                else if (std::string::npos == tokens.at(0).find('/'))
                {
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                        {
                            if ("-*" == *t)
                                _imp->set_use_prefixes_that_have_minus_star.push_back(SetUseConfigMinusStarEntry(
                                            SetName(tokens.at(0)), std::tr1::shared_ptr<const DepSpec>(), prefix));
                            else
                                _imp->set_use.push_back(SetUseConfigEntry(SetName(tokens.at(0)),
                                            std::tr1::shared_ptr<const DepSpec>(), UseFlagName(prefix + t->substr(1)), use_disabled));
                        }
                        else if (':' == t->at(t->length() - 1))
                        {
                            prefix.clear();
                            std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix),
                                    &::tolower);
                            prefix.append("_");
                        }
                        else
                            _imp->set_use.push_back(SetUseConfigEntry(SetName(tokens.at(0)),
                                        std::tr1::shared_ptr<const DepSpec>(), UseFlagName(prefix + *t), use_enabled));
                    }
                }
                else
                {
                    std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                        {
                            if ("-*" == *t)
                                _imp->use_prefixes_that_have_minus_star[a->package()].push_back(
                                        std::make_pair(a, prefix));
                            else
                                _imp->use[a->package()].push_back(UseConfigEntry(
                                            a, UseFlagName(prefix + t->substr(1)), use_disabled));
                        }
                        else if (':' == t->at(t->length() - 1))
                        {
                            prefix.clear();
                            std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix),
                                    &::tolower);
                            prefix.append("_");
                        }
                        else
                            _imp->use[a->package()].push_back(UseConfigEntry(
                                        a, UseFlagName(prefix + *t), use_enabled));
                    }
                }
            }
        }

        if (_imp->default_keywords.empty())
            throw PaludisConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    /* mirrors */
    {
        std::list<FSEntry> files;
        files.push_back(local_config_dir / "mirrors.conf");
        if ((local_config_dir / "mirrors.conf.d").exists())
            std::copy(DirIterator(local_config_dir / "mirrors.conf.d"), DirIterator(),
                    filter_inserter(std::back_inserter(files), IsFileWithExtension(".conf")));

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading mirrors file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            LineConfigFile f(*file);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> m;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(m));
                if (m.size() < 2)
                    continue;
                for (std::vector<std::string>::const_iterator mm(next(m.begin())),
                        mm_end(m.end()) ; mm != mm_end ; ++mm)
                    _imp->mirrors.insert(std::make_pair(m.at(0), *mm));
            }
        }
    }

    _imp->bashrc_files = stringify(local_config_dir / "bashrc");
}

PaludisConfig::~PaludisConfig()
{
}

std::string
PaludisConfig::bashrc_files() const
{
    return _imp->bashrc_files;
}

PaludisConfig::RepositoryIterator
PaludisConfig::begin_repositories() const
{
    return RepositoryIterator(_imp->repos.begin());
}

PaludisConfig::RepositoryIterator
PaludisConfig::end_repositories() const
{
    return RepositoryIterator(_imp->repos.end());
}

PaludisConfig::PackageKeywordsIterator
PaludisConfig::begin_package_keywords(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<std::tr1::shared_ptr<const PackageDepSpec>, KeywordName> > >::const_iterator r;
    if (_imp->keywords.end() != ((r = _imp->keywords.find(d))))
        return PackageKeywordsIterator(r->second.begin());
    else
        return PackageKeywordsIterator(_imp->empty_keywords.begin());
}

PaludisConfig::PackageKeywordsIterator
PaludisConfig::end_package_keywords(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<std::tr1::shared_ptr<const PackageDepSpec>, KeywordName> > >::const_iterator r;
    if (_imp->keywords.end() != ((r = _imp->keywords.find(d))))
        return PackageKeywordsIterator(r->second.end());
    else
        return PackageKeywordsIterator(_imp->empty_keywords.end());
}

PaludisConfig::DefaultKeywordsIterator
PaludisConfig::begin_default_keywords() const
{
    return DefaultKeywordsIterator(_imp->default_keywords.begin());
}

PaludisConfig::DefaultKeywordsIterator
PaludisConfig::end_default_keywords() const
{
    return DefaultKeywordsIterator(_imp->default_keywords.end());
}

PaludisConfig::PackageLicensesIterator
PaludisConfig::begin_package_licenses(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > >::const_iterator r;
    if (_imp->licenses.end() != ((r = _imp->licenses.find(d))))
        return PackageLicensesIterator(r->second.begin());
    else
        return PackageLicensesIterator(_imp->empty_licenses.begin());
}

PaludisConfig::PackageLicensesIterator
PaludisConfig::end_package_licenses(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > >::const_iterator r;
    if (_imp->licenses.end() != ((r = _imp->licenses.find(d))))
        return PackageLicensesIterator(r->second.end());
    else
        return PackageLicensesIterator(_imp->empty_licenses.end());
}

PaludisConfig::DefaultLicensesIterator
PaludisConfig::begin_default_licenses() const
{
    return DefaultLicensesIterator(_imp->default_licenses.begin());
}

PaludisConfig::DefaultLicensesIterator
PaludisConfig::end_default_licenses() const
{
    return DefaultLicensesIterator(_imp->default_licenses.end());
}

PaludisConfig::UserMasksIterator
PaludisConfig::begin_user_masks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator r;
    if (_imp->user_masks.end() != ((r = _imp->user_masks.find(d))))
        return UserMasksIterator(indirect_iterator<const PackageDepSpec>(r->second.begin()));
    else
        return UserMasksIterator(indirect_iterator<const PackageDepSpec>(_imp->empty_masks.begin()));
}

PaludisConfig::UserMasksIterator
PaludisConfig::end_user_masks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator r;
    if (_imp->user_masks.end() != ((r = _imp->user_masks.find(d))))
        return UserMasksIterator(indirect_iterator<const PackageDepSpec>(r->second.end()));
    else
        return UserMasksIterator(indirect_iterator<const PackageDepSpec>(_imp->empty_masks.end()));
}

PaludisConfig::UserUnmasksIterator
PaludisConfig::begin_user_unmasks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator r;
    if (_imp->user_unmasks.end() != ((r = _imp->user_unmasks.find(d))))
        return UserUnmasksIterator(indirect_iterator<const PackageDepSpec>(r->second.begin()));
    else
        return UserUnmasksIterator(indirect_iterator<const PackageDepSpec>(_imp->empty_masks.begin()));
}

PaludisConfig::UserUnmasksIterator
PaludisConfig::end_user_unmasks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::tr1::shared_ptr<const PackageDepSpec> > >::const_iterator r;
    if (_imp->user_unmasks.end() != ((r = _imp->user_unmasks.find(d))))
        return UserUnmasksIterator(indirect_iterator<const PackageDepSpec>(r->second.end()));
    else
        return UserUnmasksIterator(indirect_iterator<const PackageDepSpec>(_imp->empty_masks.end()));
}

PaludisConfig::UseConfigIterator
PaludisConfig::begin_use_config(const QualifiedPackageName & q) const
{
    std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
    if (_imp->use.end() != ((r = _imp->use.find(q))))
        return UseConfigIterator(r->second.begin());
    else
        return UseConfigIterator(_imp->empty_use.begin());
}

PaludisConfig::UseConfigIterator
PaludisConfig::end_use_config(const QualifiedPackageName & q) const
{
    std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
    if (_imp->use.end() != ((r = _imp->use.find(q))))
        return UseConfigIterator(r->second.end());
    else
        return UseConfigIterator(_imp->empty_use.end());
}

PaludisConfig::DefaultUseIterator
PaludisConfig::begin_default_use() const
{
    return DefaultUseIterator(_imp->default_use.begin());
}

PaludisConfig::DefaultUseIterator
PaludisConfig::end_default_use() const
{
    return DefaultUseIterator(_imp->default_use.end());
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

PaludisConfig::MirrorIterator
PaludisConfig::begin_mirrors(const std::string & m) const
{
    return MirrorIterator(_imp->mirrors.lower_bound(m));
}

PaludisConfig::MirrorIterator
PaludisConfig::end_mirrors(const std::string & m) const
{
    return MirrorIterator(_imp->mirrors.upper_bound(m));
}

PaludisConfig::UseMinusStarIterator
PaludisConfig::begin_use_prefixes_with_minus_star() const
{
    return UseMinusStarIterator(_imp->default_use_prefixes_that_have_minus_star.begin());
}

PaludisConfig::UseMinusStarIterator
PaludisConfig::end_use_prefixes_with_minus_star() const
{
    return UseMinusStarIterator(_imp->default_use_prefixes_that_have_minus_star.end());
}

PaludisConfig::PackageUseMinusStarIterator
PaludisConfig::begin_package_use_prefixes_with_minus_star(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > >::const_iterator r;
    if (_imp->use_prefixes_that_have_minus_star.end() != ((r = _imp->use_prefixes_that_have_minus_star.find(d))))
        return PackageUseMinusStarIterator(r->second.begin());
    else
        return PackageUseMinusStarIterator(_imp->empty_use_prefixes.begin());
}

PaludisConfig::PackageUseMinusStarIterator
PaludisConfig::end_package_use_prefixes_with_minus_star(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<std::pair<std::tr1::shared_ptr<const PackageDepSpec>, std::string> > >::const_iterator r;
    if (_imp->use_prefixes_that_have_minus_star.end() != ((r = _imp->use_prefixes_that_have_minus_star.find(d))))
        return PackageUseMinusStarIterator(r->second.end());
    else
        return PackageUseMinusStarIterator(_imp->empty_use_prefixes.end());
}

PaludisConfig::SetUseMinusStarIterator
PaludisConfig::begin_set_use_prefixes_with_minus_star() const
{
    _imp->need_sets_expanded();
    return SetUseMinusStarIterator(_imp->set_use_prefixes_that_have_minus_star.begin());
}

PaludisConfig::SetUseMinusStarIterator
PaludisConfig::end_set_use_prefixes_with_minus_star() const
{
    _imp->need_sets_expanded();
    return SetUseMinusStarIterator(_imp->set_use_prefixes_that_have_minus_star.end());
}

PaludisConfig::SetUseConfigIterator
PaludisConfig::begin_set_use_config() const
{
    _imp->need_sets_expanded();
    return SetUseConfigIterator(_imp->set_use.begin());
}

PaludisConfig::SetUseConfigIterator
PaludisConfig::end_set_use_config() const
{
    _imp->need_sets_expanded();
    return SetUseConfigIterator(_imp->set_use.end());
}

PaludisConfig::SetKeywordsIterator
PaludisConfig::begin_set_keywords() const
{
    _imp->need_sets_expanded();
    return SetKeywordsIterator(_imp->set_keywords.begin());
}

PaludisConfig::SetKeywordsIterator
PaludisConfig::end_set_keywords() const
{
    _imp->need_sets_expanded();
    return SetKeywordsIterator(_imp->set_keywords.end());
}

PaludisConfig::SetLicensesIterator
PaludisConfig::begin_set_licenses() const
{
    _imp->need_sets_expanded();
    return SetLicensesIterator(_imp->set_licenses.begin());
}

PaludisConfig::SetLicensesIterator
PaludisConfig::end_set_licenses() const
{
    _imp->need_sets_expanded();
    return SetLicensesIterator(_imp->set_licenses.end());
}

PaludisConfig::UserMasksSetsIterator
PaludisConfig::begin_user_masks_sets() const
{
    _imp->need_sets_expanded();
    return UserMasksSetsIterator(_imp->set_masks.begin());
}

PaludisConfig::UserMasksSetsIterator
PaludisConfig::end_user_masks_sets() const
{
    _imp->need_sets_expanded();
    return UserMasksSetsIterator(_imp->set_masks.end());
}

PaludisConfig::UserMasksSetsIterator
PaludisConfig::begin_user_unmasks_sets() const
{
    _imp->need_sets_expanded();
    return UserMasksSetsIterator(_imp->set_unmasks.begin());
}

PaludisConfig::UserMasksSetsIterator
PaludisConfig::end_user_unmasks_sets() const
{
    _imp->need_sets_expanded();
    return UserMasksSetsIterator(_imp->set_unmasks.end());
}

uid_t
PaludisConfig::reduced_uid() const
{
    if (! _imp->reduced_uid)
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

    return *_imp->reduced_uid;
}

gid_t
PaludisConfig::reduced_gid() const
{
    if (! _imp->reduced_gid)
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

    return *_imp->reduced_gid;
}

std::string
PaludisConfig::reduced_username() const
{
    return "paludisbuild";
}

