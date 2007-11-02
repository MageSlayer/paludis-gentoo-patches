/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <map>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct SecondIteratorTest : TestCase
    {
        SecondIteratorTest() : TestCase("second_iterator") {}

        void run()
        {
            typedef std::map<std::string, std::string> M;

            M m;
            m["I"] = "one";
            m["II"] = "two";
            m["III"] = "three";
            m["IV"] = "four";
            m["V"] = "five";

            SecondIteratorTypes<M::iterator>::Type it = second_iterator(m.begin());
            TEST_CHECK(it == it);
            TEST_CHECK(! (it != it));
            TEST_CHECK_EQUAL(*it, "one");
            TEST_CHECK_EQUAL(it->length(), 3U);

            SecondIteratorTypes<M::iterator>::Type it2(it);
            TEST_CHECK(it == it2);
            TEST_CHECK(! (it != it2));
            TEST_CHECK_EQUAL(*++it2, "two");
            TEST_CHECK_EQUAL(*it2, "two");
            TEST_CHECK_EQUAL(it2->length(), 3U);
            TEST_CHECK(it != it2);
            TEST_CHECK(! (it == it2));

            SecondIteratorTypes<M::iterator>::Type it3(it2);
            TEST_CHECK(it2 == it3++);
            TEST_CHECK(it2 != it3);
            TEST_CHECK_EQUAL(*it3, "three");
            TEST_CHECK_EQUAL(it3->length(), 5U);

            it3 = it2;
            TEST_CHECK(it2 == it3);
            TEST_CHECK_EQUAL(*it3, "two");
            TEST_CHECK_EQUAL(*it3++, "two");

            TEST_CHECK_EQUAL(join(second_iterator(m.begin()), second_iterator(m.end()), " "), "one two three four five");
        }
    } second_iterator_test;
}

