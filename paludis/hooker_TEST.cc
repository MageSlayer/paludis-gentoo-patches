/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/hook.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <iterator>

#include <gtest/gtest.h>

#include "config.h"

using namespace paludis;

TEST(Hooker, Works)
{
    TestEnvironment env;
    Hooker hooker(&env);
    HookResult result(make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = ""));

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);
#ifdef ENABLE_PYTHON_HOOKS
    result = hooker.perform_hook(Hook("py_hook"),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
#endif
    result = hooker.perform_hook(Hook("simple_hook"),
            make_null_shared_ptr());
    EXPECT_EQ(3, result.max_exit_status());
    EXPECT_EQ("", result.output());
    result = hooker.perform_hook(Hook("fancy_hook"),
            make_null_shared_ptr());
    EXPECT_EQ(5, result.max_exit_status());
    EXPECT_EQ("", result.output());
    result = hooker.perform_hook(Hook("so_hook"),
            make_null_shared_ptr());
    EXPECT_EQ(6, result.max_exit_status());
    EXPECT_EQ("", result.output());
    result = hooker.perform_hook(Hook("several_hooks"),
            make_null_shared_ptr());
    EXPECT_EQ(7, result.max_exit_status());
    EXPECT_EQ("", result.output());

}

TEST(Hooker, Ordering)
{
    TestEnvironment env;
    Hooker hooker(&env);

    FSPath("hooker_TEST_dir/ordering.out").unlink();

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);
    HookResult result(hooker.perform_hook(Hook("ordering"),
                make_null_shared_ptr()));
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("", result.output());

    SafeIFStream f(FSPath("hooker_TEST_dir/ordering.out"));
    std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

#ifdef ENABLE_PYTHON_HOOKS
    EXPECT_EQ("e\nc\nf\nd\nb\na\npy_hook\ng\ni\nh\nsohook\nk\nj\n", line);
#else
    EXPECT_EQ("e\nc\nf\nd\nb\na\ng\ni\nh\nsohook\nk\nj\n", line);
#endif

}

TEST(Hooker, BadHooks)
{
    TestEnvironment env;
    Hooker hooker(&env);

    FSPath("hooker_TEST_dir/bad_hooks.out").unlink();

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);
    HookResult result(hooker.perform_hook(Hook("bad_hooks"),
                make_null_shared_ptr()));
    EXPECT_EQ(123, result.max_exit_status());
    EXPECT_EQ("", result.output());

    SafeIFStream f(FSPath("hooker_TEST_dir/bad_hooks.out"));
    std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    EXPECT_EQ("one\nthree\n", line);
}

TEST(Hooker, Cycles)
{
    TestEnvironment env;
    Hooker hooker(&env);

    FSPath("hooker_TEST_dir/cycles.out").unlink();

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);
    HookResult result(hooker.perform_hook(Hook("cycles"),
                make_null_shared_ptr()));
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("", result.output());

    SafeIFStream f(FSPath("hooker_TEST_dir/cycles.out"));
    std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    EXPECT_EQ("b\na\ng\nf\ni\n", line);
}

TEST(Hooker, Output)
{
    TestEnvironment env;
    Hooker hooker(&env);
    HookResult result(make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = ""));

    FSPath("hooker_TEST_dir/several_output.out").unlink();

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);

    result = hooker.perform_hook(Hook("simple_hook_output")
            .grab_output(Hook::AllowedOutputValues()("foo")),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("foo", result.output());

    result = hooker.perform_hook(Hook("fancy_hook_output")
            .grab_output(Hook::AllowedOutputValues()("foo")),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("foo", result.output());

    result = hooker.perform_hook(Hook("so_hook_output")
             .grab_output(Hook::AllowedOutputValues()("foo")),
             make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("foo", result.output());

#ifdef ENABLE_PYTHON_HOOKS
    result = hooker.perform_hook(Hook("py_hook_output")
             .grab_output(Hook::AllowedOutputValues()("foo")),
             make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("foo", result.output());
#endif

    result = hooker.perform_hook(Hook("several_hooks_output")
            .grab_output(Hook::AllowedOutputValues()),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("one", result.output());

    result = hooker.perform_hook(Hook("several_hooks_output")
            .grab_output(Hook::AllowedOutputValues()("one")),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("one", result.output());

    result = hooker.perform_hook(Hook("several_hooks_output")
            .grab_output(Hook::AllowedOutputValues()("two")("three")),
            make_null_shared_ptr());
    EXPECT_EQ(0, result.max_exit_status());
    EXPECT_EQ("two", result.output());

    result = hooker.perform_hook(Hook("several_hooks_output")
            .grab_output(Hook::AllowedOutputValues()("blah")),
            make_null_shared_ptr());
    EXPECT_EQ("", result.output());
    EXPECT_EQ(0, result.max_exit_status());

    SafeIFStream f(FSPath("hooker_TEST_dir/several_output.out"));
    std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    EXPECT_EQ("one\none\none\ntwo\none\ntwo\nthree\n", line);

}

TEST(Hooker, BadOutput)
{
    TestEnvironment env;
    Hooker hooker(&env);

    FSPath("hooker_TEST_dir/several_output_bad.out").unlink();

    hooker.add_dir(FSPath("hooker_TEST_dir/"), false);

    HookResult result(hooker.perform_hook(Hook("several_hooks_output_bad")
                .grab_output(Hook::AllowedOutputValues()),
                make_null_shared_ptr()));
    EXPECT_EQ("two", result.output());
    EXPECT_EQ(99, result.max_exit_status());

    SafeIFStream f(FSPath("hooker_TEST_dir/several_output_bad.out"));
    std::string line((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    EXPECT_EQ("one\ntwo\nthree\n", line);
}


