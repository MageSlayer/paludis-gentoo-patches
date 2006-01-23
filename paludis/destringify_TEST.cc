/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett <spb@gentoo.org>
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

#include "destringify.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for destringify.hh
 *
 * \ingroup Test
 */

namespace test_cases
{
    /** \test
     * Test destringify for integers.
     *
     * \ingroup Test
     */
    struct DestringifyIntTests : TestCase
    {
        DestringifyIntTests() : TestCase("destringify int") { }

        void run()
        {
            TEST_CHECK_EQUAL(destringify<int>("0"),     0);
            TEST_CHECK_EQUAL(destringify<int>("1"),     1);
            TEST_CHECK_EQUAL(destringify<int>("99"),    99);
            TEST_CHECK_EQUAL(destringify<int>("-99"),   -99);
            TEST_CHECK_EQUAL(destringify<int>(" 12345"), 12345);
        }
    } test_case_destringify_int;

    struct DestringifyFloatTests : TestCase
    {
        DestringifyFloatTests() : TestCase("destringify float") { }

        void run()
        {
            TEST_CHECK_EQUAL(destringify<float>("0"),     0.f);
            TEST_CHECK_EQUAL(destringify<float>("0.0"),   0.f);
            TEST_CHECK_EQUAL(destringify<float>("0.1"),   0.1f);
            TEST_CHECK_EQUAL(destringify<float>("-1.54"), -1.54f);
        }
    } test_case_destringify_float;

    struct DestringifyStringTests : TestCase
    {
        DestringifyStringTests() : TestCase("destringify string") { }

        void run()
        {
            TEST_CHECK_EQUAL(destringify<std::string>("asdf"),           "asdf");
            TEST_CHECK_EQUAL(destringify<std::string>("  a f     e b "), "  a f     e b ");
        }
    } test_case_destringify_string;

    struct DestringifyBoolTests : TestCase
    {
        DestringifyBoolTests() : TestCase("destringify bool") { }

        void run()
        {
            TEST_CHECK( destringify<bool>("true"));
            TEST_CHECK( destringify<bool>("1"));
            TEST_CHECK( destringify<bool>("5"));
            TEST_CHECK(!destringify<bool>("false"));
            TEST_CHECK(!destringify<bool>("0"));
            TEST_CHECK(!destringify<bool>("-1"));
        }
    } test_case_destringify_bool;
}

