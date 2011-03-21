/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2009, 2010, 2011 Ciaran McCreesh
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

#include <algorithm>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::args;

namespace
{
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
}

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

TEST(Args, Simple)
{
    const char * args[] = { "program-name", "--other-monkey", "chimp", "--other-baz", "--spider", "--no-spider",
        "-fsne", "blah", "7", "three", "--", "--dummy", "one", "two" };
    CommandLine c1;
    c1.run(14, args, "", "", "");
    EXPECT_TRUE(c1.arg_foo.specified());
    EXPECT_TRUE(! c1.arg_bar.specified());
    EXPECT_TRUE(c1.arg_baz.specified());
    EXPECT_TRUE(c1.arg_other_baz.specified());
    EXPECT_TRUE(c1.arg_something.specified());
    EXPECT_TRUE(c1.arg_something.argument() == "blah");
    EXPECT_TRUE(c1.arg_somenum.specified());
    EXPECT_TRUE(c1.arg_somenum.argument() == 7);
    EXPECT_TRUE(c1.arg_enum.specified());
    EXPECT_TRUE(c1.arg_enum.argument() == "three");
    EXPECT_TRUE(! c1.arg_dummy.specified());
    EXPECT_TRUE(! c1.arg_other_enum.specified());
    EXPECT_TRUE(c1.arg_other_enum.argument() == "b");
    EXPECT_TRUE(c1.arg_monkey.specified());
    EXPECT_TRUE(c1.arg_monkey.argument() == "chimp");
    EXPECT_TRUE(! c1.arg_spider.specified());

    ASSERT_EQ(3, std::distance(c1.begin_parameters(), c1.end_parameters()));
    EXPECT_EQ("--dummy", *c1.begin_parameters());
    EXPECT_EQ("one", *++c1.begin_parameters());
    EXPECT_EQ("two", *++(++(c1.begin_parameters())));
}

TEST(Args, MissingParameters)
{
    const char *args[] = { "program-name", "-e" };
    CommandLine c1;
    EXPECT_THROW(c1.run(2, args, "", "", ""), MissingValue);
}

TEST(Args, NoNo)
{
    const char *args[] = { "program-name", "--no-num" };
    CommandLine c1;
    EXPECT_THROW(c1.run(2, args, "", "", ""), BadArgument);
}

TEST(Args, Removed)
{
    const char *args1[] = { "program-name", "--removed" };
    const char *args2[] = { "program-name", "-r" };
    CommandLine c1;
    c1.run(2, args1, "", "", "");
    EXPECT_TRUE(true);
    c1.run(2, args2, "", "", "");
    EXPECT_TRUE(true);
    c1.arg_removed.remove();
    EXPECT_THROW(c1.run(2, args1, "", "", ""), BadArgument);
    EXPECT_THROW(c1.run(2, args2, "", "", ""), BadArgument);
}

TEST(Args, Defaults)
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

    EXPECT_TRUE(c1.arg_enum.specified());
    EXPECT_TRUE(c2.arg_enum.specified());
    EXPECT_TRUE(! c3.arg_enum.specified());
    EXPECT_TRUE(! c4.arg_enum.specified());

    EXPECT_EQ("three", c1.arg_enum.argument());
    EXPECT_EQ("three", c2.arg_enum.argument());
    EXPECT_EQ("two", c3.arg_enum.argument());
    EXPECT_EQ("one", c4.arg_enum.argument());
}

TEST(Args, StringSet)
{
    const char *args[] = { "program-name", "--stringset", "one", "-t", "two", "-t", "three", "fnord" };
    CommandLine c1;
    c1.run(8, args, "", "", "");
    EXPECT_TRUE(c1.arg_stringset.specified());
    EXPECT_TRUE(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "one") != c1.arg_stringset.end_args());
    EXPECT_TRUE(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "two") != c1.arg_stringset.end_args());
    EXPECT_TRUE(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "three") != c1.arg_stringset.end_args());
    EXPECT_TRUE(std::find(c1.arg_stringset.begin_args(), c1.arg_stringset.end_args(), "fnord") == c1.arg_stringset.end_args());
}

