/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "display_profiles_use.hh"
#include "command_line.hh"
#include "colour.hh"
#include <paludis/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <set>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

using namespace paludis;

namespace
{
    std::string
    upperify(const std::string & s)
    {
        std::string result;
        std::transform(s.begin(), s.end(), std::back_inserter(result), &::toupper);
        return result;
    }

    void
    display_profiles_use(const AdjutrixEnvironment & env, const std::string & desc,
            const FSEntry & profile, const std::set<UseFlagName> & all_use,
            const std::multimap<std::string, UseFlagName> & all_use_expand_flags)
    {
        Context context("When displaying profile use for '" + stringify(desc) + "' at '"
                + stringify(profile) + "':");

        std::string display_profile(stringify(profile)), display_profile_chop(
                stringify(env.main_repository_dir() / "profiles"));
        if (0 == display_profile.compare(0, display_profile_chop.length(), display_profile_chop))
        {
            display_profile.erase(0, display_profile_chop.length());
            if (0 == display_profile.compare(0, 1, "/"))
                display_profile.erase(0, 1);
            if (display_profile.empty())
                display_profile = "/";
        }

        cout << std::left << std::setw(20) << (desc + ":") << display_profile << endl;

        cout << std::setw(20) << "USE:";

        PackageDatabaseEntry e(QualifiedPackageName("dummy-category/dummy-package"), VersionSpec("0"),
                env.package_database()->favourite_repository());
        for (std::set<UseFlagName>::const_iterator u(all_use.begin()), u_end(all_use.end()) ;
                u != u_end ; ++u)
            if (env.query_use(*u, &e))
                cout << *u << " ";

        std::string current_prefix("not on a boat");
        for (std::multimap<std::string, UseFlagName>::const_iterator u(all_use_expand_flags.begin()),
                u_end(all_use_expand_flags.end()) ; u != u_end ; ++u)
        {
            if (u->first != current_prefix)
                cout << endl << std::setw(20) << (upperify(stringify(u->first)) + ":");
            current_prefix = u->first;

            if (env.query_use(UseFlagName(current_prefix + "_" + stringify(u->second)), &e))
                cout << u->second << " ";
        }

        cout << endl << endl;
    }
}

void do_display_profiles_use(AdjutrixEnvironment & env)
{
    Context context("When performing display-profiles-use action:");

    std::set<UseFlagName> all_use_flags;
    {
        LineConfigFile use_desc(env.main_repository_dir() / "profiles"/ "use.desc");
        for (LineConfigFile::Iterator line(use_desc.begin()), line_end(use_desc.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

            if (tokens.size() < 2)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping invalid line '"
                        + *line + "'");
                continue;
            }

            all_use_flags.insert(UseFlagName(tokens.at(0)));
        }
    }

    std::multimap<std::string, UseFlagName> all_use_expand_flags;
    {
        for (DirIterator d(env.main_repository_dir() / "profiles" / "desc"), d_end ;
                d != d_end ; ++d)
        {
            if (! IsFileWithExtension(".desc")(*d))
                continue;

            std::string prefix(strip_trailing_string(d->basename(), ".desc"));

            LineConfigFile use_desc(*d);
            for (LineConfigFile::Iterator line(use_desc.begin()), line_end(use_desc.end()) ;
                    line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

                if (tokens.size() < 2)
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Skipping invalid line '"
                            + *line + "'");
                    continue;
                }

                all_use_expand_flags.insert(std::make_pair(prefix, UseFlagName(tokens.at(0))));
            }
        }
    }

    LineConfigFile profiles_desc(env.main_repository_dir() / "profiles" / "profiles.desc");
    {
        for (LineConfigFile::Iterator line(profiles_desc.begin()), line_end(profiles_desc.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

            if (tokens.size() != 3)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping invalid line '"
                        + *line + "'");
                continue;
            }

            env.set_profile(env.main_repository_dir() / "profiles" / tokens.at(1));
            display_profiles_use(env, tokens.at(0) + "." + tokens.at(2), env.main_repository_dir() /
                    "profiles" / tokens.at(1), all_use_flags, all_use_expand_flags);
        }
    }
}


