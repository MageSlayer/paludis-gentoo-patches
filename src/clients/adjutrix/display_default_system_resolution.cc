/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/legacy/dep_list.hh>
#include <paludis/legacy/dep_list_exceptions.hh>

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
    display_default_system_resolution(const NoConfigEnvironment & env)
    {
        int return_code(0);

        Context context("When displaying system resolution:");

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
    Context context("When performing display-default-system-resolution action:");

    if (env.default_destinations()->empty())
    {
        std::shared_ptr<Repository> fake_destination(std::make_shared<FakeInstalledRepository>(
                    make_named_values<FakeInstalledRepositoryParams>(
                        n::environment() = &env,
                        n::name() = RepositoryName("fake_destination"),
                        n::suitable_destination() = true,
                        n::supports_uninstall() = true
                        )));
        env.package_database()->add_repository(1, fake_destination);
    }

    return display_default_system_resolution(env);
}


