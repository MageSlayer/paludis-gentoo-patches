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

#include "version_spec.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VersionSpec.
 *
 * \todo This needs lots of work.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Basic version_spec creation.
     *
     * \ingroup Test
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
     * \ingroup Test
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
     * \ingroup Test
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
     * \test VersionSpec comparisons
     *
     * \ingroup Test
     */
    struct VersionSpecCompareTest : TestCase
    {
        VersionSpecCompareTest() : TestCase("version spec compare") {}

        void run()
        {
            TEST_CHECK(VersionSpec("1") < VersionSpec("2"));
            TEST_CHECK(VersionSpec("3.10g") < VersionSpec("3.10.18"));
            TEST_CHECK(VersionSpec("4.0.2_pre20051120") <
                    VersionSpec("4.0.2_pre20051223"));
            TEST_CHECK(VersionSpec("1_alpha") < VersionSpec("1_beta"));
            TEST_CHECK(VersionSpec("1_beta") < VersionSpec("1_pre"));
            TEST_CHECK(VersionSpec("1_rc") < VersionSpec("1"));
            TEST_CHECK(VersionSpec("1") < VersionSpec("1_p0"));
            TEST_CHECK(VersionSpec("1_alpha2") < VersionSpec("1_beta1"));
            TEST_CHECK(VersionSpec("1_beta2") < VersionSpec("1_pre1"));
            TEST_CHECK(VersionSpec("1_rc3") < VersionSpec("1"));
            TEST_CHECK(VersionSpec("1") < VersionSpec("1_p2"));

        }
    } test_version_spec_compare;

    /**
     * \test VersionSpec star comparisons
     *
     * \ingroup Test
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
}

