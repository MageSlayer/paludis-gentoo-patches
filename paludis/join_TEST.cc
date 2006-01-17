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

#include "join.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <vector>
#include <list>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for join.hh .
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test join on a vector.
     *
     * \ingroup Test
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
     * \ingroup Test
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
     * \test Test join with empty things.
     *
     * \ingroup Test
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
}
