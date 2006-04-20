/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <algorithm>
#include <list>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/deleter.hh>
#include <paludis/util/iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <set>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for iterator utilities.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test IndirectIterator over a vector of CountedPtr of int.
     *
     * \ingroup grptestcases
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
     * \ingroup grptestcases
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
     * \ingroup grptestcases
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
     * \ingroup grptestcases
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

#ifndef DOXYGEN
struct Counter
{
    int n;

    Counter() :
        n(0)
    {
    }

    int operator() ()
    {
        return n++;
    }
};

int is_even(const int & v)
{
    return ! (v & 1);
}
#endif


namespace test_cases
{
    /**
     * \test Test FilterInsertIterator.
     *
     * \ingroup grptestcases
     */
    struct FilterInsertIteratorTest : TestCase
    {
        FilterInsertIteratorTest() : TestCase("filter insert iterator") { }

        void run()
        {
            std::set<int> v;
            std::generate_n(filter_inserter(std::inserter(v, v.begin()), std::ptr_fun(&is_even)),
                    5, Counter());
            TEST_CHECK_EQUAL(v.size(), 3);
            for (int n = 0 ; n < 5 ; ++n)
            {
                TestMessageSuffix s("n=" + stringify(n));
                if (is_even(n))
                {
                    TEST_CHECK(v.end() != v.find(n));
                    TEST_CHECK_EQUAL(*v.find(n), n);
                }
                else
                    TEST_CHECK(v.end() == v.find(n));
            }
        }
    } test_filter_insert_iterator;

    /**
     * \test Test iterator_utilities next()
     *
     * \ingroup grptestcases
     */
    struct IteratorNextTest : public TestCase
    {
        IteratorNextTest() : TestCase("iterator next()") { }

        void run()
        {
            std::vector<int> v;
            v.push_back(1);
            v.push_back(2);
            std::vector<int>::iterator iter(v.begin());

            TEST_CHECK(*(next(iter)) == 2);
            TEST_CHECK(next(next(iter)) == v.end());
            iter = next(iter);
            TEST_CHECK(++iter == v.end());
        }
    } test_iterator_next;

    /**
     * \test Test iterator_utilities previous()
     *
     * \ingroup grptestcases
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
}

#ifndef DOXYGEN
int f(const int & v)
{
    return -v;
}
#endif

namespace test_cases
{
    /**
     * \test Test TransformInsertIterator.
     *
     * \ingroup grptestcases
     */
    struct TransformInsertIteratorTest : TestCase
    {
        TransformInsertIteratorTest() : TestCase("transform insert iterator") { }

        void run()
        {
            std::vector<int> v;
            std::generate_n(transform_inserter(std::back_inserter(v), std::ptr_fun(&f)),
                    5, Counter());
            TEST_CHECK_EQUAL(v.size(), 5);
            for (int n = 0 ; n < 5 ; ++n)
            {
                TestMessageSuffix s("n=" + stringify(n));
                TEST_CHECK_EQUAL(v.at(n), -n);
            }
        }
    } test_transform_insert_iterator;

    /**
     * \test Test SelectFirst and SelectSecond.
     *
     * \ingroup grptestcases
     */
    struct SimpleSelectPairTest : TestCase
    {
        SimpleSelectPairTest() : TestCase("Simple SelectFirst and SelectSecond") {}

        void run()
        {
            std::pair<int,int> p(1,2);
            SelectFirst<int,int> f;
            SelectSecond<int,int> s;

            TEST_CHECK(f(p) == 1);
            TEST_CHECK(s(p) == 2);
        }
    } test_select_pair;
}

#ifndef DOXYGEN
struct C
{
    std::string s;

    explicit C(const std::string & ss) :
        s(ss)
    {
    }
};
#endif

namespace test_cases
{
    /**
     * \test Test create_inserter.
     *
     * \ingroup grptestcases
     */
    struct CreateInsertIteratorTest : TestCase
    {
        CreateInsertIteratorTest() : TestCase("create insert iterator") { }

        void run()
        {
            std::vector<std::string> v;
            v.push_back("one");
            v.push_back("two");

            std::vector<C> vv;
            std::copy(v.begin(), v.end(), create_inserter<C>(std::back_inserter(vv)));

            TEST_CHECK_EQUAL(vv.size(), 2);
            TEST_CHECK_EQUAL(vv.at(0).s, "one");
            TEST_CHECK_EQUAL(vv.at(1).s, "two");
        }
    } test_create_insert_iterator;
}
