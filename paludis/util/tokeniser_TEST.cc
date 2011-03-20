/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2011 Ciaran McCreesh
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

#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>

#include <iterator>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Tokeniser, AnyOfDefaultDelim)
{
    const std::string delims(",.+");

    std::vector<std::string> tokens;

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("one,two...+...three...", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(3), tokens.size());
    EXPECT_EQ("one", tokens.at(0));
    EXPECT_EQ("two", tokens.at(1));
    EXPECT_EQ("three", tokens.at(2));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("...one,two...+...three", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(3), tokens.size());
    EXPECT_EQ("one", tokens.at(0));
    EXPECT_EQ("two", tokens.at(1));
    EXPECT_EQ("three", tokens.at(2));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("one", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(1), tokens.size());
    EXPECT_EQ("one", tokens.at(0));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(".+.,.", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(0), tokens.size());
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(0), tokens.size());
    tokens.clear();
}

TEST(Tokeniser, AnyOfBoundary)
{
    const std::string delims(",.+");

    std::vector<std::string> tokens;

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("one,two...+...three...", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(6), tokens.size());
    EXPECT_EQ("one", tokens.at(0));
    EXPECT_EQ(",", tokens.at(1));
    EXPECT_EQ("two", tokens.at(2));
    EXPECT_EQ("...+...", tokens.at(3));
    EXPECT_EQ("three", tokens.at(4));
    EXPECT_EQ("...", tokens.at(5));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("...one,two...+...three", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(6), tokens.size());
    EXPECT_EQ("...", tokens.at(0));
    EXPECT_EQ("one", tokens.at(1));
    EXPECT_EQ(",", tokens.at(2));
    EXPECT_EQ("two", tokens.at(3));
    EXPECT_EQ("...+...", tokens.at(4));
    EXPECT_EQ("three", tokens.at(5));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("one", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(1), tokens.size());
    EXPECT_EQ("one", tokens.at(0));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>(".+.,.", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(1), tokens.size());
    EXPECT_EQ(".+.,.", tokens.at(0));
    tokens.clear();

    ASSERT_TRUE(tokens.empty());
    tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("", delims, "", std::back_inserter(tokens));
    ASSERT_EQ(std::size_t(0), tokens.size());
    tokens.clear();
}

TEST(QuotedWhitespaceTokeniser, Works)
{
    std::vector<std::string> v1;
    tokenise_whitespace_quoted("one \"two three\" four 'five six' seven '' eight", std::back_inserter(v1));
    ASSERT_EQ(7u, v1.size());
    EXPECT_EQ("one", v1.at(0));
    EXPECT_EQ("two three", v1.at(1));
    EXPECT_EQ("four", v1.at(2));
    EXPECT_EQ("five six", v1.at(3));
    EXPECT_EQ("seven", v1.at(4));
    EXPECT_EQ("", v1.at(5));
    EXPECT_EQ("eight", v1.at(6));
}

TEST(QuotedWhitespaceTokeniser, Throws)
{
    std::vector<std::string> v1;
    EXPECT_THROW(tokenise_whitespace_quoted("one \"two three", std::back_inserter(v1)), TokeniserError);
    EXPECT_THROW(tokenise_whitespace_quoted("one tw\"o\" three", std::back_inserter(v1)), TokeniserError);
    EXPECT_THROW(tokenise_whitespace_quoted("one tw\"o", std::back_inserter(v1)), TokeniserError);
    EXPECT_THROW(tokenise_whitespace_quoted("one tw\"o\"three", std::back_inserter(v1)), TokeniserError);
    EXPECT_THROW(tokenise_whitespace_quoted("one \"two\"three\"", std::back_inserter(v1)), TokeniserError);
    EXPECT_THROW(tokenise_whitespace_quoted("one \"two\"\"three\"", std::back_inserter(v1)), TokeniserError);
}

