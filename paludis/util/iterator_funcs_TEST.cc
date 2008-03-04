/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#include <paludis/util/iterator_funcs.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <vector>
#include <list>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct IteratorNextTest : public TestCase
    {
        IteratorNextTest() : TestCase("iterator next()") { }

        void run()
        {
            std::vector<int> v;
            v.push_back(1);
            v.push_back(2);
            v.push_back(3);
            std::vector<int>::iterator iter(v.begin());

            TEST_CHECK(*(next(iter)) == 2);
            TEST_CHECK(next(next(next(iter))) == v.end());
            TEST_CHECK(next(iter, 3) == v.end());
            iter = next(iter);
            TEST_CHECK(*(next(iter, 1)) == 3);
            iter = next(iter);
            TEST_CHECK(++iter == v.end());
        }
    } test_iterator_next;

    /**
     * \test Test iterator_utilities previous()
     *
     */
    struct IteratorpreviousTest : public TestCase
    {
        IteratorpreviousTest() : TestCase("iterator previous()") { }

        void run()
        {
            std::vector<int> v;
            v.push_back(1);
            v.push_back(2);
            std::vector<int>::iterator iter(v.end());

            TEST_CHECK(*(previous(iter)) == 2);
            TEST_CHECK(previous(previous(iter)) == v.begin());
            iter = previous(iter);
            TEST_CHECK(--iter == v.begin());
        }
    } test_iterator_previous;

    struct CappedDistanceTest : TestCase
    {
        CappedDistanceTest() : TestCase("capped distance") { }

        void run()
        {
            std::list<int> v;
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(0));

            v.push_back(1);
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(1));

            v.push_back(2);
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(2));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(2));

            v.push_back(3);
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(3));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(3));

            v.push_back(4);
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(3));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(4));

            v.push_back(5);
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 0), static_cast<std::size_t>(0));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 1), static_cast<std::size_t>(1));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 3), static_cast<std::size_t>(3));
            TEST_CHECK_EQUAL(capped_distance(v.begin(), v.end(), 10), static_cast<std::size_t>(5));
        }
    } test_capped_distance;
}

