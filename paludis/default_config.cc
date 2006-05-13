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

#include <paludis/config_file.hh>
#include <paludis/default_config.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>
#include <algorithm>
#include <sstream>
#include <ctype.h>

/** \file
 * Implementation of default_config.hh classes.
 *
 * \ingroup grpdefaultconfig
 */

using namespace paludis;

DefaultConfigError::DefaultConfigError(const std::string & msg) throw () :
    ConfigurationError("Default configuration error: " + msg)
{
}

DefaultConfig::DefaultConfig() :
    _paludis_command("paludis")
{
    _config_suffix_can_be_set = false;

    Context context("When loading default configuration:");

    if (! getenv_with_default("PALUDIS_SKIP_CONFIG", "").empty())
        return;

    /* indirection */
    std::string root_prefix;
    std::string config_suffix;
    if (! _config_suffix.empty())
        config_suffix = "-" + _config_suffix;

    FSEntry config_dir(FSEntry(getenv_with_default("PALUDIS_HOME", getenv_or_error("HOME"))) /
            (".paludis" + config_suffix));
    if (! config_dir.exists())
        config_dir = (FSEntry(SYSCONFDIR) / ("paludis" + config_suffix));
    if (! config_dir.exists())
        throw DefaultConfigError("Can't find configuration directory");

    Log::get_instance()->message(ll_debug, "DefaultConfig initial directory is '"
            + stringify(config_dir) + "'");

    if ((config_dir / "specpath").exists())
    {
        KeyValueConfigFile specpath(config_dir / "specpath");
        root_prefix = specpath.get("root");
        config_suffix = specpath.get("config-suffix");

        if (! root_prefix.empty() && stringify(FSEntry(root_prefix).realpath()) != "/")
        {
            config_dir = FSEntry(root_prefix) / SYSCONFDIR / ("paludis" + config_suffix);
            if (! config_dir.exists())
                throw DefaultConfigError("Can't find configuration directory under root ("
                        "tried '" + stringify(config_dir) + "'");
        }
    }

    _root = root_prefix;
    _config_dir = stringify(config_dir);

    std::map<std::string, std::string> conf_vars;
    conf_vars.insert(std::make_pair("ROOT", root_prefix));

    Log::get_instance()->message(ll_debug, "DefaultConfig real directory is '"
            + stringify(config_dir) + "', root prefix is '" + root_prefix +
            "', config suffix is '" + config_suffix + "'");

    /* repositories */
    {
        std::list<FSEntry> dirs;
        dirs.push_back(config_dir / "repositories");

        std::list<FSEntry> repo_files;
        for (std::list<FSEntry>::const_iterator dir(dirs.begin()), dir_end(dirs.end()) ;
                dir != dir_end ; ++dir)
        {
            if (! dir->exists())
                continue;

            std::copy(DirIterator(*dir), DirIterator(),
                    filter_inserter(std::back_inserter(repo_files), IsFileWithExtension(".conf")));
        }

        for (std::list<FSEntry>::const_iterator repo_file(repo_files.begin()), repo_file_end(repo_files.end()) ;
                repo_file != repo_file_end ; ++repo_file)
        {
            Context local_context("When reading repository file '" + stringify(*repo_file) + "':");

            KeyValueConfigFile k(*repo_file, conf_vars);

            std::string format(k.get("format"));
            if (format.empty())
                throw DefaultConfigError("Key 'format' not specified or empty");

            int importance(0);
            if (! k.get("importance").empty())
                importance = destringify<int>(k.get("importance"));

            std::map<std::string, std::string> keys(k.begin(), k.end());
            keys["repo_file"] = stringify(*repo_file);
            keys["root"] = root_prefix;
            _repos.push_back(RepositoryConfigEntry(format, importance, keys));
        }

        if (_repos.empty())
            throw DefaultConfigError("No repositories specified");

        _repos.sort();
    }

    /* keywords */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "keywords.conf");

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
                            create_inserter<KeywordName>(std::back_inserter(_default_keywords)));
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _keywords[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_default_keywords.empty())
            throw DefaultConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    /* licenses */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "licenses.conf");

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
                    std::copy(next(tokens.begin()), tokens.end(), std::back_inserter(_default_licenses));
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _licenses[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_default_licenses.empty())
            throw DefaultConfigError("No default licenses specified (a licenses.conf file should "
                    "contain an entry in the form '* license license', or '* *' if you don't want any "
                    "license filtering)");
    }

    /* user mask */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "package_mask.conf");

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
                PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
                _user_masks[a->package()].push_back(a);
            }
        }
    }

    /* user unmask */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "package_unmask.conf");

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
                PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
                _user_unmasks[a->package()].push_back(a);
            }
        }
    }

    /* use */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "use.conf");

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
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                            _default_use.push_back(std::make_pair(UseFlagName(
                                            prefix + t->substr(1)), use_disabled));
                        else if (':' == t->at(t->length() - 1))
                        {
                            prefix.clear();
                            std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix),
                                    &::tolower);
                            prefix.append("_");
                        }
                        else
                            _default_use.push_back(std::make_pair(UseFlagName(
                                            prefix + *t), use_enabled));
                    }
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                            _use[a->package()].push_back(UseConfigEntry(
                                        a, UseFlagName(prefix + t->substr(1)), use_disabled));
                        else if (':' == t->at(t->length() - 1))
                        {
                            prefix.clear();
                            std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix),
                                    &::tolower);
                            prefix.append("_");
                        }
                        else
                            _use[a->package()].push_back(UseConfigEntry(
                                        a, UseFlagName(prefix + *t), use_enabled));
                    }
                }
            }
        }

        if (_default_keywords.empty())
            throw DefaultConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    _bashrc_files = stringify(config_dir / "bashrc");
}

DefaultConfig::~DefaultConfig()
{
}

std::string DefaultConfig::_config_suffix;
bool DefaultConfig::_config_suffix_can_be_set(true);

void
DefaultConfig::set_config_suffix(const std::string & s)
{
    if (! _config_suffix_can_be_set)
        throw InternalError(PALUDIS_HERE, "DefaultConfig::set_config_suffix called after "
                "DefaultConfig has been instantiated.");

    static const std::string allowed_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_+:");

    if (std::string::npos != s.find_first_not_of(allowed_chars))
        throw DefaultConfigError("Invalid config suffix '" + s + "'");

    if (! s.empty())
        if ('-' == s.at(0) || '-' == s.at(s.length() - 1))
            throw DefaultConfigError("Invalid config suffix '" + s + "'");

    _config_suffix = s;
}

std::string
DefaultConfig::bashrc_files() const
{
    return _bashrc_files;
}

