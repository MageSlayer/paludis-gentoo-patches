/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "package_name_part.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for package_name_part.hh .
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test PackageNamePart creation.
     *
     * \ingroup Test
     */
    struct PackageNamePartCreationTest : public TestCase
    {
        PackageNamePartCreationTest() : TestCase("creation") { }

        void run()
        {
            PackageNamePart p("foo");
            TEST_CHECK(true);
        }
    } test_package_name_part_creation;

    /**
     * \test Test PackageNamePart validation.
     *
     * \ingroup Test
     */
    struct PackageNamePartValidationTest : public TestCase
    {
        PackageNamePartValidationTest() : TestCase("validation") { }

        void run()
        {
            PackageNamePart p("foo-200dpi");
            TEST_CHECK_THROWS(p = PackageNamePart(""), NameError);
            TEST_CHECK_THROWS(p = PackageNamePart("!!!"), NameError);
            TEST_CHECK_THROWS(p = PackageNamePart("foo-2"), NameError);
            TEST_CHECK_THROWS(p = PackageNamePart("foo-200"), NameError);
        }
    } test_package_name_part_validation;

    /**
     * \test Test PackageNamePart comparison.
     *
     * \ingroup Test
     */
    struct PackageNamePartComparisonTest : public TestCase
    {
        PackageNamePartComparisonTest() : TestCase("comparison") { }

        void run()
        {
            PackageNamePart p1("p1");
            PackageNamePart p2("p2");

            TEST_CHECK( (p1 <  p2));
            TEST_CHECK( (p1 <= p2));
            TEST_CHECK(!(p1 == p2));
            TEST_CHECK(!(p1 >  p2));
            TEST_CHECK(!(p1 >= p2));
            TEST_CHECK( (p1 != p2));

            TEST_CHECK(!(p2 <  p2));
            TEST_CHECK( (p2 <= p2));
            TEST_CHECK( (p2 == p2));
            TEST_CHECK(!(p2 >  p2));
            TEST_CHECK( (p2 >= p2));
            TEST_CHECK(!(p2 != p2));

            TEST_CHECK(!(p2 <  p1));
            TEST_CHECK(!(p2 <= p1));
            TEST_CHECK(!(p2 == p1));
            TEST_CHECK( (p2 >  p1));
            TEST_CHECK( (p2 >= p1));
            TEST_CHECK( (p2 != p1));
        }
    } test_package_name_part_comparison;
}



