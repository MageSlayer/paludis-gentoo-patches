/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include <iterator>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct TestTokeniserAD : TestCase
    {
        TestTokeniserAD() : TestCase("Tokeniser<AnyOfTag, DelimiterTag(default)>") { }

        void run()
        {
            const std::string delims(",.+");

            std::vector<std::string> tokens;

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("one,two...+...three...", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(3));
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), "two");
            TEST_CHECK_EQUAL(tokens.at(2), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("...one,two...+...three", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(3));
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), "two");
            TEST_CHECK_EQUAL(tokens.at(2), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("one", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(1));
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(".+.,.", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(0));
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>("", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(0));
            tokens.clear();
        }
    } test_tokeniser_ad;

    struct TestTokeniserAB : TestCase
    {
        TestTokeniserAB() : TestCase("Tokeniser<AnyOfTag, BoundaryTag>") { }

        void run()
        {
            const std::string delims(",.+");

            std::vector<std::string> tokens;

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("one,two...+...three...", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(6));
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), ",");
            TEST_CHECK_EQUAL(tokens.at(2), "two");
            TEST_CHECK_EQUAL(tokens.at(3), "...+...");
            TEST_CHECK_EQUAL(tokens.at(4), "three");
            TEST_CHECK_EQUAL(tokens.at(5), "...");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("...one,two...+...three", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(6));
            TEST_CHECK_EQUAL(tokens.at(0), "...");
            TEST_CHECK_EQUAL(tokens.at(1), "one");
            TEST_CHECK_EQUAL(tokens.at(2), ",");
            TEST_CHECK_EQUAL(tokens.at(3), "two");
            TEST_CHECK_EQUAL(tokens.at(4), "...+...");
            TEST_CHECK_EQUAL(tokens.at(5), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("one", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(1));
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>(".+.,.", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(1));
            TEST_CHECK_EQUAL(tokens.at(0), ".+.,.");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            tokenise<delim_kind::AnyOfTag, delim_mode::BoundaryTag>("", delims, "", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), std::size_t(0));
            tokens.clear();
        }
    } test_tokeniser_ab;

    struct QuotedWhitespaceTokeniserTest : TestCase
    {
        QuotedWhitespaceTokeniserTest() : TestCase("quoted whitespace tokeniser") { }

        void run()
        {
            std::vector<std::string> v1;
            tokenise_whitespace_quoted("one \"two three\" four 'five six' seven '' eight", std::back_inserter(v1));
            TestMessageSuffix s(join(v1.begin(), v1.end(), "#"));
            TEST_CHECK_EQUAL(v1.size(), 7u);
            TEST_CHECK_EQUAL(v1.at(0), "one");
            TEST_CHECK_EQUAL(v1.at(1), "two three");
            TEST_CHECK_EQUAL(v1.at(2), "four");
            TEST_CHECK_EQUAL(v1.at(3), "five six");
            TEST_CHECK_EQUAL(v1.at(4), "seven");
            TEST_CHECK_EQUAL(v1.at(5), "");
            TEST_CHECK_EQUAL(v1.at(6), "eight");

            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one \"two three", std::back_inserter(v1)), TokeniserError);
            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one tw\"o\" three", std::back_inserter(v1)), TokeniserError);
            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one tw\"o", std::back_inserter(v1)), TokeniserError);
            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one tw\"o\"three", std::back_inserter(v1)), TokeniserError);
            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one \"two\"three\"", std::back_inserter(v1)), TokeniserError);
            TEST_CHECK_THROWS(tokenise_whitespace_quoted("one \"two\"\"three\"", std::back_inserter(v1)), TokeniserError);
        }
    } test_quoted_whitespace_tokeniser;
}

