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

#include "display_profiles_use.hh"
#include "command_line.hh"
#include <output/colour.hh>
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
    display_profiles_use(const NoConfigEnvironment & env, const std::string & desc,
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

void do_display_profiles_use(NoConfigEnvironment & env)
{
    Context context("When performing display-profiles-use action:");

    std::set<UseFlagName> all_use_flags;
    {
        LineConfigFile use_desc(env.main_repository_dir() / "profiles"/ "use.desc", LineConfigFileOptions());
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

            LineConfigFile use_desc(*d, LineConfigFileOptions());
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

    if (CommandLine::get_instance()->a_profile.begin_args() ==
            CommandLine::get_instance()->a_profile.end_args())
    {
        for (RepositoryPortageInterface::ProfilesIterator p(env.main_repository()->portage_interface->begin_profiles()),
                p_end(env.main_repository()->portage_interface->end_profiles()) ; p != p_end ; ++p)
        {
            env.main_repository()->portage_interface->set_profile(p);
            display_profiles_use(env, p->arch + "." + p->status, p->path,
                    all_use_flags, all_use_expand_flags);
        }
    }
    else
    {
        for (args::StringSetArg::Iterator i(CommandLine::get_instance()->a_profile.begin_args()),
                i_end(CommandLine::get_instance()->a_profile.end_args()) ; i != i_end ; ++i)
        {
            RepositoryPortageInterface::ProfilesIterator p(
                    env.main_repository()->portage_interface->find_profile(
                        env.main_repository_dir() / "profiles" / (*i)));
            if (p == env.main_repository()->portage_interface->end_profiles())
                throw ConfigurationError("Repository does not have a profile listed in profiles.desc matching '"
                        + stringify(*i) + "'");
            env.main_repository()->portage_interface->set_profile(p);
            display_profiles_use(env, *i, env.main_repository_dir() /
                    "profiles" / *i, all_use_flags, all_use_expand_flags);
        }
    }
}


