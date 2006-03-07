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

#include <paludis/qualified_package_name.hh>
#include <paludis/util/exception.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for QualifiedPackageName.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Basic QualifiedPackageName tests.
     *
     * \ingroup Test
     */
    struct QualifiedPackageNameTest : TestCase
    {
        QualifiedPackageNameTest() : TestCase("basic") { }

        void run()
        {
            QualifiedPackageName a("foo/bar1");
            TEST_CHECK(true);
        }
    } test_qualified_package_name;

    /**
     * \test Validate QualifiedPackageName tests.
     *
     * \ingroup Test
     */
    struct QualifiedPackageNameValidateTest : TestCase
    {
        QualifiedPackageNameValidateTest() : TestCase("validate") { }

        void run()
        {
            QualifiedPackageName a("foo/bar");
            TEST_CHECK_THROWS(a = QualifiedPackageName("moo"), NameError);
            TEST_CHECK_THROWS(a = QualifiedPackageName("foo/bar!"), NameError);
            TEST_CHECK_THROWS(a = QualifiedPackageName("foo/bar/baz"), NameError);
        }
    } test_qualified_package_name_validate;

    /**
     * \test Compare QualifiedPackageName tests.
     *
     * \ingroup Test
     */
    struct QualifiedPackageNameCompareTest : TestCase
    {
        QualifiedPackageNameCompareTest() : TestCase("compare") { }

        void run()
        {
            QualifiedPackageName foo1_bar1("foo1/bar1");
            QualifiedPackageName foo1_bar2("foo1/bar2");
            QualifiedPackageName foo2_bar1("foo2/bar1");

            TEST_CHECK( (foo1_bar1 <  foo1_bar2));
            TEST_CHECK( (foo1_bar1 <= foo1_bar2));
            TEST_CHECK(!(foo1_bar1 == foo1_bar2));
            TEST_CHECK( (foo1_bar1 != foo1_bar2));
            TEST_CHECK(!(foo1_bar1 >= foo1_bar2));
            TEST_CHECK(!(foo1_bar1 >  foo1_bar2));

            TEST_CHECK(!(foo2_bar1 <  foo1_bar2));
            TEST_CHECK(!(foo2_bar1 <= foo1_bar2));
            TEST_CHECK(!(foo2_bar1 == foo1_bar2));
            TEST_CHECK( (foo2_bar1 != foo1_bar2));
            TEST_CHECK( (foo2_bar1 >= foo1_bar2));
            TEST_CHECK( (foo2_bar1 >  foo1_bar2));
        }
    } test_qualified_package_name_compare;

}

