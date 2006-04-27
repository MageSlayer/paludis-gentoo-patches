/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/version_spec.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <iterator>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VersionSpec.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Basic version_spec creation.
     *
     * \ingroup grptestcases
     */
    struct VersionSpecCreationTest : TestCase
    {
        VersionSpecCreationTest() : TestCase("version spec creation") { }

        void run()
        {
            VersionSpec v("1");
            VersionSpec v1("1b");
            VersionSpec v2("1_alpha");
            VersionSpec v3("1_beta");
            VersionSpec v4("1_pre");
            VersionSpec v5("1_rc");
            VersionSpec v6("1_p");
            VersionSpec v7("1_alpha1");
            VersionSpec v8("1_beta1");
            VersionSpec v9("1_pre1");
            VersionSpec v10("1_rc1");
            VersionSpec v11("1_p1");
            VersionSpec v12("1_alpha-r1");
            VersionSpec v13("1_beta-r1");
            VersionSpec v14("1_pre-r1");
            VersionSpec v15("1_rc-r1");
            VersionSpec v16("1_p-r1");
            VersionSpec v17("1_alpha1-r1");
            VersionSpec v18("1_beta1-r1");
            VersionSpec v19("1_pre1-r1");
            VersionSpec v20("1_rc1-r1");
            VersionSpec v21("1_p1-r1");
            VersionSpec v22("1_alpha_p");


            TEST_CHECK(true);
        }
    } test_version_spec_creation;

    /**
     * \test Invalid version rejection
     *
     * \ingroup grptestcases
     */
    struct VersionSpecRejectTest : TestCase
    {
        VersionSpecRejectTest() : TestCase("version spec reject") {}

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec v1("b"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r1_pre"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-pre"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_blah"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre-r2b"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre_alpha"),
                    BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_p_alpha"), BadVersionSpecError);
        }
    } test_version_spec_reject;

    /**
     * \test Not so basic version_spec creation.
     *
     * \ingroup grptestcases
     */
    struct VersionSpecParseTest : TestCase
    {
        VersionSpecParseTest() : TestCase("version spec parse") { }

        void run()
        {
            VersionSpec v("1.2.3");
            TEST_CHECK(v == VersionSpec("1.2.3.0"));
            VersionSpec v1("1.2_pre2-r1");
            TEST_CHECK(v1 == VersionSpec("1.2.0_pre2-r1"));
        }
    } test_version_spec_parse;

    /**
     * \test VersionSpec star comparisons
     *
     * \ingroup grptestcases
     */
    struct VersionSpecStarCompareTest : TestCase
    {
        VersionSpecStarCompareTest() : TestCase("version spec star compare") {}

        void run()
        {
            TEST_CHECK(VersionSpec("1.2").equal_star_compare(VersionSpec("1")));
            TEST_CHECK(VersionSpec("1.2").equal_star_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("1.2.1").equal_star_compare(VersionSpec("1")));
            TEST_CHECK(VersionSpec("1.2.1").equal_star_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("2.2").equal_star_compare(VersionSpec("2")));
            TEST_CHECK(VersionSpec("2").equal_star_compare(VersionSpec("2")));
            TEST_CHECK(VersionSpec("2.59").equal_star_compare(VersionSpec("2.5")));
            TEST_CHECK(VersionSpec("2.59_alpha5-r1").equal_star_compare(VersionSpec("2.59_alpha")));
            TEST_CHECK(! VersionSpec("2").equal_star_compare(VersionSpec("2.5")));
            TEST_CHECK(! VersionSpec("2.59").equal_star_compare(VersionSpec("2.50")));
            TEST_CHECK(! VersionSpec("1").equal_star_compare(VersionSpec("2")));

        }
    } test_version_spec_star_compare;

    /**
     * \test VersionSpec remove revision
     *
     * \ingroup grptestcases
     */
    struct VersionRemoveRevisionTest : TestCase
    {
        VersionRemoveRevisionTest() : TestCase("version spec remove revision") {}

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2").remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r").remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99").remove_revision(), "1.2");
        }
    } test_version_remove_revision;

    /**
     * \test VersionSpec revision only
     *
     * \ingroup grptestcases
     */
    struct VersionRevisionOnlyTest : TestCase
    {
        VersionRevisionOnlyTest() : TestCase("version spec revision only") {}

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2").revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r").revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99").revision_only(), "r99");
        }
    } test_version_revision_only;

    /**
     * \test VersionSpec ordering.
     *
     * \ingroup grptestcases
     */
    struct VersionSpecCompareSCMTest : TestCase
    {
        VersionSpecCompareSCMTest() : TestCase("version spec compare") {}

        virtual unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TEST_CHECK(VersionSpec("1.0") == VersionSpec("1"));
            TEST_CHECK(VersionSpec("1") == VersionSpec("1.0"));
            TEST_CHECK(! (VersionSpec("1") < VersionSpec("1.0")));
            TEST_CHECK(! (VersionSpec("1") > VersionSpec("1.0")));
            TEST_CHECK(! (VersionSpec("1.0") < VersionSpec("1")));
            TEST_CHECK(! (VersionSpec("1.0") > VersionSpec("1")));
            TEST_CHECK(VersionSpec("1.0_alpha") == VersionSpec("1_alpha"));
            TEST_CHECK(VersionSpec("1_alpha") == VersionSpec("1.0_alpha"));
            TEST_CHECK(! (VersionSpec("1_alpha") < VersionSpec("1.0_alpha")));
            TEST_CHECK(! (VersionSpec("1_alpha") > VersionSpec("1.0_alpha")));
            TEST_CHECK(! (VersionSpec("1.0_alpha") < VersionSpec("1_alpha")));
            TEST_CHECK(! (VersionSpec("1.0_alpha") > VersionSpec("1_alpha")));

            std::vector<VersionSpec> v;
            v.push_back(VersionSpec("1_alpha"));
            v.push_back(VersionSpec("1_alpha10"));
            v.push_back(VersionSpec("1_alpha10-r1"));
            v.push_back(VersionSpec("1_alpha10_p1"));
            v.push_back(VersionSpec("1_alpha10_p1-r1"));
            v.push_back(VersionSpec("1_alpha11"));
            v.push_back(VersionSpec("1_beta"));
            v.push_back(VersionSpec("1_beta10"));
            v.push_back(VersionSpec("1_beta10-r1"));
            v.push_back(VersionSpec("1_beta10_p1"));
            v.push_back(VersionSpec("1_beta10_p1-r1"));
            v.push_back(VersionSpec("1_beta11"));
            v.push_back(VersionSpec("1_pre"));
            v.push_back(VersionSpec("1_pre10"));
            v.push_back(VersionSpec("1_pre10-r1"));
            v.push_back(VersionSpec("1_pre10_p1"));
            v.push_back(VersionSpec("1_pre10_p1-r1"));
            v.push_back(VersionSpec("1_pre11"));
            v.push_back(VersionSpec("1_rc"));
            v.push_back(VersionSpec("1_rc10"));
            v.push_back(VersionSpec("1_rc10-r1"));
            v.push_back(VersionSpec("1_rc10_p1"));
            v.push_back(VersionSpec("1_rc10_p1-r1"));
            v.push_back(VersionSpec("1_rc11"));
            v.push_back(VersionSpec("1"));
            v.push_back(VersionSpec("1-r1"));
            v.push_back(VersionSpec("1_p1"));
            v.push_back(VersionSpec("1p"));
            v.push_back(VersionSpec("1.1_alpha3"));
            v.push_back(VersionSpec("1.1"));
            v.push_back(VersionSpec("1.1-r1"));
            v.push_back(VersionSpec("1.1.1"));
            v.push_back(VersionSpec("1.1.1-scm"));
            v.push_back(VersionSpec("1.1.2"));
            v.push_back(VersionSpec("1.1-scm"));
            v.push_back(VersionSpec("1.1-scm-r1"));
            v.push_back(VersionSpec("1.2_alpha"));
            v.push_back(VersionSpec("1.2_alpha-scm"));
            v.push_back(VersionSpec("1.2_beta"));
            v.push_back(VersionSpec("1.2_beta10"));
            v.push_back(VersionSpec("1.2_beta10_p1"));
            v.push_back(VersionSpec("1.2_beta10_p1-scm"));
            v.push_back(VersionSpec("1.2_beta10-scm"));
            v.push_back(VersionSpec("1.2_beta11"));
            v.push_back(VersionSpec("1.2_beta11-scm"));
            v.push_back(VersionSpec("1.2_beta-scm"));
            v.push_back(VersionSpec("1.2"));
            v.push_back(VersionSpec("1.2-r1"));
            v.push_back(VersionSpec("1.2-scm"));
            v.push_back(VersionSpec("1.2-scm-r1"));
            v.push_back(VersionSpec("1-scm"));
            v.push_back(VersionSpec("2_alpha"));
            v.push_back(VersionSpec("scm"));
            v.push_back(VersionSpec("scm-r3"));

            std::vector<VersionSpec>::iterator v1(v.begin()), v_end(v.end());
            for ( ; v1 != v_end ; ++v1)
            {
                TestMessageSuffix s1("v1:" + stringify(*v1), false);
                std::vector<VersionSpec>::iterator v2(v.begin());
                for ( ; v2 != v_end ; ++v2)
                {
                    TestMessageSuffix s2("v2:" + stringify(*v2), false);
                    if (std::distance(v.begin(), v1) < std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v1 < *v2);
                        TEST_CHECK(*v2 > *v1);
                        TEST_CHECK(*v1 != *v2);
                        TEST_CHECK(*v2 != *v1);
                    }
                    else if (std::distance(v.begin(), v1) > std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v2 < *v1);
                        TEST_CHECK(*v1 > *v2);
                        TEST_CHECK(*v2 != *v1);
                        TEST_CHECK(*v1 != *v2);
                    }
                    else
                    {
                        TEST_CHECK(*v2 == *v1);
                        TEST_CHECK(*v1 == *v2);
                    }
                }
            }
        }
    } test_version_spec_compare;
}

