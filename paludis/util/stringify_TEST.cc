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

#include <paludis/util/stringify.hh>
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for stringify.hh .
 *
 * \ingroup Test
 */

namespace test_cases
{
    /** \test
     * Test stringify on int.
     *
     * \ingroup Test
     */
    struct StringifyIntTests : TestCase
    {
        StringifyIntTests() : TestCase("stringify int") { }

        void run()
        {
            TEST_CHECK_EQUAL(stringify(0),     "0");
            TEST_CHECK_EQUAL(stringify(1),     "1");
            TEST_CHECK_EQUAL(stringify(99),    "99");
            TEST_CHECK_EQUAL(stringify(-99),   "-99");
            TEST_CHECK_EQUAL(stringify(12345), "12345");
        }
    } test_case_stringify_int;

    /** \test
     * Test stringify on char *.
     *
     * \ingroup Test
     */
    struct StringifyCharStarTests : TestCase
    {
        StringifyCharStarTests() : TestCase("stringify char *") { }

        void run()
        {
            TEST_CHECK_EQUAL(stringify("moo"), std::string("moo"));
            TEST_CHECK_EQUAL(stringify(""), std::string(""));
            TEST_CHECK(stringify("").empty());
            TEST_CHECK_EQUAL(stringify("  quack quack  "), std::string("  quack quack  "));
        }
    } test_case_stringify_char_star;

    /** \test
     * Test stringify on std::string.
     *
     * \ingroup Test
     */
    struct StringifyStringTests : TestCase
    {
        StringifyStringTests() : TestCase("stringify string") { }

        void run()
        {
            TEST_CHECK_EQUAL(stringify(std::string("moo")), std::string("moo"));
            TEST_CHECK_EQUAL(stringify(std::string("")), std::string(""));
            TEST_CHECK(stringify(std::string("")).empty());
            TEST_CHECK_EQUAL(stringify(std::string("  quack quack  ")), std::string("  quack quack  "));
        }
    } test_case_stringify_string;

    /** \test
     * Test stringify on char.
     *
     * \ingroup Test
     */
    struct StringifyCharTests : TestCase
    {
        StringifyCharTests() : TestCase("stringify char") { }

        void run()
        {
            char c('a');
            TEST_CHECK_EQUAL(stringify(c), std::string("a"));

            unsigned char u('a');
            TEST_CHECK_EQUAL(stringify(u), std::string("a"));

            signed char s('a');
            TEST_CHECK_EQUAL(stringify(s), std::string("a"));
        }
    } test_case_stringify_char;

    /** \test
     * Test stringify on bool.
     *
     * \ingroup Test
     */
    struct StringifyBoolTests : TestCase
    {
        StringifyBoolTests() : TestCase("stringify bool") { }

        void run()
        {
            TEST_CHECK_EQUAL(stringify(true), std::string("true"));
            TEST_CHECK_EQUAL(stringify(false), std::string("false"));
        }
    } test_case_stringify_bool;
}

