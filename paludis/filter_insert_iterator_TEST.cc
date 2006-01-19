/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "filter_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <set>
#include <algorithm>

using namespace test;
using namespace paludis;

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
}

