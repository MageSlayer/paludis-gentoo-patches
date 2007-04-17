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


#include <algorithm>
#include <paludis/util/strip.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for strip.hh.
 *
 */

namespace test_cases
{
    /**
     * \test Test StripLeadingString.
     *
     */
    struct StripLeadingStringTest : TestCase
    {
        StripLeadingStringTest() : TestCase("StripLeadingString") { }

        void run()
        {
            TEST_CHECK("bar" == strip_leading_string("foobar", "foo"));
            TEST_CHECK("fishbar" == strip_leading_string("fishbar", "foo"));
            TEST_CHECK("" == strip_leading_string("foo", "foo"));
            TEST_CHECK("fishfoobar" == strip_leading_string("fishfoobar", "foo"));
            TEST_CHECK("blahfoo" == strip_leading_string("blahfoo", "foo"));
        }
    } test_strip_leading_string;

    /**
     * \test Test StripLeading.
     *
     */
    struct StripLeadingTest : TestCase
    {
        StripLeadingTest() : TestCase("StripLeading") {}

        void run()
        {
            TEST_CHECK("bar" == strip_leading("foobar", "foo"));
            TEST_CHECK("ishbar" == strip_leading("fishbar", "foo"));
            TEST_CHECK("" == strip_leading("foo", "foo"));
            TEST_CHECK("ishfoobar" == strip_leading("fishfoobar", "foo"));
            TEST_CHECK("blahfoo" == strip_leading("blahfoo", "foo"));
        }
    } test_strip_leading;

    /**
     * \test Test StripTrailingString.
     *
     */
    struct StripTrailingStringTest : TestCase
    {
        StripTrailingStringTest() : TestCase("StripTrailingString") { }

        void run()
        {
            TEST_CHECK("foobar" == strip_trailing_string("foobar", "foo"));
            TEST_CHECK("fishbar" == strip_trailing_string("fishbar", "foo"));
            TEST_CHECK("" == strip_trailing_string("foo", "foo"));
            TEST_CHECK("fishfoobar" == strip_trailing_string("fishfoobar", "foo"));
            TEST_CHECK("blah" == strip_trailing_string("blahfoo", "foo"));
        }
    } test_strip_trailing_string;

    /**
     * \test Test StripTrailing.
     *
     */
    struct StripTrailingTest : TestCase
    {
        StripTrailingTest() : TestCase("StripTrailing") {}

        void run()
        {
            TEST_CHECK("foobar" == strip_trailing("foobar", "foo"));
            TEST_CHECK("fishbar" == strip_trailing("fishbar", "foo"));
            TEST_CHECK("" == strip_trailing("foo", "foo"));
            TEST_CHECK("fishfoobar" == strip_trailing("fishfoobar", "foo"));
            TEST_CHECK("blah" == strip_trailing("blahfoo", "foo"));
        }
    } test_strip_trailing;
}

