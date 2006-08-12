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

/** \file
 * Implementation of default_config.hh classes.
 *
 * \ingroup grpdefaultconfig
 */

using namespace paludis;

#include <paludis/repository_config_entry-sr.cc>
#include <paludis/use_config_entry-sr.cc>

namespace paludis
{
    /**
     * Implementation data for DefaultConfig.
     *
     * \ingroup grpdefaultconfig
     */
    template<>
    struct Implementation<DefaultConfig> :
        InternalCounted<DefaultConfig>,
        InstantiationPolicy<DefaultConfig, instantiation_method::NonCopyableTag>
    {
        static std::string config_suffix;
        static bool config_suffix_can_be_set;
        std::string paludis_command;
        std::string root;
        std::string config_dir;
        std::string bashrc_files;

        std::list<RepositoryConfigEntry> repos;

        std::map<QualifiedPackageName, std::vector<
            std::pair<PackageDepAtom::ConstPointer, KeywordName> > > keywords;

        const std::vector<std::pair<PackageDepAtom::ConstPointer, KeywordName> > empty_keywords;

        std::vector<KeywordName> default_keywords;

        std::map<QualifiedPackageName, std::vector<
            std::pair<PackageDepAtom::ConstPointer, std::string> > > licenses;

        const std::vector<std::pair<PackageDepAtom::ConstPointer, std::string> > empty_licenses;

        std::vector<std::string> default_licenses;

        std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > user_masks;

        std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> > user_unmasks;

        std::vector<PackageDepAtom::ConstPointer> empty_masks;

        std::map<QualifiedPackageName, std::vector<UseConfigEntry> > use;

        std::vector<UseConfigEntry> empty_use;

        std::vector<std::pair<UseFlagName, UseFlagState> > default_use;

        std::multimap<std::string, std::string> mirrors;

        Implementation();
    };

    Implementation<DefaultConfig>::Implementation() :
        paludis_command("paludis")
    {
    }

    std::string Implementation<DefaultConfig>::config_suffix;
    bool Implementation<DefaultConfig>::config_suffix_can_be_set(true);
}

DefaultConfigError::DefaultConfigError(const std::string & msg) throw () :
    ConfigurationError("Default configuration error: " + msg)
{
}

