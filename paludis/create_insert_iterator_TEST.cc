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

#include "create_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for CreateInsertIterator.
 *
 * \ingroup Test
 * \ingroup Iterator
 */

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
     * \ingroup Test
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

