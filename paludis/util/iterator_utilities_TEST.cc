/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser <halcy0n@gentoo.org>
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

#include <paludis/util/iterator_utilities.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>
#include <vector>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for iterator_utilities.hh .
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test iterator_utilities next()
     *
     * \ingroup Test
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
     * \ingroup Test
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