DefaultConfig::DefaultConfig() :
    PrivateImplementationPattern<DefaultConfig>(new Implementation<DefaultConfig>)
{
    Context context("When loading default configuration:");

    if (! getenv_with_default("PALUDIS_SKIP_CONFIG", "").empty())
    {
        _imp->config_suffix_can_be_set = false;
        return;
    }

    /* indirection */
    std::string root_prefix;
    std::string config_suffix;
    if (! _imp->config_suffix.empty())
        config_suffix = "-" + _imp->config_suffix;

    FSEntry config_dir(FSEntry(getenv_with_default("PALUDIS_HOME", getenv_or_error("HOME"))) /
            (".paludis" + config_suffix));
    if (! config_dir.exists())
        config_dir = (FSEntry(SYSCONFDIR) / ("paludis" + config_suffix));
    if (! config_dir.exists())
        throw DefaultConfigError("Can't find configuration directory");

    Log::get_instance()->message(ll_debug, lc_no_context, "DefaultConfig initial directory is '"
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

    _imp->root = root_prefix;
    _imp->config_dir = stringify(config_dir);

    AssociativeCollection<std::string, std::string>::Pointer conf_vars(
            new AssociativeCollection<std::string, std::string>::Concrete);
    conf_vars->insert("ROOT", root_prefix);

    Log::get_instance()->message(ll_debug, lc_no_context, "DefaultConfig real directory is '"
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

            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete(k.begin(), k.end()));

            keys->erase("repo_file");
            keys->insert("repo_file", stringify(*repo_file));

            keys->erase("root");
            keys->insert("root", root_prefix);

            _imp->repos.push_back(RepositoryConfigEntry(format, importance, keys));
        }

        if (_imp->repos.empty())
            throw DefaultConfigError("No repositories specified");

        _imp->repos.sort();
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
                            create_inserter<KeywordName>(std::back_inserter(_imp->default_keywords)));
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _imp->keywords[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_imp->default_keywords.empty())
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
                    std::copy(next(tokens.begin()), tokens.end(), std::back_inserter(_imp->default_licenses));
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _imp->licenses[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_imp->default_licenses.empty())
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
                _imp->user_masks[a->package()].push_back(a);
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
                _imp->user_unmasks[a->package()].push_back(a);
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
                            _imp->default_use.push_back(std::make_pair(UseFlagName(
                                            prefix + t->substr(1)), use_disabled));
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
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                    {
                        if ('-' == t->at(0))
                            _imp->use[a->package()].push_back(UseConfigEntry(
                                        a, UseFlagName(prefix + t->substr(1)), use_disabled));
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
            throw DefaultConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    /* mirrors */
    {
        std::list<FSEntry> files;
        files.push_back(config_dir / "mirrors.conf");

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

    _imp->bashrc_files = stringify(config_dir / "bashrc");

    _imp->config_suffix_can_be_set = false;
}

DefaultConfig::~DefaultConfig()
{
}

void
DefaultConfig::set_config_suffix(const std::string & s)
{
    if (! Implementation<DefaultConfig>::config_suffix_can_be_set)
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

    Implementation<DefaultConfig>::config_suffix = s;
}

std::string
DefaultConfig::bashrc_files() const
{
    return _imp->bashrc_files;
}

std::string
DefaultConfig::config_suffix()
{
    return Implementation<DefaultConfig>::config_suffix;
}

DefaultConfig::RepositoryIterator
DefaultConfig::begin_repositories() const
{
    return RepositoryIterator(_imp->repos.begin());
}

DefaultConfig::RepositoryIterator
DefaultConfig::end_repositories() const
{
    return RepositoryIterator(_imp->repos.end());
}

DefaultConfig::PackageKeywordsIterator
DefaultConfig::begin_package_keywords(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
    if (_imp->keywords.end() != ((r = _imp->keywords.find(d))))
        return PackageKeywordsIterator(r->second.begin());
    else
        return PackageKeywordsIterator(_imp->empty_keywords.begin());
}

DefaultConfig::PackageKeywordsIterator
DefaultConfig::end_package_keywords(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<PackageDepAtom::ConstPointer, KeywordName> > >::const_iterator r;
    if (_imp->keywords.end() != ((r = _imp->keywords.find(d))))
        return PackageKeywordsIterator(r->second.end());
    else
        return PackageKeywordsIterator(_imp->empty_keywords.end());
}

DefaultConfig::DefaultKeywordsIterator
DefaultConfig::begin_default_keywords() const
{
    return DefaultKeywordsIterator(_imp->default_keywords.begin());
}

DefaultConfig::DefaultKeywordsIterator
DefaultConfig::end_default_keywords() const
{
    return DefaultKeywordsIterator(_imp->default_keywords.end());
}

DefaultConfig::PackageLicensesIterator
DefaultConfig::begin_package_licenses(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<PackageDepAtom::ConstPointer, std::string> > >::const_iterator r;
    if (_imp->licenses.end() != ((r = _imp->licenses.find(d))))
        return PackageLicensesIterator(r->second.begin());
    else
        return PackageLicensesIterator(_imp->empty_licenses.begin());
}

DefaultConfig::PackageLicensesIterator
DefaultConfig::end_package_licenses(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<
        std::pair<PackageDepAtom::ConstPointer, std::string> > >::const_iterator r;
    if (_imp->licenses.end() != ((r = _imp->licenses.find(d))))
        return PackageLicensesIterator(r->second.end());
    else
        return PackageLicensesIterator(_imp->empty_licenses.end());
}

DefaultConfig::DefaultLicensesIterator
DefaultConfig::begin_default_licenses() const
{
    return DefaultLicensesIterator(_imp->default_licenses.begin());
}

DefaultConfig::DefaultLicensesIterator
DefaultConfig::end_default_licenses() const
{
    return DefaultLicensesIterator(_imp->default_licenses.end());
}

DefaultConfig::UserMasksIterator
DefaultConfig::begin_user_masks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
    if (_imp->user_masks.end() != ((r = _imp->user_masks.find(d))))
        return UserMasksIterator(indirect_iterator<const PackageDepAtom>(r->second.begin()));
    else
        return UserMasksIterator(indirect_iterator<const PackageDepAtom>(_imp->empty_masks.begin()));
}

