/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2011 Ciaran McCreesh
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

#include <paludis/util/stringify.hh>

#include <string>

#include <gtest/gtest.h>

using namespace paludis;

TEST(StringifyInt, Works)
{
    ASSERT_EQ("0", stringify(0));
    ASSERT_EQ("1", stringify(1));
    ASSERT_EQ("99", stringify(99));
    ASSERT_EQ("-99", stringify(-99));
    ASSERT_EQ("12345", stringify(12345));
}

TEST(StringifyCharStar, Works)
{
    ASSERT_EQ(std::string("moo"), stringify("moo"));
    ASSERT_EQ(std::string(""), stringify(""));
    ASSERT_TRUE(stringify("").empty());
    ASSERT_EQ(std::string("  quack quack  "), stringify("  quack quack  "));
}

TEST(StringifyString, Works)
{
    ASSERT_EQ(std::string("moo"), stringify(std::string("moo")));
    ASSERT_EQ(std::string(""), stringify(std::string("")));
    ASSERT_TRUE(stringify(std::string("")).empty());
    ASSERT_EQ(std::string("  quack quack  "), stringify(std::string("  quack quack  ")));
}

TEST(StringifyChar, Works)
{
    char c('a');
    ASSERT_EQ(std::string("a"), stringify(c));

    unsigned char u('a');
    ASSERT_EQ(std::string("a"), stringify(u));

    signed char s('a');
    ASSERT_EQ(std::string("a"), stringify(s));
}

TEST(StringifyBool, Works)
{
    ASSERT_EQ(std::string("true"), stringify(true));
    ASSERT_EQ(std::string("false"), stringify(false));
}

