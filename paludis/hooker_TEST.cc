/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "config.h"
#include <paludis/hooker.hh>
#include <paludis/hook.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
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
            HookResult result(make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = ""));

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);
#ifdef ENABLE_PYTHON_HOOKS
            result = hooker.perform_hook(Hook("py_hook"),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
#endif
            result = hooker.perform_hook(Hook("simple_hook"),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 3);
            TEST_CHECK_EQUAL(result.output(), "");
            result = hooker.perform_hook(Hook("fancy_hook"),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 5);
            TEST_CHECK_EQUAL(result.output(), "");
            result = hooker.perform_hook(Hook("so_hook"),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 6);
            TEST_CHECK_EQUAL(result.output(), "");
            result = hooker.perform_hook(Hook("several_hooks"),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 7);
            TEST_CHECK_EQUAL(result.output(), "");

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
            HookResult result(hooker.perform_hook(Hook("ordering"),
                        make_null_shared_ptr()));
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "");

            SafeIFStream f(FSEntry("hooker_TEST_dir/ordering.out"));
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

#ifdef ENABLE_PYTHON_HOOKS
            TEST_CHECK_EQUAL(line, "e\nc\nf\nd\nb\na\npy_hook\ng\ni\nh\nsohook\nk\nj\n");
#else
            TEST_CHECK_EQUAL(line, "e\nc\nf\nd\nb\na\ng\ni\nh\nsohook\nk\nj\n");
#endif

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
            HookResult result(hooker.perform_hook(Hook("bad_hooks"),
                        make_null_shared_ptr()));
            TEST_CHECK_EQUAL(result.max_exit_status(), 123);
            TEST_CHECK_EQUAL(result.output(), "");

            SafeIFStream f(FSEntry("hooker_TEST_dir/bad_hooks.out"));
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
            HookResult result(hooker.perform_hook(Hook("cycles"),
                        make_null_shared_ptr()));
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "");

            SafeIFStream f(FSEntry("hooker_TEST_dir/cycles.out"));
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "b\na\ng\nf\ni\n");
        }
    } test_hooker_cycles;

    struct HookerOutputTest : TestCase
    {
        HookerOutputTest() : TestCase("hooker output") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);
            HookResult result(make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = ""));

            FSEntry("hooker_TEST_dir/several_output.out").unlink();

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);

            result = hooker.perform_hook(Hook("simple_hook_output")
                    .grab_output(Hook::AllowedOutputValues()("foo")),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "foo");

            result = hooker.perform_hook(Hook("fancy_hook_output")
                    .grab_output(Hook::AllowedOutputValues()("foo")),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "foo");

            result = hooker.perform_hook(Hook("so_hook_output")
                     .grab_output(Hook::AllowedOutputValues()("foo")),
                     make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "foo");

#ifdef ENABLE_PYTHON_HOOKS
            result = hooker.perform_hook(Hook("py_hook_output")
                     .grab_output(Hook::AllowedOutputValues()("foo")),
                     make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "foo");
#endif

            result = hooker.perform_hook(Hook("several_hooks_output")
                    .grab_output(Hook::AllowedOutputValues()),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "one");

            result = hooker.perform_hook(Hook("several_hooks_output")
                    .grab_output(Hook::AllowedOutputValues()("one")),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "one");

            result = hooker.perform_hook(Hook("several_hooks_output")
                    .grab_output(Hook::AllowedOutputValues()("two")("three")),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);
            TEST_CHECK_EQUAL(result.output(), "two");

            result = hooker.perform_hook(Hook("several_hooks_output")
                    .grab_output(Hook::AllowedOutputValues()("blah")),
                    make_null_shared_ptr());
            TEST_CHECK_EQUAL(result.output(), "");
            TEST_CHECK_EQUAL(result.max_exit_status(), 0);

            SafeIFStream f(FSEntry("hooker_TEST_dir/several_output.out"));
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "one\none\none\ntwo\none\ntwo\nthree\n");

        }
    } test_hooker_output;

    struct HookerBadOutputHooksTest : TestCase
    {
        HookerBadOutputHooksTest() : TestCase("hooker bad ouput hooks") { }

        void run()
        {
            TestEnvironment env;
            Hooker hooker(&env);

            FSEntry("hooker_TEST_dir/several_output_bad.out").unlink();

            hooker.add_dir(FSEntry("hooker_TEST_dir/"), false);

            HookResult result(hooker.perform_hook(Hook("several_hooks_output_bad")
                        .grab_output(Hook::AllowedOutputValues()),
                        make_null_shared_ptr()));
            TEST_CHECK_EQUAL(result.output(), "two");
            TEST_CHECK_EQUAL(result.max_exit_status(), 99);

            SafeIFStream f(FSEntry("hooker_TEST_dir/several_output_bad.out"));
            std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            TEST_CHECK_EQUAL(line, "one\ntwo\nthree\n");
        }
    } test_hooker_bad_output_hooks;
}