DefaultConfig::UserMasksIterator
DefaultConfig::end_user_masks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
    if (_imp->user_masks.end() != ((r = _imp->user_masks.find(d))))
        return UserMasksIterator(indirect_iterator<const PackageDepAtom>(r->second.end()));
    else
        return UserMasksIterator(indirect_iterator<const PackageDepAtom>(_imp->empty_masks.end()));
}

DefaultConfig::UserUnmasksIterator
DefaultConfig::begin_user_unmasks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
    if (_imp->user_unmasks.end() != ((r = _imp->user_unmasks.find(d))))
        return UserUnmasksIterator(indirect_iterator<const PackageDepAtom>(r->second.begin()));
    else
        return UserUnmasksIterator(indirect_iterator<const PackageDepAtom>(_imp->empty_masks.begin()));
}

DefaultConfig::UserUnmasksIterator
DefaultConfig::end_user_unmasks(const QualifiedPackageName & d) const
{
    std::map<QualifiedPackageName, std::vector<PackageDepAtom::ConstPointer> >::const_iterator r;
    if (_imp->user_unmasks.end() != ((r = _imp->user_unmasks.find(d))))
        return UserUnmasksIterator(indirect_iterator<const PackageDepAtom>(r->second.end()));
    else
        return UserUnmasksIterator(indirect_iterator<const PackageDepAtom>(_imp->empty_masks.end()));
}

DefaultConfig::UseConfigIterator
DefaultConfig::begin_use_config(const QualifiedPackageName & q) const
{
    std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
    if (_imp->use.end() != ((r = _imp->use.find(q))))
        return UseConfigIterator(r->second.begin());
    else
        return UseConfigIterator(_imp->empty_use.begin());
}

DefaultConfig::UseConfigIterator
DefaultConfig::end_use_config(const QualifiedPackageName & q) const
{
    std::map<QualifiedPackageName, std::vector<UseConfigEntry> >::const_iterator r;
    if (_imp->use.end() != ((r = _imp->use.find(q))))
        return UseConfigIterator(r->second.end());
    else
        return UseConfigIterator(_imp->empty_use.end());
}

DefaultConfig::DefaultUseIterator
DefaultConfig::begin_default_use() const
{
    return DefaultUseIterator(_imp->default_use.begin());
}

DefaultConfig::DefaultUseIterator
DefaultConfig::end_default_use() const
{
    return DefaultUseIterator(_imp->default_use.end());
}

std::string
DefaultConfig::paludis_command() const
{
    return _imp->paludis_command;
}

void
DefaultConfig::set_paludis_command(const std::string & s)
{
    _imp->paludis_command = s;
}

std::string
DefaultConfig::root() const
{
    return _imp->root;
}

std::string
DefaultConfig::config_dir() const
{
    return _imp->config_dir;
}

DefaultConfig::MirrorIterator
DefaultConfig::begin_mirrors(const std::string & m) const
{
    return MirrorIterator(_imp->mirrors.lower_bound(m));
}

DefaultConfig::MirrorIterator
DefaultConfig::end_mirrors(const std::string & m) const
{
    return MirrorIterator(_imp->mirrors.upper_bound(m));
}

