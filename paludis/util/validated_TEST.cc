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

#include <paludis/util/validated.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for validated.hh .
 *
 * \ingroup Test
 */

#ifndef DOXYGEN
struct PositiveEvenValidator
{
    struct NotValid
    {
    };

    static void validate(int value)
    {
        if ((value < 0) || (value % 2))
            throw NotValid();
    }
};

typedef Validated<int, PositiveEvenValidator> PositiveEven;
#endif

namespace test_cases
{
    /**
     * \test Validated<PositiveEven> tests.
     *
     * \ingroup Test
     */
    struct ValidatedPositiveEvenTests : TestCase
    {
        ValidatedPositiveEvenTests() : TestCase("Validated<PositiveEven> tests") { }

        void run()
        {
            PositiveEven v(2);
            TEST_CHECK_EQUAL(v, PositiveEven(2));
            v = PositiveEven(4);
            TEST_CHECK_EQUAL(v, PositiveEven(4));
            TEST_CHECK_THROWS(((v = PositiveEven(5))), PositiveEvenValidator::NotValid);
            TEST_CHECK_EQUAL(v, PositiveEven(4));
            TEST_CHECK_THROWS(PositiveEven w(5), PositiveEvenValidator::NotValid);
        }
    } test_validated_positive_even;

    /**
     * \test Validated<PositiveEven> comparison tests.
     *
     * \ingroup Test
     */
    struct ValidatedPositiveEvenComparisonTests : TestCase
    {
        ValidatedPositiveEvenComparisonTests() :
            TestCase("Validated<PositiveEven> comparison tests") { }

        void run()
        {
            PositiveEven v2(2);
            PositiveEven v4(4);
            PositiveEven v4b(4);

            TEST_CHECK(v2 <  v4);
            TEST_CHECK(v2 <= v4);
            TEST_CHECK(! (v2 == v4));
            TEST_CHECK(v2 != v4);
            TEST_CHECK(! (v2 >= v4));
            TEST_CHECK(! (v2 >  v4));

            TEST_CHECK(! (v4 <  v2));
            TEST_CHECK(! (v4 <= v2));
            TEST_CHECK(! (v4 == v2));
            TEST_CHECK(v4 != v2);
            TEST_CHECK(v4 >= v2);
            TEST_CHECK(v4 >  v2);

            TEST_CHECK(! (v2 <  v2));
            TEST_CHECK(v2 <= v2);
            TEST_CHECK(v2 == v2);
            TEST_CHECK(! (v2 != v2));
            TEST_CHECK(v2 >= v2);
            TEST_CHECK(! (v2 >  v2));

            TEST_CHECK(! (v4 <  v4));
            TEST_CHECK(v4 <= v4);
            TEST_CHECK(v4 == v4);
            TEST_CHECK(! (v4 != v4));
            TEST_CHECK(v4 >= v4);
            TEST_CHECK(! (v4 >  v4));

            TEST_CHECK(! (v4 <  v4b));
            TEST_CHECK(v4 <= v4b);
            TEST_CHECK(v4 == v4b);
            TEST_CHECK(! (v4 != v4b));
            TEST_CHECK(v4 >= v4b);
            TEST_CHECK(! (v4 >  v4b));
        }
    } test_validated_positive_even_comparisons;
}

