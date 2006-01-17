/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/args/args.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::args;
using namespace test;

/** \file
 * Test cases for paludis::args things.
 */

#ifndef DOXYGEN

struct CommandLine : public ArgsHandler
{
    ArgsGroup group_one;
    SwitchArg arg_foo;
    SwitchArg arg_bar;
    SwitchArg arg_dummy;

    ArgsGroup group_two;
    SwitchArg arg_baz;
    AliasArg arg_other_baz;
    StringArg arg_something;
    IntegerArg arg_somenum;
    EnumArg arg_enum;
    EnumArg arg_other_enum;

    CommandLine() :
        group_one(this, "Group one"),
        arg_foo(&group_one, "foo", 'f', "Enable foo"),
        arg_bar(&group_one, "bar", 'b', "Enable bar"),
        arg_dummy(&group_one, "dummy", 'd', "Enable something else"),

        group_two(this, "Group two"),
        arg_baz(&group_two, "baz", 'z', "Enable baz"),
        arg_other_baz(&arg_baz, "other-baz"),
        arg_something(&group_two, "something", 's', "Value of something"),
        arg_somenum(&group_two, "num", 'n', "Some number"),
        arg_enum(&group_two, "enum", 'e', "One of three", EnumArg::EnumArgOptions("one", "Option one")("two", "option two")("three", "option three"), "two"),
        arg_other_enum(&group_two, "something", '\0', "Blah.", EnumArg::EnumArgOptions("a", "A")("b", "B")("c", "C"), "b")
    {
    }
};

#endif

namespace test_cases
{
    /**
     * \test Simple args tests.
     */
    struct ArgsTestSimple : TestCase
    {
        ArgsTestSimple() : TestCase("simple") { }

        void run()
        {
            char * args[] = { "program-name", "--other-baz", "-fsne", "blah", "7", "three", "--", "--dummy",
                "one", "two" };
            CommandLine c1;
            c1.run(10, args);
            TEST_CHECK(c1.arg_foo.specified());
            TEST_CHECK(! c1.arg_bar.specified());
            TEST_CHECK(c1.arg_baz.specified());
            TEST_CHECK(c1.arg_other_baz.specified());
            TEST_CHECK(c1.arg_something.specified());
            TEST_CHECK(c1.arg_something.argument() == "blah");
            TEST_CHECK(c1.arg_somenum.specified());
            TEST_CHECK(c1.arg_somenum.argument() == 7);
            TEST_CHECK(c1.arg_enum.specified());
            TEST_CHECK(c1.arg_enum.argument() == "three");
            TEST_CHECK(! c1.arg_dummy.specified());
            TEST_CHECK(! c1.arg_other_enum.specified());
            TEST_CHECK(c1.arg_other_enum.argument() == "b");

            TEST_CHECK_EQUAL(std::distance(c1.begin_parameters(), c1.end_parameters()), 3);
            TEST_CHECK_EQUAL(*c1.begin_parameters(), "--dummy");
            TEST_CHECK_EQUAL(*++c1.begin_parameters(), "one");
            TEST_CHECK_EQUAL(*++(++(c1.begin_parameters())), "two");
        }
    } test_args_simple;
}

