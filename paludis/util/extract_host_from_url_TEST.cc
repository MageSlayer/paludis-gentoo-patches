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

#include <paludis/util/extract_host_from_url.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TestExtractHostFromURL : TestCase
    {
        TestExtractHostFromURL() : TestCase("extract host from url") { }

        void run()
        {
            TEST_CHECK_EQUAL(extract_host_from_url(""), "");
            TEST_CHECK_EQUAL(extract_host_from_url("http://foo"), "foo");
            TEST_CHECK_EQUAL(extract_host_from_url("http://foo/bar"), "foo");
            TEST_CHECK_EQUAL(extract_host_from_url("http://foo/bar/baz"), "foo");
        }
    } test_extract_host_from_url;
}
