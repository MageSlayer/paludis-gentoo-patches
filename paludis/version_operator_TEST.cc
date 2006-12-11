/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/version_operator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for version_operator.hh.
 *
 */

namespace test_cases
{
    /**
     * \test Test VersionOperator creation and assignment.
     *
     */
    struct VersionOperatorTest : TestCase
    {
        VersionOperatorTest() : TestCase("version operator") { }

        void run()
        {
            VersionOperator v1(vo_greater);
            VersionOperator v2(vo_less_equal);
            TEST_CHECK(v1 != v2);
            TEST_CHECK(v1 == vo_greater);
            TEST_CHECK(v1 != vo_less_equal);
            TEST_CHECK(v2 == vo_less_equal);
            TEST_CHECK(v2 != vo_greater);

            VersionOperator v3(v1);
            TEST_CHECK(v1 == vo_greater);
            TEST_CHECK(v3 == vo_greater);
            TEST_CHECK(v1 == v3);

            v3 = v2;
            TEST_CHECK(v1 != v3);
            TEST_CHECK(v1 != v2);
            TEST_CHECK(v2 == v3);
            TEST_CHECK(v1 == vo_greater);
            TEST_CHECK(v2 == vo_less_equal);
            TEST_CHECK(v3 == vo_less_equal);
        }
    } test_version_operator;

    /**
     * \test Test VersionOperator creation and assignment from a string.
     *
     */
    struct VersionOperatorFromStringTest : TestCase
    {
        VersionOperatorFromStringTest() : TestCase("version operator from string") { }

        void run()
        {
            VersionOperator v1(">");
            VersionOperator v2("<=");
            TEST_CHECK(v1 != v2);
            TEST_CHECK(v1 == vo_greater);
            TEST_CHECK(v1 != vo_less_equal);
            TEST_CHECK(v2 == vo_less_equal);
            TEST_CHECK(v2 != vo_greater);

            TEST_CHECK_EQUAL(VersionOperator("<"),  VersionOperator(vo_less));
            TEST_CHECK_EQUAL(VersionOperator("<="), VersionOperator(vo_less_equal));
            TEST_CHECK_EQUAL(VersionOperator("="),  VersionOperator(vo_equal));
            TEST_CHECK_EQUAL(VersionOperator("~"),  VersionOperator(vo_tilde));
            TEST_CHECK_EQUAL(VersionOperator(">"),  VersionOperator(vo_greater));
            TEST_CHECK_EQUAL(VersionOperator(">="), VersionOperator(vo_greater_equal));
            TEST_CHECK_EQUAL(VersionOperator("~>"), VersionOperator(vo_tilde_greater));
        }
    } test_version_operator_from_string;

    /**
     * \test Test VersionOperator stringification.
     *
     */
    struct VersionOperatorToStringTest : TestCase
    {
        VersionOperatorToStringTest() : TestCase("version operator to string") { }

        void run()
        {
            TEST_CHECK_EQUAL(stringify(VersionOperator(vo_greater)), ">");
            TEST_CHECK_EQUAL(stringify(VersionOperator(vo_equal)), "=");
            TEST_CHECK_EQUAL(stringify(VersionOperator(vo_tilde)), "~");
            TEST_CHECK_EQUAL(stringify(VersionOperator(vo_tilde_greater)), "~>");
        }
    } test_version_operator_to_string;
}

