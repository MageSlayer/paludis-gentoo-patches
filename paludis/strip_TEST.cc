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


#include "strip.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for strip.hh.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test StripLeadingString.
     *
     * \ingroup Test
     */
    struct StripLeadingStringTest : TestCase
    {
        StripLeadingStringTest() : TestCase("StripLeadingString") { }

        void run()
        {
            StripLeadingString a("foo");

            TEST_CHECK("bar" == a("foobar"));
            TEST_CHECK("fishbar" == a("fishbar"));
            TEST_CHECK("" == a("foo"));
            TEST_CHECK("fishfoobar" == a("fishfoobar"));
            TEST_CHECK("blahfoo" == a("blahfoo"));
        }
    } test_strip_leading_string;

    /**
     * \test Test StripLeading.
     *
     * \ingroup Test
     */
    struct StripLeadingTest : TestCase
    {
        StripLeadingTest() : TestCase("StripLeading") {}

        void run()
        {
            StripLeading a("foo");

            TEST_CHECK("bar" == a("foobar"));
            TEST_CHECK("ishbar" == a("fishbar"));
            TEST_CHECK("" == a("foo"));
            TEST_CHECK("ishfoobar" == a("fishfoobar"));
            TEST_CHECK("blahfoo" == a("blahfoo"));
        }
    } test_strip_leading;

    /**
     * \test Test StripTrailingString.
     *
     * \ingroup Test
     */
    struct StripTrailingStringTest : TestCase
    {
        StripTrailingStringTest() : TestCase("StripTrailingString") { }

        void run()
        {
            StripTrailingString a("foo");

            TEST_CHECK("foobar" == a("foobar"));
            TEST_CHECK("fishbar" == a("fishbar"));
            TEST_CHECK("" == a("foo"));
            TEST_CHECK("fishfoobar" == a("fishfoobar"));
            TEST_CHECK("blah" == a("blahfoo"));
        }
    } test_strip_trailing_string;

    /**
     * \test Test StripTrailing.
     *
     * \ingroup Test
     */
    struct StripTrailingTest : TestCase
    {
        StripTrailingTest() : TestCase("StripTrailing") {}

        void run()
        {
            StripTrailing a("foo");

            TEST_CHECK("foobar" == a("foobar"));
            TEST_CHECK("fishbar" == a("fishbar"));
            TEST_CHECK("" == a("foo"));
            TEST_CHECK("fishfoobar" == a("fishfoobar"));
            TEST_CHECK("blah" == a("blahfoo"));
        }
    } test_strip_trailing;

}

