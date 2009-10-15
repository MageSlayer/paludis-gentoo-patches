/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Victor Meyerson
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

#include <clocale>
#include <cstdlib>
#include <paludis/util/pretty_print.hh>
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test case for pretty_print.hh .
 *
 */

namespace test_cases
{
    /** \test
     * Test pretty_print_bytes.
     *
     */
    struct PrettyPrintBytesTests : TestCase
    {
        PrettyPrintBytesTests() : TestCase("pretty_print_bytes") { }

        void run()
        {
            TEST_CHECK_EQUAL(pretty_print_bytes(0),          "0 Bytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1),          "1 Bytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(100),        "100 Bytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1024),       "1.00 kBytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1536),       "1.50 kBytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1048576),    "1.00 MBytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1953497),    "1.86 MBytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1073741824), "1.00 GBytes");
            TEST_CHECK_EQUAL(pretty_print_bytes(1537598292), "1.43 GBytes");
        }
    } test_case_pretty_print_bytes;

    struct PrettyPrintTimeTests : TestCase
    {
        PrettyPrintTimeTests() : TestCase("pretty_print_time") { }

        void run()
        {
            std::setlocale(LC_TIME, "C");
            setenv("TZ", "America/New_York", 1);
            TEST_CHECK_EQUAL(pretty_print_time(1234567890), "Fri Feb 13 18:31:30 EST 2009");
            TEST_CHECK_EQUAL(pretty_print_time(1255617780), "Thu Oct 15 10:43:00 EDT 2009");
        }
    } test_case_pretty_print_time;
}

