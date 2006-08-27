/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for tokeniser.hh .
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test Tokeniser<AnyOfTag, DelimiterTag>
     *
     * \ingroup grptestcases
     */
    struct TestTokeniserAD : TestCase
    {
        TestTokeniserAD() : TestCase("Tokeniser<AnyOfTag, DelimiterTag>") { }

        void run()
        {
            Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> t(",.+");
            std::vector<std::string> tokens;

            TEST_CHECK(tokens.empty());
            t.tokenise("one,two...+...three...", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 3);
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), "two");
            TEST_CHECK_EQUAL(tokens.at(2), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("...one,two...+...three", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 3);
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), "two");
            TEST_CHECK_EQUAL(tokens.at(2), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("one", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 1);
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise(".+.,.", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 0);
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 0);
            tokens.clear();
        }
    } test_tokeniser_ad;

    /**
     * \test Test Tokeniser<AnyOfTag, BoundaryTag>
     *
     * \ingroup grptestcases
     */
    struct TestTokeniserAB : TestCase
    {
        TestTokeniserAB() : TestCase("Tokeniser<AnyOfTag, BoundaryTag>") { }

        void run()
        {
            Tokeniser<delim_kind::AnyOfTag, delim_mode::BoundaryTag> t(",.+");
            std::vector<std::string> tokens;

            TEST_CHECK(tokens.empty());
            t.tokenise("one,two...+...three...", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 6);
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            TEST_CHECK_EQUAL(tokens.at(1), ",");
            TEST_CHECK_EQUAL(tokens.at(2), "two");
            TEST_CHECK_EQUAL(tokens.at(3), "...+...");
            TEST_CHECK_EQUAL(tokens.at(4), "three");
            TEST_CHECK_EQUAL(tokens.at(5), "...");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("...one,two...+...three", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 6);
            TEST_CHECK_EQUAL(tokens.at(0), "...");
            TEST_CHECK_EQUAL(tokens.at(1), "one");
            TEST_CHECK_EQUAL(tokens.at(2), ",");
            TEST_CHECK_EQUAL(tokens.at(3), "two");
            TEST_CHECK_EQUAL(tokens.at(4), "...+...");
            TEST_CHECK_EQUAL(tokens.at(5), "three");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("one", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 1);
            TEST_CHECK_EQUAL(tokens.at(0), "one");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise(".+.,.", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 1);
            TEST_CHECK_EQUAL(tokens.at(0), ".+.,.");
            tokens.clear();

            TEST_CHECK(tokens.empty());
            t.tokenise("", std::back_inserter(tokens));
            TEST_CHECK_EQUAL(tokens.size(), 0);
            tokens.clear();
        }
    } test_tokeniser_ab;
}
