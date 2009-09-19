/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/enum_iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    enum Numbers
    {
        one,
        two,
        three,
        last_number
    };
}

namespace test_cases
{
    struct EnumIteratorTest : TestCase
    {
        EnumIteratorTest() : TestCase("enum iterator") { }

        void run()
        {
            EnumIterator<Numbers> n, n_end(last_number);

            TEST_CHECK(n != n_end);
            TEST_CHECK_EQUAL(*n, one);
            ++n;

            TEST_CHECK(n != n_end);
            TEST_CHECK_EQUAL(*n, two);
            ++n;

            TEST_CHECK(n != n_end);
            TEST_CHECK_EQUAL(*n, three);
            ++n;

            TEST_CHECK(n == n_end);
            TEST_CHECK_EQUAL(*n, last_number);
        }
    } test_enum_iterator;
}

