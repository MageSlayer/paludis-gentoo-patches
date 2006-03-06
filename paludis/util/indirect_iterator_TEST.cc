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

#include <paludis/util/indirect_iterator.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/deleter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <list>
#include <algorithm>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for IndirectIterator.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test IndirectIterator over a vector of CountedPtr of int.
     *
     * \ingroup Test
     */
    struct IndirectIteratorVecCPIntTest : TestCase
    {
        IndirectIteratorVecCPIntTest() : TestCase("vector<CountedPtr<int> >") { }

        void run()
        {
            std::vector<CountedPtr<int, count_policy::ExternalCountTag> > v;
            v.push_back(CountedPtr<int, count_policy::ExternalCountTag>(new int(5)));
            v.push_back(CountedPtr<int, count_policy::ExternalCountTag>(new int(10)));
            IndirectIterator<std::vector<CountedPtr<int,
                count_policy::ExternalCountTag> >::iterator, int> vi(v.begin()), vi_end(v.end());
            TEST_CHECK(vi != vi_end);
            TEST_CHECK(vi < vi_end);
            TEST_CHECK(! (vi > vi_end));
            TEST_CHECK_EQUAL(*vi, 5);
            TEST_CHECK(++vi != vi_end);
            TEST_CHECK(vi < vi_end);
            TEST_CHECK(! (vi > vi_end));
            TEST_CHECK_EQUAL(*vi, 10);
            TEST_CHECK(++vi == vi_end);
        }
    } test_indirect_iterator_vec_cp_int;

    /**
     * \test Test IndirectIterator over a list of CountedPtr of int.
     *
     * \ingroup Test
     */
    struct IndirectIteratorListCPIntTest : TestCase
    {
        IndirectIteratorListCPIntTest() : TestCase("list<CountedPtr<int> >") { }

        void run()
        {
            std::list<CountedPtr<int, count_policy::ExternalCountTag> > v;
            v.push_back(CountedPtr<int, count_policy::ExternalCountTag>(new int(5)));
            v.push_back(CountedPtr<int, count_policy::ExternalCountTag>(new int(10)));
            IndirectIterator<std::list<CountedPtr<int,
                count_policy::ExternalCountTag> >::iterator, int> vi(v.begin()), vi_end(v.end());
            TEST_CHECK(vi != vi_end);
            TEST_CHECK_EQUAL(*vi, 5);
            TEST_CHECK(++vi != vi_end);
            TEST_CHECK_EQUAL(*vi, 10);
            TEST_CHECK(++vi == vi_end);
        }
    } test_indirect_iterator_list_cp_int;

    /**
     * \test Test IndirectIterator over a vector of int *.
     *
     * \ingroup Test
     */
    struct IndirectIteratorVecPIntTest : TestCase
    {
        IndirectIteratorVecPIntTest() : TestCase("vector<int *>") { }

        void run()
        {
            std::vector<int *> v;
            v.push_back(new int(5));
            v.push_back(new int(10));
            IndirectIterator<std::vector<int *>::iterator, int> vi(v.begin()), vi_end(v.end());
            TEST_CHECK(vi != vi_end);
            TEST_CHECK(vi < vi_end);
            TEST_CHECK(! (vi > vi_end));
            TEST_CHECK_EQUAL(*vi, 5);
            TEST_CHECK(++vi != vi_end);
            TEST_CHECK(vi < vi_end);
            TEST_CHECK(! (vi > vi_end));
            TEST_CHECK_EQUAL(*vi, 10);
            TEST_CHECK(++vi == vi_end);

            std::for_each(v.begin(), v.end(), Deleter());
        }
    } test_indirect_iterator_vec_p_int;

    /**
     * \test Test IndirectIterator over a list of int *.
     *
     * \ingroup Test
     */
    struct IndirectIteratorListPIntTest : TestCase
    {
        IndirectIteratorListPIntTest() : TestCase("list<CountedPtr<int *>") { }

        void run()
        {
            std::list<int *> v;
            v.push_back(new int(5));
            v.push_back(new int(10));
            IndirectIterator<std::list<int *>::iterator, int> vi(v.begin()), vi_end(v.end());
            TEST_CHECK(vi != vi_end);
            TEST_CHECK_EQUAL(*vi, 5);
            TEST_CHECK(++vi != vi_end);
            TEST_CHECK_EQUAL(*vi, 10);
            TEST_CHECK(++vi == vi_end);

            std::for_each(v.begin(), v.end(), Deleter());
        }
    } test_indirect_iterator_list_p_int;
}

