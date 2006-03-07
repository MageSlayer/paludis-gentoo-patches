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

#include <paludis/category_name_part.hh>
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for category_name_part.hh .
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test CategoryNamePart creation.
     *
     * \ingroup Test
     */
    struct CategoryNamePartCreationTest : public TestCase
    {
        CategoryNamePartCreationTest() : TestCase("creation") { }

        void run()
        {
            CategoryNamePart p("foo");
            TEST_CHECK(true);
        }
    } category_package_name_part_creation;

    /**
     * \test Test CategoryNamePart validation
     *
     * \ingroup Test
     */
    struct CategoryNamePartValidationTest : public TestCase
    {
        CategoryNamePartValidationTest() : TestCase("validation") {}

        void run()
        {
            CategoryNamePart p = CategoryNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_");

            TEST_CHECK_THROWS(p = CategoryNamePart(""), CategoryNamePartError);
            TEST_CHECK_THROWS(p = CategoryNamePart("*"), CategoryNamePartError);
            TEST_CHECK_THROWS(p = CategoryNamePart("foo bar"), CategoryNamePartError);
        }
    } category_package_name_part_validation;
}

