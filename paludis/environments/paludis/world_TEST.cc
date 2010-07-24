/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/environments/paludis/world.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/options.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <iterator>
#include <cstdlib>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TestWorldUpdates : TestCase
    {
        TestWorldUpdates() : TestCase("world updates") { }

        void run()
        {
            std::shared_ptr<FSEntry> w(std::make_shared<FSEntry>(FSEntry::cwd() / "world_TEST_dir" / "world"));

            {
                TestEnvironment env;
                paludis_environment::World world(&env, w);
                world.update_config_files_for_package_move(make_package_dep_spec({ })
                        .package(QualifiedPackageName("cat/before")),
                        QualifiedPackageName("cat/after"));
            }

            SafeIFStream f(*w);
            std::string ff((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(ff,
                    "cat/unchanged\n"
                    "cat/alsounchanged\n"
                    "cat/after\n"
                    );
        }
    } world_updates_test;
}

