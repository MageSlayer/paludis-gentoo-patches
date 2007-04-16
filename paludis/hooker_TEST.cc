/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/hooker.hh>
#include <paludis/environments/test/test_environment.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <fstream>
#include <iterator>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct HookerTest : TestCase
    {
        HookerTest() : TestCase("hooker") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("simple_hook")), 3);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("fancy_hook")), 5);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("several_hooks")), 7);
        }
    } test_hooker;

    struct HookerOrderingTest : TestCase
    {
        HookerOrderingTest() : TestCase("hooker ordering") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);

            FSEntry("hooker_TEST_dir/ordering.out").unlink();

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("ordering")), 0);

            std::ifstream f(stringify(FSEntry("hooker_TEST_dir/ordering.out")).c_str());
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "e\nc\nf\nd\nb\na\ng\ni\nh\nk\nj\n");
        }
    } test_hooker_ordering;

    struct HookerBadHooksTest : TestCase
    {
        HookerBadHooksTest() : TestCase("hooker bad hooks") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);

            FSEntry("hooker_TEST_dir/bad_hooks.out").unlink();

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("bad_hooks")), 123);

            std::ifstream f(stringify(FSEntry("hooker_TEST_dir/bad_hooks.out")).c_str());
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "one\nthree\n");
        }
    } test_hooker_bad_hooks;

    struct HookerCyclesTest : TestCase
    {
        HookerCyclesTest() : TestCase("hooker cycles") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);

            FSEntry("hooker_TEST_dir/cycles.out").unlink();

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);
            TEST_CHECK_EQUAL(hooker.perform_hook(Hook("cycles")), 0);

            std::ifstream f(stringify(FSEntry("hooker_TEST_dir/cycles.out")).c_str());
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "b\na\ng\nf\ni\n");
        }
    } test_hooker_cycles;
}

