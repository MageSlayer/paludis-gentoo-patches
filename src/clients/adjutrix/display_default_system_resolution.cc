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

#include "display_default_system_resolution.hh"
#include "command_line.hh"
#include <output/colour.hh>
#include <paludis/util/config_file.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/set.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_list.hh>
#include <paludis/dep_list_exceptions.hh>

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
        d_options.circular() = dl_circular_discard_silently;
        d_options.blocks() = dl_blocks_discard_completely;
        DepList d(&env, d_options);

        try
        {
            d.add(*env.set(SetName("system")), env.default_destinations());

            for (DepList::ConstIterator e(d.begin()), e_end(d.end()) ; e != e_end ; ++e)
                cout << "    " << *e->package_id() << endl;
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

    if (env.default_destinations()->empty())
    {
        std::tr1::shared_ptr<Repository> fake_destination(new FakeInstalledRepository(
                    make_named_values<FakeInstalledRepositoryParams>(
                        value_for<n::environment>(&env),
                        value_for<n::name>(RepositoryName("fake_destination")),
                        value_for<n::suitable_destination>(true),
                        value_for<n::supports_uninstall>(true)
                        )));
        env.package_database()->add_repository(1, fake_destination);
    }

    if (CommandLine::get_instance()->a_profile.begin_args() ==
            CommandLine::get_instance()->a_profile.end_args())
    {
        for (RepositoryEInterface::ProfilesConstIterator
                p((*env.main_repository()).e_interface()->begin_profiles()),
                p_end((*env.main_repository()).e_interface()->end_profiles()) ; p != p_end ; ++p)
        {
            (*env.main_repository()).e_interface()->set_profile(p);
            return_code |= display_default_system_resolution(env, (*p).arch() + "." + (*p).status(), (*p).path());
        }
    }
    else
    {
        for (args::StringSetArg::ConstIterator i(CommandLine::get_instance()->a_profile.begin_args()),
                i_end(CommandLine::get_instance()->a_profile.end_args()) ; i != i_end ; ++i)
        {
            RepositoryEInterface::ProfilesConstIterator
                p((*env.main_repository()).e_interface()->find_profile(
                            env.main_repository_dir() / "profiles" / (*i)));
            if (p == (*env.main_repository()).e_interface()->end_profiles())
                throw ConfigurationError("Repository does not have a profile listed in profiles.desc matching '"
                        + stringify(*i) + "'");
            (*env.main_repository()).e_interface()->set_profile(p);
            return_code |= display_default_system_resolution(env, *i, env.main_repository_dir()
                    / "profiles" / *i);
        }
    }

    return return_code;
}


