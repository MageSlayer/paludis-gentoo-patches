/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh
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

#include <paludis/about.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/**
 * \file
 * Test cases for about.hh .
 *
 */

namespace test_cases
{
    /**
     * \test Version tests.
     *
     */
    struct VersionTest : TestCase
    {
        VersionTest() : TestCase("about test") { }

        void run()
        {
            TEST_CHECK(PALUDIS_VERSION_MAJOR >= 0);
            TEST_CHECK(PALUDIS_VERSION_MAJOR <= 9);

            TEST_CHECK(PALUDIS_VERSION_MINOR >= 0);
            TEST_CHECK(PALUDIS_VERSION_MINOR <= 99);

            TEST_CHECK(PALUDIS_VERSION_MICRO >= 0);
            TEST_CHECK(PALUDIS_VERSION_MICRO <= 99);

            TEST_CHECK(PALUDIS_VERSION >= 0);
            TEST_CHECK(PALUDIS_VERSION <= 99999);
            TEST_CHECK_EQUAL(PALUDIS_VERSION, 10000 * PALUDIS_VERSION_MAJOR +
                    100 * PALUDIS_VERSION_MINOR + PALUDIS_VERSION_MICRO);

            TEST_CHECK(std::string(PALUDIS_GIT_HEAD) != "i am a fish");
        }
    } test_case_about;

    /**
     * \test Build info tests.
     *
     */
    struct BuildInfoTest : TestCase
    {
        BuildInfoTest() : TestCase("build info test") { }

        void run()
        {
            TEST_CHECK(! std::string(PALUDIS_BUILD_CXX).empty());
            TEST_CHECK(! std::string(PALUDIS_BUILD_CXXFLAGS).empty());
            TEST_CHECK(std::string(PALUDIS_BUILD_LDFLAGS) != "i am a fish");

            TEST_CHECK(! std::string(PALUDIS_BUILD_USER).empty());
            TEST_CHECK(! std::string(PALUDIS_BUILD_HOST).empty());
            TEST_CHECK(! std::string(PALUDIS_BUILD_DATE).empty());
        }
    } test_case_build_info;
}

