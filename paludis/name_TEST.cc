/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/name.hh>
#include <paludis/util/exception.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for QualifiedPackageName.
 *
 */

namespace test_cases
{
    /**
     * \test Basic QualifiedPackageName tests.
     *
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

    /**
     * \test Test CategoryNamePart creation.
     *
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

    /**
     * \test Test PackageNamePart creation.
     *
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

    /**
     * \test Test RepositoryName creation.
     *
     */
    struct RepositoryNameCreationTest : public TestCase
    {
        RepositoryNameCreationTest() : TestCase("creation") { }

        void run()
        {
            RepositoryName r("foo");
            TEST_CHECK(true);
        }
    } test_repository_name_creation;

    /**
     * \test Test RepositoryName validation.
     *
     */
    struct RepositoryNameValidationTest : public TestCase
    {
        RepositoryNameValidationTest() : TestCase("validation") { }

        void run()
        {
            RepositoryName r("repo0_-");
            TEST_CHECK_THROWS(r = RepositoryName(""), NameError);
            TEST_CHECK_THROWS(r = RepositoryName("!!!"), NameError);
            TEST_CHECK_THROWS(r = RepositoryName("-foo"), NameError);
            TEST_CHECK_THROWS(r = RepositoryName("fo$o"), NameError);
        }
    } test_repository_name_validation;

    /**
     * \test Test SlotName creation.
     *
     */
    struct SlotNameCreationTest : public TestCase
    {
        SlotNameCreationTest() : TestCase("creation") { }

        void run()
        {
            SlotName s("foo");
            TEST_CHECK(true);
        }
    } test_slot_name_creation;

    /**
     * \test Test SlotName validation.
     *
     */
    struct SlotNameValidationTest : public TestCase
    {
        SlotNameValidationTest() : TestCase("validation") { }

        void run()
        {
            SlotName s("slot0+_.-");
            TEST_CHECK_THROWS(s = SlotName(""), NameError);
            TEST_CHECK_THROWS(s = SlotName("!!!"), NameError);
            TEST_CHECK_THROWS(s = SlotName("-foo"), NameError);
            TEST_CHECK_THROWS(s = SlotName(".foo"), NameError);
            TEST_CHECK_THROWS(s = SlotName("fo$o"), NameError);
        }
    } test_slot_name_validation;

    /**
     * \test Test UseFlagName creation.
     *
     */
    struct UseFlagNameCreationTest : public TestCase
    {
        UseFlagNameCreationTest() : TestCase("creation") { }

        void run()
        {
            UseFlagName u("foo");
            TEST_CHECK(true);
        }
    } test_use_flag_name_creation;

    /**
     * \test Test UseFlagName validation.
     *
     */
    struct UseFlagNameValidationTest : public TestCase
    {
        UseFlagNameValidationTest() : TestCase("validation") { }

        void run()
        {
            UseFlagName u("use0+_x@-x");
            TEST_CHECK_THROWS(u = UseFlagName(""), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("!!!"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("-foo"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("_foo"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("@foo"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("+foo"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("fo$o"), NameError);

            TEST_CHECK_THROWS(u = UseFlagName("foo:"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName(":foo"), NameError);
            TEST_CHECK_THROWS(u = UseFlagName("foo:_"), NameError);
        }
    } test_use_flag_name_validation;

    /**
     * \test Test KeywordName creation.
     *
     */
    struct KeywordNameCreationTest : public TestCase
    {
        KeywordNameCreationTest() : TestCase("creation") { }

        void run()
        {
            KeywordName k("foo");
            TEST_CHECK(true);
        }
    } test_keyword_name_creation;

    /**
     * \test Test KeywordName validation.
     *
     */
    struct KeywordNameValidationTest : public TestCase
    {
        KeywordNameValidationTest() : TestCase("validation") { }

        void run()
        {
            KeywordName k("keyword0_-");
            KeywordName k1("~keyword0_-");
            KeywordName k2("-keyword0_-");
            KeywordName k3("*");
            KeywordName k4("-*");
            TEST_CHECK_THROWS(k = KeywordName(""), NameError);
            TEST_CHECK_THROWS(k = KeywordName("!!!"), NameError);
            TEST_CHECK_THROWS(k = KeywordName("~"), NameError);
            TEST_CHECK_THROWS(k = KeywordName("-"), NameError);
            TEST_CHECK_THROWS(k = KeywordName("@foo"), NameError);
            TEST_CHECK_THROWS(k = KeywordName("fo$o"), NameError);
            TEST_CHECK_THROWS(k = KeywordName("foo+"), NameError);
        }
    } test_keyword_name_validation;

    struct SetNameValidationTest : public TestCase
    {
        SetNameValidationTest() : TestCase("validation") { }

        void run()
        {
            SetName k("set0_-");
            SetName k1("set0_-");
            SetName k2("set0*");
            TEST_CHECK_THROWS(k = SetName(""), NameError);
            TEST_CHECK_THROWS(k = SetName("!!!"), NameError);
            TEST_CHECK_THROWS(k = SetName("~"), NameError);
            TEST_CHECK_THROWS(k = SetName("-"), NameError);
            TEST_CHECK_THROWS(k = SetName("f?oo"), NameError);
            TEST_CHECK_THROWS(k = SetName("*"), NameError);
            TEST_CHECK_THROWS(k = SetName("?"), NameError);
            TEST_CHECK_THROWS(k = SetName("*set"), NameError);
            TEST_CHECK_THROWS(k = SetName("set**"), NameError);
            TEST_CHECK_THROWS(k = SetName("set*?"), NameError);
            TEST_CHECK_THROWS(k = SetName("set?"), NameError);
        }
    } test_set_name_validator;
}

