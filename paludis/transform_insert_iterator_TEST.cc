/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "transform_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

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
     * \ingroup Test
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
}

