/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
 */

namespace test_cases
{
    /**
     * \test Basic version_spec creation.
     *
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
            VersionSpec v20("1_pre1-r1.2");
            VersionSpec v21("1_rc1-r1");
            VersionSpec v22("1_p1-r1");
            VersionSpec v23("1_alpha_p");
            VersionSpec v24("scm");

            TEST_CHECK(true);
        }
    } test_version_spec_creation;

    /**
     * \test Invalid version rejection
     *
     */
    struct VersionSpecRejectTest : TestCase
    {
        VersionSpecRejectTest() : TestCase("version spec reject") {}

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec v1(""), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("b"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r1_pre"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-pre"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_blah"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre-r2b"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre-r2-r2"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r2_pre"), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("10000000000000000000000000000000000000000000000000000"), BadVersionSpecError);
        }
    } test_version_spec_reject;

    /**
     * \test Not so basic version_spec creation.
     *
     */
    struct VersionSpecParseTest : TestCase
    {
        VersionSpecParseTest() : TestCase("version spec parse") { }

        void run()
        {
            VersionSpec v("1.2.3");
            TEST_CHECK(v == VersionSpec("1.2.3"));
            VersionSpec v1("1.2_pre2-r1");
            TEST_CHECK(v1 == VersionSpec("1.2_pre2-r1"));
            VersionSpec v2("1.2_pre2_rc5_p6-r1");
            TEST_CHECK(v2 == VersionSpec("1.2_pre2_rc5_p6-r1"));
            VersionSpec v3("1.2_pre2_pre3_pre4");
            TEST_CHECK(v3 == VersionSpec("1.2_pre2_pre3_pre4"));
        }
    } test_version_spec_parse;

    /**
     * \test VersionSpec star comparisons
     *
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
     * \test VersionSpec tilde comparisons
     *
     */
    struct VersionSpecTildeCompareTest : TestCase
    {
        VersionSpecTildeCompareTest() : TestCase("version spec tilde compare") {}

        void run()
        {
            TEST_CHECK(! VersionSpec("1.4-r1").tilde_compare(VersionSpec("1.3-r1")));
            TEST_CHECK(! VersionSpec("1.4").tilde_compare(VersionSpec("1.3-r1")));
            TEST_CHECK(! VersionSpec("1.2").tilde_compare(VersionSpec("1.3-r1")));
            TEST_CHECK(! VersionSpec("1.3").tilde_compare(VersionSpec("1.3-r1")));

            TEST_CHECK(VersionSpec("1.2").tilde_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("1.2-r1").tilde_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("1.2-r1.2.3").tilde_compare(VersionSpec("1.2")));
            TEST_CHECK(! VersionSpec("1.3").tilde_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("1.2-r2").tilde_compare(VersionSpec("1.2-r1")));
            TEST_CHECK(VersionSpec("1.2-r2.3").tilde_compare(VersionSpec("1.2-r1")));
            TEST_CHECK(VersionSpec("1.2-r2").tilde_compare(VersionSpec("1.2-r2")));
            TEST_CHECK(! VersionSpec("1.2-r1").tilde_compare(VersionSpec("1.2-r2")));
            TEST_CHECK(! VersionSpec("1.2-r1.3").tilde_compare(VersionSpec("1.2-r2")));
            TEST_CHECK(! VersionSpec("1.2-r2").tilde_compare(VersionSpec("1.2-r2.3")));
            TEST_CHECK(VersionSpec("1.2-r2.4").tilde_compare(VersionSpec("1.2-r2.3")));
        }
    } test_version_spec_tilde_compare;

    /**
     * \test VersionSpec tilde greater comparisons
     *
     */
    struct VersionSpecTildeGreaterCompareTest : TestCase
    {
        VersionSpecTildeGreaterCompareTest() : TestCase("version spec tilde greater compare") {}

        void run()
        {
            TEST_CHECK(VersionSpec("1.2").tilde_greater_compare(VersionSpec("1.2")));
            TEST_CHECK(VersionSpec("1.2.1").tilde_greater_compare(VersionSpec("1.2")));
            TEST_CHECK(! VersionSpec("1.1").tilde_greater_compare(VersionSpec("1.2")));
            TEST_CHECK(! VersionSpec("2.0").tilde_greater_compare(VersionSpec("1.2")));
        }
    } test_version_spec_tilde_greater_compare;

    /**
     * \test VersionSpec remove revision
     *
     */
    struct VersionRemoveRevisionTest : TestCase
    {
        VersionRemoveRevisionTest() : TestCase("version spec remove revision") {}

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2").remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r").remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99").remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r3.4").remove_revision(), "1.2");

            TEST_CHECK_EQUAL(VersionSpec("1.2").remove_revision(), VersionSpec("1.2"));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r").remove_revision(), VersionSpec("1.2"));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r99").remove_revision(), VersionSpec("1.2"));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r3.4").remove_revision(), VersionSpec("1.2"));
        }
    } test_version_remove_revision;

    /**
     * \test VersionSpec bump
     *
     */
    struct VersionBumpTest : TestCase
    {
        VersionBumpTest() : TestCase("version spec bump") {}

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2").bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99").bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2.3").bump(), "1.3");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1").bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("scm").bump(), "scm");
        }
    } test_version_bump;

    /**
     * \test VersionSpec revision only
     *
     */
    struct VersionRevisionOnlyTest : TestCase
    {
        VersionRevisionOnlyTest() : TestCase("version spec revision only") {}

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2").revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r").revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99").revision_only(), "r99");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r3.4").revision_only(), "r3.4");
        }
    } test_version_revision_only;

    /**
     * \test VersionSpec is_scm
     *
     */
    struct VersionIsScmTest : TestCase
    {
        VersionIsScmTest() : TestCase("version spec is_scm") {}

        void run()
        {
            TEST_CHECK(! VersionSpec("1.2").is_scm());
            TEST_CHECK(VersionSpec("1.2-scm-r99").is_scm());

            TEST_CHECK(! VersionSpec("1.2-r9998").is_scm());
            TEST_CHECK(VersionSpec("1.2-r9999").is_scm());

            TEST_CHECK(! VersionSpec("9998").is_scm());
            TEST_CHECK(! VersionSpec("9999_alpha2").is_scm());
            TEST_CHECK(VersionSpec("9999").is_scm());
            TEST_CHECK(VersionSpec("9999-r4").is_scm());
        }
    } test_version_is_scm;

    struct VersionSpecHashTest : TestCase
    {
        VersionSpecHashTest() : TestCase("version spec hash_value()") { }

        void run()
        {
            TEST_CHECK(VersionSpec("0").hash_value() != VersionSpec("0.0").hash_value());
            TEST_CHECK(VersionSpec("1").hash_value() != VersionSpec("1.0").hash_value());
            TEST_CHECK(VersionSpec("1.0").hash_value() != VersionSpec("1").hash_value());
            TEST_CHECK(VersionSpec("1.0_alpha").hash_value() != VersionSpec("1_alpha").hash_value());
            TEST_CHECK(VersionSpec("1_alpha").hash_value() != VersionSpec("1.0_alpha").hash_value());
        }
    } test_version_spec_hash_value;

    /**
     * \test VersionSpec ordering.
     *
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
            TEST_CHECK(VersionSpec("1.0") > VersionSpec("1"));
            TEST_CHECK(VersionSpec("1") < VersionSpec("1.0"));
            TEST_CHECK(VersionSpec("1.0_alpha") > VersionSpec("1_alpha"));
            TEST_CHECK(VersionSpec("1.0_alpha") > VersionSpec("1"));
            TEST_CHECK(VersionSpec("1.0_alpha") < VersionSpec("1.0"));
            TEST_CHECK(VersionSpec("1.2.0.0_alpha7-r4") > VersionSpec("1.2_alpha7-r4"));

            TEST_CHECK(VersionSpec("0001") == VersionSpec("1"));
            TEST_CHECK(VersionSpec("01") == VersionSpec("001"));
            TEST_CHECK(VersionSpec("0001.1") == VersionSpec("1.1"));
            TEST_CHECK(VersionSpec("01.01") == VersionSpec("1.01"));
            TEST_CHECK(VersionSpec("1.010") == VersionSpec("1.01"));
            TEST_CHECK(VersionSpec("1.00") == VersionSpec("1.0"));
            TEST_CHECK(VersionSpec("1.0100") == VersionSpec("1.010"));
            TEST_CHECK(VersionSpec("1") == VersionSpec("1-r0"));
            TEST_CHECK(VersionSpec("1.2") == VersionSpec("1.2-r"));
            TEST_CHECK(VersionSpec("1.2-r3") == VersionSpec("1.2-r3.0"));
            TEST_CHECK(VersionSpec("1.2") == VersionSpec("1.2-r0.0"));

            TEST_CHECK(VersionSpec("1_alpha_beta-scm") == VersionSpec("1_alpha0_beta-scm"));
            TEST_CHECK(VersionSpec("1_alpha_beta0_rc3-scm") == VersionSpec("1_alpha0_beta_rc3-scm"));

            TEST_CHECK(VersionSpec("0001").hash_value() == VersionSpec("1").hash_value());
            TEST_CHECK(VersionSpec("01").hash_value() == VersionSpec("001").hash_value());
            TEST_CHECK(VersionSpec("0001.1").hash_value() == VersionSpec("1.1").hash_value());
            TEST_CHECK(VersionSpec("01.01").hash_value() == VersionSpec("1.01").hash_value());
            TEST_CHECK(VersionSpec("1.010").hash_value() == VersionSpec("1.01").hash_value());
            TEST_CHECK(VersionSpec("1.00").hash_value() == VersionSpec("1.0").hash_value());
            TEST_CHECK(VersionSpec("1.0100").hash_value() == VersionSpec("1.010").hash_value());
            TEST_CHECK(VersionSpec("1").hash_value() == VersionSpec("1-r0").hash_value());
            TEST_CHECK(VersionSpec("1.2").hash_value() == VersionSpec("1.2-r").hash_value());
            TEST_CHECK(VersionSpec("1.2-r3").hash_value() == VersionSpec("1.2-r3.0").hash_value());
            TEST_CHECK(VersionSpec("1.2").hash_value() == VersionSpec("1.2-r0.0").hash_value());

            std::vector<VersionSpec> v;
            v.push_back(VersionSpec("1_alpha_alpha"));
            v.push_back(VersionSpec("1_alpha"));
            v.push_back(VersionSpec("1_alpha1_alpha"));
            v.push_back(VersionSpec("1_alpha1_beta_pre"));
            v.push_back(VersionSpec("1_alpha1_beta"));
            v.push_back(VersionSpec("1_alpha1"));
            v.push_back(VersionSpec("1_alpha1-r1"));
            v.push_back(VersionSpec("1_alpha10"));
            v.push_back(VersionSpec("1_alpha10-r1"));
            v.push_back(VersionSpec("1_alpha10-r1.1"));
            v.push_back(VersionSpec("1_alpha10-r1.2"));
            v.push_back(VersionSpec("1_alpha10-r2"));
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
            v.push_back(VersionSpec("1-try2"));
            v.push_back(VersionSpec("1p"));
            v.push_back(VersionSpec("1.0"));
            v.push_back(VersionSpec("1.0a"));
            v.push_back(VersionSpec("1.0.0"));
            v.push_back(VersionSpec("1.001"));
            v.push_back(VersionSpec("1.01"));
            v.push_back(VersionSpec("1.0101"));
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
            v.push_back(VersionSpec("1.2_beta_p1-scm"));
            v.push_back(VersionSpec("1.2_beta_p-scm"));
            v.push_back(VersionSpec("1.2_beta1_p-scm"));
            v.push_back(VersionSpec("1.2_beta10"));
            v.push_back(VersionSpec("1.2_beta10_p"));
            v.push_back(VersionSpec("1.2_beta10_p1"));
            v.push_back(VersionSpec("1.2_beta10_p1-scm"));
            v.push_back(VersionSpec("1.2_beta10_p10"));
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
            v.push_back(VersionSpec("09"));
            v.push_back(VersionSpec("10"));
            v.push_back(VersionSpec("100"));
            v.push_back(VersionSpec("scm"));
            v.push_back(VersionSpec("scm-r3"));
            v.push_back(VersionSpec("scm-r3.4"));

            std::vector<VersionSpec>::iterator v1(v.begin()), v_end(v.end());
            for ( ; v1 != v_end ; ++v1)
            {
                TestMessageSuffix s1("v1:" + stringify(*v1));
                std::vector<VersionSpec>::iterator v2(v.begin());
                for ( ; v2 != v_end ; ++v2)
                {
                    TestMessageSuffix s2("v2:" + stringify(*v2));
                    if (std::distance(v.begin(), v1) < std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v1 < *v2);
                        TEST_CHECK(*v2 > *v1);
                        TEST_CHECK(*v1 != *v2);
                        TEST_CHECK(*v2 != *v1);
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash_value()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash_value()));
                        TEST_CHECK(v1->hash_value() != v2->hash_value());
                        TEST_CHECK(v2->hash_value() != v1->hash_value());
                    }
                    else if (std::distance(v.begin(), v1) > std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v2 < *v1);
                        TEST_CHECK(*v1 > *v2);
                        TEST_CHECK(*v2 != *v1);
                        TEST_CHECK(*v1 != *v2);
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash_value()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash_value()));
                        TEST_CHECK(v1->hash_value() != v2->hash_value());
                        TEST_CHECK(v2->hash_value() != v1->hash_value());
                    }
                    else
                    {
                        TEST_CHECK(*v2 == *v1);
                        TEST_CHECK(*v1 == *v2);
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash_value()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash_value()));
                        TEST_CHECK(v1->hash_value() == v2->hash_value());
                        TEST_CHECK(v2->hash_value() == v1->hash_value());
                    }
                }
            }
        }
    } test_version_spec_compare;
}

