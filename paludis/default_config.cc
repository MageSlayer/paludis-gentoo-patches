/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "default_config.hh"
#include "fs_entry.hh"
#include "dir_iterator.hh"
#include "getenv.hh"
#include "key_value_config_file.hh"
#include "default_config_error.hh"
#include "filter_insert_iterator.hh"
#include "is_file_with_extension.hh"
#include "stringify.hh"
#include "tokeniser.hh"
#include "line_config_file.hh"
#include "create_insert_iterator.hh"
#include <fstream>
#include <algorithm>

using namespace paludis;

DefaultConfig::DefaultConfig()
{
    Context context("When loading default configuration:");

    Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");

    /* repositories */
    {
        std::list<FSEntry> dirs;
        if (! getenv_with_default("PALUDIS_CONFIG_DIR", "").empty())
            dirs.push_back(FSEntry(getenv_or_error("PALUDIS_CONFIG_DIR")) / "repositories");
        else
        {
            dirs.push_back(FSEntry(getenv_with_default("ROOT", "/") + "" SYSCONFDIR)
                    / "paludis" / "repositories");
            dirs.push_back(FSEntry(getenv_or_error("HOME")) / ".paludis" / "repositories");
        }

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

            std::ifstream kf(stringify(*repo_file).c_str());
            if (! kf)
                throw DefaultConfigError("Couldn't open " + stringify(*repo_file));

            KeyValueConfigFile k(&kf);

            if (k.get("location").empty())
                throw DefaultConfigError("Key 'location' empty or not specified in " +
                        stringify(*repo_file));
            if (k.get("format").empty())
                throw DefaultConfigError("Key 'format' empty or not specified in " +
                        stringify(*repo_file));
            if (k.get("profile").empty())
                throw DefaultConfigError("Key 'profile' empty or not specified in " +
                        stringify(*repo_file));

            _repos.push_back(RepositoryConfigEntry(k.get("location"),
                        k.get("profile"), k.get("format")));
        }

        if (_repos.empty())
            throw DefaultConfigError("No repositories specified");
    }

    /* keywords */
    {
        std::list<FSEntry> files;
        if (! getenv_with_default("PALUDIS_CONFIG_DIR", "").empty())
            files.push_back(FSEntry(getenv_or_error("PALUDIS_CONFIG_DIR")) / "keywords.conf");
        else
        {
            files.push_back(FSEntry(getenv_with_default("ROOT", "/") + "" SYSCONFDIR)
                    / "paludis" / "keywords.conf");
            files.push_back(FSEntry(getenv_or_error("HOME")) / ".paludis" / "keywords.conf");
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading keywords file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            std::ifstream kf(stringify(*file).c_str());
            if (! kf)
                throw DefaultConfigError("Couldn't open " + stringify(*file));

            LineConfigFile f(&kf);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                tokeniser.tokenise(*line, std::back_inserter(tokens));
                if (tokens.empty())
                    continue;
                if ("*" == tokens.at(0))
                {
                    _default_keywords.clear();
                    std::copy(++(tokens.begin()), tokens.end(),
                            create_inserter<KeywordName>(std::back_inserter(_default_keywords)));
                }
                else
                {
                    PackageDepAtom::ConstPointer a(new PackageDepAtom(tokens.at(0)));
                    for (std::vector<std::string>::const_iterator t(++(tokens.begin())), t_end(tokens.end()) ;
                            t != t_end ; ++t)
                        _keywords[a->package()].push_back(std::make_pair(a, *t));
                }
            }
        }

        if (_default_keywords.empty())
            throw DefaultConfigError("No default keywords specified (a keywords.conf file should "
                    "contain an entry in the form '* keyword')");
    }

    /* user mask */
    {
        std::list<FSEntry> files;
        if (! getenv_with_default("PALUDIS_CONFIG_DIR", "").empty())
            files.push_back(FSEntry(getenv_or_error("PALUDIS_CONFIG_DIR")) / "package_mask.conf");
        else
        {
            files.push_back(FSEntry(getenv_with_default("ROOT", "/") + "" SYSCONFDIR)
                    / "paludis" / "package_mask.conf");
            files.push_back(FSEntry(getenv_or_error("HOME")) / ".paludis" / "package_mask.conf");
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_mask file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            std::ifstream kf(stringify(*file).c_str());
            if (! kf)
                throw DefaultConfigError("Couldn't open " + stringify(*file));

            LineConfigFile f(&kf);
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
        if (! getenv_with_default("PALUDIS_CONFIG_DIR", "").empty())
            files.push_back(FSEntry(getenv_or_error("PALUDIS_CONFIG_DIR")) / "package_unmask.conf");
        else
        {
            files.push_back(FSEntry(getenv_with_default("ROOT", "/") + "" SYSCONFDIR)
                    / "paludis" / "package_unmask.conf");
            files.push_back(FSEntry(getenv_or_error("HOME")) / ".paludis" / "package_unmask.conf");
        }

        for (std::list<FSEntry>::const_iterator file(files.begin()), file_end(files.end()) ;
                file != file_end ; ++file)
        {
            Context local_context("When reading package_unmask file '" + stringify(*file) + "':");

            if (! file->is_regular_file())
                continue;

            std::ifstream kf(stringify(*file).c_str());
            if (! kf)
                throw DefaultConfigError("Couldn't open " + stringify(*file));

            LineConfigFile f(&kf);
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                PackageDepAtom::ConstPointer a(new PackageDepAtom(*line));
                _user_unmasks[a->package()].push_back(a);
            }
        }
    }
}

