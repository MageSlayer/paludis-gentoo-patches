/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/options.hh>

#include <paludis/partially_made_package_dep_spec.hh>

#include <iterator>
#include <cstdlib>

#include <gtest/gtest.h>

using namespace paludis;

TEST(World, Updates)
{
    std::shared_ptr<FSPath> w(std::make_shared<FSPath>(FSPath::cwd() / "world_TEST_dir" / "world"));

    {
        TestEnvironment env;
        paludis_environment::World world(&env, w);
        world.update_config_files_for_package_move(make_package_dep_spec({ })
                .package(QualifiedPackageName("cat/before")),
                QualifiedPackageName("cat/after"));
    }

    SafeIFStream f(*w);
    std::string ff((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_EQ(
            "cat/unchanged\n"
            "cat/alsounchanged\n"
            "cat/after\n",
            ff
            );
}

