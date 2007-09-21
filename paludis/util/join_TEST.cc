/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <list>
#include <paludis/util/join.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for join.hh .
 *
 */

namespace
{
    std::string make_purdy(const int x)
    {
        return "<" + stringify(x) + ">";
    }
}

namespace test_cases
{
    /**
     * \test Test join on a vector.
     *
     */
    struct JoinVectorTest : TestCase
    {
        JoinVectorTest() : TestCase("join vector") { }

        void run()
        {
            std::vector<std::string> v;
            v.push_back("one");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one");
            v.push_back("two");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one/two");
            v.push_back("three");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one/two/three");
        }
    } test_join_vector;

    /**
     * \test Test join on a list.
     *
     */
    struct JoinListTest : TestCase
    {
        JoinListTest() : TestCase("join list") { }

        void run()
        {
            std::list<std::string> v;
            v.push_back("one");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one");
            v.push_back("two");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one/two");
            v.push_back("three");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "one/two/three");
        }
    } test_join_list;

    /**
     * \test Test join on a list of ints.
     *
     */
    struct JoinListIntTest : TestCase
    {
        JoinListIntTest() : TestCase("join list int") { }

        void run()
        {
            std::list<int> v;
            v.push_back(1);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "1");
            v.push_back(2);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "1/2");
            v.push_back(3);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/"), "1/2/3");
        }
    } test_join_list_int;

    /**
     * \test Test join with empty things.
     *
     */
    struct JoinEmptyTest : TestCase
    {
        JoinEmptyTest() : TestCase("join empty") { }

        void run()
        {
            std::list<std::string> v;
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), ""), "");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "*"), "");
            v.push_back("");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), ""), "");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "*"), "");
            v.push_back("");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), ""), "");
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "*"), "*");
        }
    } test_join_empty;

    struct JoinFunctionTest : TestCase
    {
        JoinFunctionTest() : TestCase("join function") { }

        void run()
        {
            std::list<int> v;
            v.push_back(1);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/", &make_purdy), "<1>");
            v.push_back(2);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/", &make_purdy), "<1>/<2>");
            v.push_back(3);
            TEST_CHECK_EQUAL(join(v.begin(), v.end(), "/", &make_purdy), "<1>/<2>/<3>");
        }
    } test_join_function;
}

