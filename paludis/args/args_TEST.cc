/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2009, 2010 Ciaran McCreesh
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
#include <paludis/args/args_error.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::args;
using namespace test;

/** \file
 * Test cases for paludis::args things.
 *
 */

#ifndef DOXYGEN

struct CommandLine : public ArgsHandler
{
    ArgsGroup group_one;
    SwitchArg arg_foo;
    SwitchArg arg_bar;
    SwitchArg arg_dummy;
    SwitchArg arg_removed;

    ArgsGroup group_two;
    SwitchArg arg_baz;
    AliasArg arg_other_baz;
    StringArg arg_something;
    StringArg arg_monkey;
    AliasArg arg_other_monkey;
    IntegerArg arg_somenum;
    EnumArg arg_enum;
    SwitchArg arg_spider;

    ArgsGroup group_three;
    EnumArg arg_other_enum;
    StringSetArg arg_stringset;

    CommandLine();
    ~CommandLine();

    std::string app_name() const
    {
        return "args_TEST";
    }

    std::string app_synopsis() const
    {
        return "tests for args";
    }

    std::string app_description() const
    {
        return "Tests args";
    }
};

CommandLine::CommandLine() :
    group_one(main_options_section(), "Group one", "Description of group one"),
    arg_foo(&group_one, "foo", 'f', "Enable foo", false),
    arg_bar(&group_one, "bar", 'b', "Enable bar", false),
    arg_dummy(&group_one, "dummy", 'd', "Enable something else", false),
    arg_removed(&group_one, "removed", 'r', "Removed", false),

    group_two(main_options_section(), "Group two", "Description of group two"),
    arg_baz(&group_two, "baz", 'z', "Enable baz", false),
    arg_other_baz(&arg_baz, "other-baz"),
    arg_something(&group_two, "something", 's', "Value of something"),
    arg_monkey(&group_two, "monkey", 'm', "A monkey?"),
    arg_other_monkey(&arg_monkey, "other-monkey"),
    arg_somenum(&group_two, "num", 'n', "Some number"),
    arg_enum(&group_two, "enum", 'e', "One of three",
            EnumArg::EnumArgOptions("one", "Option one")("two", "option two")("three", "option three"), "two"),
    arg_spider(&group_two, "spider", '\0', "A spider?", true),

    group_three(main_options_section(), "Group three", "Description of group three"),
    arg_other_enum(&group_three, "something-else", '\0', "Blah.", EnumArg::EnumArgOptions("a", "A")("b", "B")("c", "C"), "b"),
    arg_stringset(&group_three, "stringset", 't', "A StringSet.")
{
}

CommandLine::~CommandLine()
{
}

#endif

namespace test_cases
{
    /**
     * \test Simple args tests.
     *
     */
    struct ArgsTestSimple : TestCase
    {
        ArgsTestSimple() : TestCase("simple") { }

        void run()
        {
            const char * args[] = { "program-name", "--other-monkey", "chimp", "--other-baz", "--spider", "--no-spider",
                "-fsne", "blah", "7", "three", "--", "--dummy", "one", "two" };
            CommandLine c1;
            c1.run(14, args, "", "", "");
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
            TEST_CHECK(c1.arg_monkey.specified());
            TEST_CHECK(c1.arg_monkey.argument() == "chimp");
            TEST_CHECK(! c1.arg_spider.specified());

            TEST_CHECK_EQUAL(std::distance(c1.begin_parameters(), c1.end_parameters()), 3);
            TEST_CHECK_EQUAL(*c1.begin_parameters(), "--dummy");
            TEST_CHECK_EQUAL(*++c1.begin_parameters(), "one");
            TEST_CHECK_EQUAL(*++(++(c1.begin_parameters())), "two");
        }
    } test_args_simple;

    /**
     * \test Missing parameters tests.
     *
     */
    struct ArgsTestNoParam : TestCase
    {
        ArgsTestNoParam() : TestCase("Missing parameters") { }

        void run()
        {
            const char *args[] = { "program-name", "-e" };
            CommandLine c1;
            TEST_CHECK_THROWS(c1.run(2, args, "", "", ""), MissingValue);
        }
    } test_args_no_param;

    struct ArgsTestNoNo : TestCase
    {
        ArgsTestNoNo() : TestCase("No --no-") { }

        void run()
        {
            const char *args[] = { "program-name", "--no-num" };
            CommandLine c1;
            TEST_CHECK_THROWS(c1.run(2, args, "", "", ""), BadArgument);
        }
    } test_args_no_no;

    /**
     * \test Removed arguments tests.
     *
     */
    struct ArgsTestRemovedArg : TestCase
    {
        ArgsTestRemovedArg() : TestCase("Removed arguments") { }

        void run()
        {
            const char *args1[] = { "program-name", "--removed" };
            const char *args2[] = { "program-name", "-r" };
            CommandLine c1;
            c1.run(2, args1, "", "", "");
            TEST_CHECK(true);
            c1.run(2, args2, "", "", "");
            TEST_CHECK(true);
            c1.arg_removed.remove();
            TEST_CHECK_THROWS(c1.run(2, args1, "", "", ""), BadArgument);
            TEST_CHECK_THROWS(c1.run(2, args2, "", "", ""), BadArgument);
        }
    } test_args_removed_arg;

    /**
     * \test Default arguments tests.
     *
     */
    struct ArgsTestDefaultArg : TestCase
    {
        ArgsTestDefaultArg() : TestCase("Default arguments") { }

        void run()
        {
            const char *args1[] = { "program-name", "--enum", "three" };
            const char *args2[] = { "program-name" };
            CommandLine c1, c2, c3, c4;
            c2.arg_enum.set_default_arg("one");
            c4.arg_enum.set_default_arg("one");

            c1.run(3, args1, "", "", "");
            c2.run(3, args1, "", "", "");
            c1.run(1, args2, "", "", "");
            c2.run(1, args2, "", "", "");

            TEST_CHECK(c1.arg_enum.specified());
            TEST_CHECK(c2.arg_enum.specified());
            TEST_CHECK(! c3.arg_enum.specified());
            TEST_CHECK(! c4.arg_enum.specified());

            TEST_CHECK_EQUAL(c1.arg_enum.argument(), "three");
            TEST_CHECK_EQUAL(c2.arg_enum.argument(), "three");
            TEST_CHECK_EQUAL(c3.arg_enum.argument(), "two");
            TEST_CHECK_EQUAL(c4.arg_enum.argument(), "one");
        }
    } test_args_default_arg;

    /**
     * \test String tests.
     *
     */
    struct ArgsTestStringSet : TestCase
    {
        ArgsTestStringSet() : TestCase("StringSet") { }

        void run()
        {
            const char *args[] = { "program-name", "--stringset", "one", "-t", "two", "-t", "three", "fnord" };
            CommandLine c1;
            c1.run(8, args, "", "", "");
            TEST_CHECK(c1.arg_stringset.specified());
            TEST_CHECK(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "one") != c1.arg_stringset.end_args());
            TEST_CHECK(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "two") != c1.arg_stringset.end_args());
            TEST_CHECK(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "three") != c1.arg_stringset.end_args());
            TEST_CHECK(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "fnord") == c1.arg_stringset.end_args());
        }
    } test_args_string_set;
}

