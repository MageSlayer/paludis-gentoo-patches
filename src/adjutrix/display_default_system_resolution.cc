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

#include "display_default_system_resolution.hh"
#include "command_line.hh"
#include "colour.hh"
#include <paludis/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>
#include <paludis/dep_list.hh>

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
    int
    display_default_system_resolution(const NoConfigEnvironment & env, const std::string & desc,
            const FSEntry & profile)
    {
        int return_code(0);

        Context context("When displaying system resolution for '" + stringify(desc) + "' at '"
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

        DepListOptions d_options;
        d_options.circular = dl_circular_discard;
        DepList d(&env, d_options);

        try
        {
            d.add(env.package_set("system"));

            for (DepList::Iterator e(d.begin()), e_end(d.end()) ; e != e_end ; ++e)
                cout << "    " << e->package << ":" << e->metadata->slot << endl;
        }
        catch (const NoSuchPackageError & e)
        {
            cout << endl;
            cerr << "Query error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ");
            cerr << "No such package '" << e.name() << "'" << endl;

            return_code |= 1;
        }
        catch (const DepListError & e)
        {
            cout << endl;
            cerr << "Dependency error:" << endl;
            cerr << "  * " << e.backtrace("\n  * ") << e.message() << " ("
                << e.what() << ")" << endl;
            cerr << endl;

            return_code |= 1;
        }

        cout << endl;

        return return_code;
    }
}

int do_display_default_system_resolution(NoConfigEnvironment & env)
{
    int return_code(0);

    Context context("When performing display-default-system-resolution action:");

    if (CommandLine::get_instance()->a_profile.args_begin() ==
            CommandLine::get_instance()->a_profile.args_end())
    {
        for (NoConfigEnvironment::ProfilesIterator p(env.begin_profiles()), p_end(env.end_profiles()) ;
                p != p_end ; ++p)
        {
            env.set_profile(p->path);
            return_code |= display_default_system_resolution(env, p->arch + "." + p->status, p->path);
        }
    }
    else
    {
        for (args::StringSetArg::Iterator i(CommandLine::get_instance()->a_profile.args_begin()),
                i_end(CommandLine::get_instance()->a_profile.args_end()) ; i != i_end ; ++i)
        {
            env.set_profile(env.main_repository_dir() / "profiles" / (*i));
            return_code |= display_default_system_resolution(env, *i, env.main_repository_dir() / "profiles" / *i);
        }
    }

    return return_code;
}


