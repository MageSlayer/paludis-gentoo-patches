/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2011 Ciaran McCreesh
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

#include <paludis/util/simple_parser.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(SimpleParser, Works)
{
    std::string text("oneTWOthree");
    std::string one;
    std::string two;
    std::string three;
    SimpleParser parser(text);
    ASSERT_TRUE(parser.consume(simple_parser::exact("one") >> one));
    ASSERT_TRUE(parser.consume(simple_parser::exact_ignoring_case("two") >> two));
    ASSERT_TRUE(! parser.consume(simple_parser::exact("THREE") >> three));
    ASSERT_TRUE(parser.consume(simple_parser::exact_ignoring_case("THREE") >> three));

    ASSERT_TRUE(parser.eof());
    EXPECT_EQ("one", one);
    EXPECT_EQ("TWO", two);
    EXPECT_EQ("three", three);
}

