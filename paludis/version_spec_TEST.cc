/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
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
            VersionSpec v("1", VersionSpecOptions());
            VersionSpec v1("1b", VersionSpecOptions());
            VersionSpec v2("1_alpha", VersionSpecOptions());
            VersionSpec v3("1_beta", VersionSpecOptions());
            VersionSpec v4("1_pre", VersionSpecOptions());
            VersionSpec v5("1_rc", VersionSpecOptions());
            VersionSpec v6("1_p", VersionSpecOptions());
            VersionSpec v7("1_alpha1", VersionSpecOptions());
            VersionSpec v8("1_beta1", VersionSpecOptions());
            VersionSpec v9("1_pre1", VersionSpecOptions());
            VersionSpec v10("1_rc1", VersionSpecOptions());
            VersionSpec v11("1_p1", VersionSpecOptions());
            VersionSpec v12("1_alpha-r1", VersionSpecOptions());
            VersionSpec v13("1_beta-r1", VersionSpecOptions());
            VersionSpec v14("1_pre-r1", VersionSpecOptions());
            VersionSpec v15("1_rc-r1", VersionSpecOptions());
            VersionSpec v16("1_p-r1", VersionSpecOptions());
            VersionSpec v17("1_alpha1-r1", VersionSpecOptions());
            VersionSpec v18("1_beta1-r1", VersionSpecOptions());
            VersionSpec v19("1_pre1-r1", VersionSpecOptions());
            VersionSpec v20("1_pre1-r1.2", VersionSpecOptions());
            VersionSpec v21("1_rc1-r1", VersionSpecOptions());
            VersionSpec v22("1_p1-r1", VersionSpecOptions());
            VersionSpec v23("1_alpha_p", VersionSpecOptions());
            VersionSpec v24("1_p3_alpha", VersionSpecOptions());
            VersionSpec v25("1_p4_p-r2", VersionSpecOptions());
            VersionSpec v26("scm", VersionSpecOptions());

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
            TEST_CHECK_THROWS(VersionSpec v1("", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("b", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r1_pre", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-pre", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_blah", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre-r2b", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_pre-r2-r2", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-try-try", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-try_alpha", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-scm-scm", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-scm-try", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-scm_alpha", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r2_pre", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1.", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1.1.", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1.-r", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1.-r1", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r.0", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1-r1.", VersionSpecOptions()), BadVersionSpecError);
            TEST_CHECK_THROWS(VersionSpec v1("1_p1.", VersionSpecOptions()), BadVersionSpecError);
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
            VersionSpec v("1.2.3", VersionSpecOptions());
            TEST_CHECK(v == VersionSpec("1.2.3", VersionSpecOptions()));
            VersionSpec v1("1.2_pre2-r1", VersionSpecOptions());
            TEST_CHECK(v1 == VersionSpec("1.2_pre2-r1", VersionSpecOptions()));
            VersionSpec v2("1.2_pre2_rc5_p6-r1", VersionSpecOptions());
            TEST_CHECK(v2 == VersionSpec("1.2_pre2_rc5_p6-r1", VersionSpecOptions()));
            VersionSpec v3("1.2_pre2_pre3_pre4", VersionSpecOptions());
            TEST_CHECK(v3 == VersionSpec("1.2_pre2_pre3_pre4", VersionSpecOptions()));
        }
    } test_version_spec_parse;

    /**
     * \test VersionSpec stupid star comparisons
     *
     */
    struct VersionSpecStupidStarCompareTest : TestCase
    {
        VersionSpecStupidStarCompareTest() : TestCase("version spec stupid star compare") {}

        void run()
        {
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.2", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.59", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.5", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.59_alpha5-r1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.59_alpha", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.5", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.59", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.50", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2", VersionSpecOptions())));

            TEST_CHECK(! VersionSpec("01", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.02", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1.020", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.020", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1.02", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_alpha01", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha01", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_alpha1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_alpha01", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_alpha0", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_pre1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_p", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_pre-scm", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_pre", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_pre1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_pre0", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_alpha-r1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("1_beta", VersionSpecOptions())));

            TEST_CHECK(VersionSpec("010", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("010", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("010", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.010", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.0105", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.010", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.0135", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.010", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.010.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.011.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.010.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01.1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.011.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01.1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.10", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.1", VersionSpecOptions())));
        }
    } test_version_spec_stupid_star_compare;

    /**
     * \test VersionSpec nice star comparisons
     *
     */
    struct VersionSpecNiceStarCompareTest : TestCase
    {
        VersionSpecNiceStarCompareTest() : TestCase("version spec nice star compare") {}

        void run()
        {
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2.1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2.1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.2", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.59", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.5", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.59_alpha5-r1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.59_alpha", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.5", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.59", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.50", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2", VersionSpecOptions())));

            TEST_CHECK(VersionSpec("01", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.02", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1.020", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.020", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1.02", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_alpha1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_alpha01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_alpha01", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_alpha1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha01", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_alpha0", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_pre1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_p", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1_pre-scm", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_pre", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_pre1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_pre0", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_alpha-r1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1_alpha1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("1_beta", VersionSpecOptions())));

            TEST_CHECK(VersionSpec("010", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("010", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("010", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.010", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.0105", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.010", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.0135", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.010", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.010.1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.011.1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.01", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("2.010.1", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.01.1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.011.1", VersionSpecOptions()).stupid_equal_star_compare(VersionSpec("2.01.1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.10", VersionSpecOptions()).nice_equal_star_compare(VersionSpec("2.1", VersionSpecOptions())));
        }
    } test_version_spec_nice_star_compare;

    /**
     * \test VersionSpec tilde comparisons
     *
     */
    struct VersionSpecTildeCompareTest : TestCase
    {
        VersionSpecTildeCompareTest() : TestCase("version spec tilde compare") {}

        void run()
        {

            TEST_CHECK(! VersionSpec("1.4-r1", VersionSpecOptions()).tilde_compare(VersionSpec("1.3-r1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.4", VersionSpecOptions()).tilde_compare(VersionSpec("1.3-r1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2", VersionSpecOptions()).tilde_compare(VersionSpec("1.3-r1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.3-r1", VersionSpecOptions())));

            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r1.2.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r2.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r1.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r2.3", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r2.4", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r2.3", VersionSpecOptions())));

            TEST_CHECK(VersionSpec("1.2-r0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r0.0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.0", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r0.0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.0", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r0.1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r0", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.1", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2-r1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.1", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r0.1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r1", VersionSpecOptions())));

            TEST_CHECK(! VersionSpec("1.2.3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r3", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r3", VersionSpecOptions()).tilde_compare(VersionSpec("1.2.3", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.2-r0.1", VersionSpecOptions()).tilde_compare(VersionSpec("1.2-r0.2", VersionSpecOptions())));
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
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).tilde_greater_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(VersionSpec("1.2.1", VersionSpecOptions()).tilde_greater_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("1.1", VersionSpecOptions()).tilde_greater_compare(VersionSpec("1.2", VersionSpecOptions())));
            TEST_CHECK(! VersionSpec("2.0", VersionSpecOptions()).tilde_greater_compare(VersionSpec("1.2", VersionSpecOptions())));
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
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2", VersionSpecOptions()).remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r", VersionSpecOptions()).remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99", VersionSpecOptions()).remove_revision(), "1.2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r3.4", VersionSpecOptions()).remove_revision(), "1.2");

            TEST_CHECK_EQUAL(VersionSpec("1.2", VersionSpecOptions()).remove_revision(), VersionSpec("1.2", VersionSpecOptions()));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r", VersionSpecOptions()).remove_revision(), VersionSpec("1.2", VersionSpecOptions()));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r99", VersionSpecOptions()).remove_revision(), VersionSpec("1.2", VersionSpecOptions()));
            TEST_CHECK_EQUAL(VersionSpec("1.2-r3.4", VersionSpecOptions()).remove_revision(), VersionSpec("1.2", VersionSpecOptions()));
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
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2", VersionSpecOptions()).bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99", VersionSpecOptions()).bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2.3", VersionSpecOptions()).bump(), "1.3");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1", VersionSpecOptions()).bump(), "2");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.99.0", VersionSpecOptions()).bump(), "1.100");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.099.0", VersionSpecOptions()).bump(), "1.100");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.0099.0", VersionSpecOptions()).bump(), "1.0100");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("scm", VersionSpecOptions()).bump(), "scm");
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
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2", VersionSpecOptions()).revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r", VersionSpecOptions()).revision_only(), "r0");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r99", VersionSpecOptions()).revision_only(), "r99");
            TEST_CHECK_STRINGIFY_EQUAL(VersionSpec("1.2-r3.4", VersionSpecOptions()).revision_only(), "r3.4");
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
            TEST_CHECK(! VersionSpec("1.2", VersionSpecOptions()).is_scm());
            TEST_CHECK(VersionSpec("1.2-scm-r99", VersionSpecOptions()).is_scm());

            TEST_CHECK(! VersionSpec("1.2-r9998", VersionSpecOptions()).is_scm());
            TEST_CHECK(VersionSpec("1.2-r9999", VersionSpecOptions()).is_scm());

            TEST_CHECK(! VersionSpec("9998", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("9999_alpha2", VersionSpecOptions()).is_scm());
            TEST_CHECK(VersionSpec("9999", VersionSpecOptions()).is_scm());
            TEST_CHECK(VersionSpec("9999-r4", VersionSpecOptions()).is_scm());

            TEST_CHECK(VersionSpec("99999999-r4", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("99999998-r4", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("999", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("1.9999", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("9999.1", VersionSpecOptions()).is_scm());
            TEST_CHECK(! VersionSpec("9999.9999", VersionSpecOptions()).is_scm());

        }
    } test_version_is_scm;

    /**
     * \test VersionSpec has_*
     *
     */
    struct VersionHasStuffTest : TestCase
    {
        VersionHasStuffTest() : TestCase("version spec has_*") {}

        void run()
        {
            TEST_CHECK(! VersionSpec("1.2", VersionSpecOptions()).has_scm_part());
            TEST_CHECK(VersionSpec("1.2-scm", VersionSpecOptions()).has_scm_part());
            TEST_CHECK(VersionSpec("1.2-scm-r99", VersionSpecOptions()).has_scm_part());
            TEST_CHECK(! VersionSpec("9999", VersionSpecOptions()).has_scm_part());
            TEST_CHECK(VersionSpec("scm", VersionSpecOptions()).has_scm_part());

            TEST_CHECK(! VersionSpec("1", VersionSpecOptions()).has_try_part());
            TEST_CHECK(VersionSpec("1-try2", VersionSpecOptions()).has_try_part());
            TEST_CHECK(VersionSpec("1.2-try3-r4", VersionSpecOptions()).has_try_part());

            TEST_CHECK(! VersionSpec("1.2", VersionSpecOptions()).has_local_revision());
            TEST_CHECK(! VersionSpec("1.2-r0", VersionSpecOptions()).has_local_revision());
            TEST_CHECK(! VersionSpec("1.2-r3", VersionSpecOptions()).has_local_revision());
            TEST_CHECK(VersionSpec("1.2-r3.0", VersionSpecOptions()).has_local_revision());
            TEST_CHECK(VersionSpec("1.2-r3.4", VersionSpecOptions()).has_local_revision());
            TEST_CHECK(VersionSpec("1.2-r3.4.5", VersionSpecOptions()).has_local_revision());
        }
    } test_version_has_stuff;

    struct VersionSpecHashTest : TestCase
    {
        VersionSpecHashTest() : TestCase("version spec hash()") { }

        void run()
        {
            TEST_CHECK(VersionSpec("0", VersionSpecOptions()).hash() != VersionSpec("0.0", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1", VersionSpecOptions()).hash() != VersionSpec("1.0", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.0", VersionSpecOptions()).hash() != VersionSpec("1", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.0_alpha", VersionSpecOptions()).hash() != VersionSpec("1_alpha", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1_alpha", VersionSpecOptions()).hash() != VersionSpec("1.0_alpha", VersionSpecOptions()).hash());
        }
    } test_version_spec_hash;

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
            TEST_CHECK(VersionSpec("1.0", VersionSpecOptions()) > VersionSpec("1", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1", VersionSpecOptions()) < VersionSpec("1.0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.0_alpha", VersionSpecOptions()) > VersionSpec("1_alpha", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.0_alpha", VersionSpecOptions()) > VersionSpec("1", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.0_alpha", VersionSpecOptions()) < VersionSpec("1.0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2.0.0_alpha7-r4", VersionSpecOptions()) > VersionSpec("1.2_alpha7-r4", VersionSpecOptions()));

            TEST_CHECK(VersionSpec("0001", VersionSpecOptions()) == VersionSpec("1", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("01", VersionSpecOptions()) == VersionSpec("001", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("0001.1", VersionSpecOptions()) == VersionSpec("1.1", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("01.01", VersionSpecOptions()) == VersionSpec("1.01", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.010", VersionSpecOptions()) == VersionSpec("1.01", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.00", VersionSpecOptions()) == VersionSpec("1.0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.0100", VersionSpecOptions()) == VersionSpec("1.010", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1", VersionSpecOptions()) == VersionSpec("1-r0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1-r00", VersionSpecOptions()) == VersionSpec("1-r0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()) == VersionSpec("1.2-r", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2-r3", VersionSpecOptions()) == VersionSpec("1.2-r3.0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()) == VersionSpec("1.2-r0.0", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()) != VersionSpec("1.2-r0.1", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1.2-r0.1", VersionSpecOptions()) != VersionSpec("1.2", VersionSpecOptions()));

            TEST_CHECK(VersionSpec("1_alpha_beta-scm", VersionSpecOptions()) == VersionSpec("1_alpha0_beta-scm", VersionSpecOptions()));
            TEST_CHECK(VersionSpec("1_alpha_beta000_rc3-scm", VersionSpecOptions()) == VersionSpec("1_alpha00_beta_rc3-scm", VersionSpecOptions()));

            TEST_CHECK(VersionSpec("0001", VersionSpecOptions()).hash() == VersionSpec("1", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("01", VersionSpecOptions()).hash() == VersionSpec("001", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("0001.1", VersionSpecOptions()).hash() == VersionSpec("1.1", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("01.01", VersionSpecOptions()).hash() == VersionSpec("1.01", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.010", VersionSpecOptions()).hash() == VersionSpec("1.01", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.00", VersionSpecOptions()).hash() == VersionSpec("1.0", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.0100", VersionSpecOptions()).hash() == VersionSpec("1.010", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1", VersionSpecOptions()).hash() == VersionSpec("1-r0", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).hash() == VersionSpec("1.2-r", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.2-r3", VersionSpecOptions()).hash() == VersionSpec("1.2-r3.0", VersionSpecOptions()).hash());
            TEST_CHECK(VersionSpec("1.2", VersionSpecOptions()).hash() == VersionSpec("1.2-r0.0", VersionSpecOptions()).hash());

            std::vector<VersionSpec> v;
            v.push_back(VersionSpec("1_alpha_alpha", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha1_alpha", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha1_beta_pre", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha1_beta", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10-r1.1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10-r1.2", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10-r2", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha10_p1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_alpha11", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta10", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta10-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta10_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta10_p1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_beta11", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre10", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre10-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre10_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre10_p1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_pre11", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc10", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc10-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc10_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc10_p1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_rc11", VersionSpecOptions()));
            v.push_back(VersionSpec("1", VersionSpecOptions()));
            v.push_back(VersionSpec("1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1-try2", VersionSpecOptions()));
            v.push_back(VersionSpec("1p", VersionSpecOptions()));
            v.push_back(VersionSpec("1.0", VersionSpecOptions()));
            v.push_back(VersionSpec("1.0a", VersionSpecOptions()));
            v.push_back(VersionSpec("1.0.0", VersionSpecOptions()));
            v.push_back(VersionSpec("1.001", VersionSpecOptions()));
            v.push_back(VersionSpec("1.01", VersionSpecOptions()));
            v.push_back(VersionSpec("1.0101", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1_alpha3", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1.1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1.1-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1.2", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.1-scm-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_alpha", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_alpha-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta_p0-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta_p1-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta_p-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta1_p-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10_p", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10_p1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10_p1-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10_p10", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta10-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta11", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta11-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_beta-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_p3_pre", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_p3", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_p3_p", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2_p3-try4", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("1.2-scm-r1", VersionSpecOptions()));
            v.push_back(VersionSpec("1-scm", VersionSpecOptions()));
            v.push_back(VersionSpec("2_alpha", VersionSpecOptions()));
            v.push_back(VersionSpec("09", VersionSpecOptions()));
            v.push_back(VersionSpec("10", VersionSpecOptions()));
            v.push_back(VersionSpec("100", VersionSpecOptions()));
            v.push_back(VersionSpec("scm", VersionSpecOptions()));
            v.push_back(VersionSpec("scm-r3", VersionSpecOptions()));
            v.push_back(VersionSpec("scm-r3.4", VersionSpecOptions()));

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
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash()));
                        TEST_CHECK(v1->hash() != v2->hash());
                        TEST_CHECK(v2->hash() != v1->hash());
                    }
                    else if (std::distance(v.begin(), v1) > std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v2 < *v1);
                        TEST_CHECK(*v1 > *v2);
                        TEST_CHECK(*v2 != *v1);
                        TEST_CHECK(*v1 != *v2);
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash()));
                        TEST_CHECK(v1->hash() != v2->hash());
                        TEST_CHECK(v2->hash() != v1->hash());
                    }
                    else
                    {
                        TEST_CHECK(*v2 == *v1);
                        TEST_CHECK(*v1 == *v2);
                        TestMessageSuffix sv1("hv1:" + stringify(v1->hash()));
                        TestMessageSuffix sv2("hv2:" + stringify(v2->hash()));
                        TEST_CHECK(v1->hash() == v2->hash());
                        TEST_CHECK(v2->hash() == v1->hash());
                    }
                }
            }
        }
    } test_version_spec_compare;

    struct VersionSpecComponentsTest : TestCase
    {
        VersionSpecComponentsTest() : TestCase("components") { }

        void run()
        {
            VersionSpec v1("1.2x_pre3_rc-scm", VersionSpecOptions());
            VersionSpec::ConstIterator i(v1.begin()), i_end(v1.end());

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_number);
            TEST_CHECK_EQUAL(i->number_value(), "1");
            TEST_CHECK_EQUAL(i->text(), "1");
            ++i;

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_number);
            TEST_CHECK_EQUAL(i->number_value(), "2");
            TEST_CHECK_EQUAL(i->text(), ".2");
            ++i;

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_letter);
            TEST_CHECK_EQUAL(i->number_value(), "x");
            TEST_CHECK_EQUAL(i->text(), "x");
            ++i;

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_pre);
            TEST_CHECK_EQUAL(i->number_value(), "3");
            TEST_CHECK_EQUAL(i->text(), "_pre3");
            ++i;

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_rc);
            TEST_CHECK_EQUAL(i->number_value(), "MAX");
            TEST_CHECK_EQUAL(i->text(), "_rc");
            ++i;

            TEST_CHECK(i != i_end);
            TEST_CHECK_EQUAL(i->type(), vsct_scm);
            TEST_CHECK_EQUAL(i->number_value(), "0");
            TEST_CHECK_EQUAL(i->text(), "-scm");
            ++i;
        }
    } test_version_spec_components;

    struct VersionSpecIgnoreCaseTest : TestCase
    {
        VersionSpecIgnoreCaseTest() : TestCase("ignore case") { }

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec("1.2A", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v1("1.2A", VersionSpecOptions() + vso_ignore_case);
            VersionSpec v2("1.2a", VersionSpecOptions());
            TEST_CHECK(v1 == v2);
            TEST_CHECK(v1.hash() == v2.hash());

            TEST_CHECK_THROWS(VersionSpec("1_ALPHA3", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v3("1_ALPHA3", VersionSpecOptions() + vso_ignore_case);
            VersionSpec v4("1_alpha3", VersionSpecOptions());
            TEST_CHECK(v3 == v4);
            TEST_CHECK(v3.hash() == v4.hash());

            TEST_CHECK_THROWS(VersionSpec("SCM", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v5("SCM", VersionSpecOptions() + vso_ignore_case);
            VersionSpec v6("scm", VersionSpecOptions());
            TEST_CHECK(v5 == v6);
            TEST_CHECK(v5.hash() == v6.hash());
        }
    } test_version_spec_ignore_case;

    struct VersionSpecFlexibleDashesTest : TestCase
    {
        VersionSpecFlexibleDashesTest() : TestCase("flexible dashes") { }

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec("1.2-alpha1", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v1("1.2-alpha1", VersionSpecOptions() + vso_flexible_dashes);
            VersionSpec v2("1.2_alpha1", VersionSpecOptions());
            TEST_CHECK(v1 == v2);
            TEST_CHECK(v1.hash() == v2.hash());

            TEST_CHECK_THROWS(VersionSpec("1_scm", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v3("1_scm", VersionSpecOptions() + vso_flexible_dashes);
            VersionSpec v4("1-scm", VersionSpecOptions());
            TEST_CHECK(v3 == v4);
            TEST_CHECK(v3.hash() == v4.hash());

            TEST_CHECK_THROWS(VersionSpec("1.2_r3", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v5("1.2_r3", VersionSpecOptions() + vso_flexible_dashes);
            VersionSpec v6("1.2-r3", VersionSpecOptions());
            TEST_CHECK(v5 == v6);
            TEST_CHECK(v5.hash() == v6.hash());

            TEST_CHECK_THROWS(VersionSpec("1.23alpha4rc5", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v7("1.23alpha4rc5", VersionSpecOptions() + vso_flexible_dashes);
            VersionSpec v8("1.23_alpha4_rc5", VersionSpecOptions());
            TEST_CHECK(v7 == v8);
            TEST_CHECK(v7.hash() == v8.hash());
        }
    } test_version_spec_flexible_dashes;

    struct VersionSpecFlexibleDotsTest : TestCase
    {
        VersionSpecFlexibleDotsTest() : TestCase("flexible dots") { }

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec("1.2-3_alpha4", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v1("1.2-3_alpha4", VersionSpecOptions() + vso_flexible_dots);
            VersionSpec v2("1.2.3_alpha4", VersionSpecOptions());
            TEST_CHECK(v1 == v2);
            TEST_CHECK(v1.hash() == v2.hash());

            TEST_CHECK_THROWS(VersionSpec("1_2-3-4.5", VersionSpecOptions() + vso_flexible_dots), BadVersionSpecError);
            VersionSpec v3("1_2-3-4.5", VersionSpecOptions() + vso_flexible_dots + vso_flexible_dashes);
            VersionSpec v4("1.2.3.4.5", VersionSpecOptions());
            TEST_CHECK(v3 == v4);
            TEST_CHECK(v3.hash() == v4.hash());

        }
    } test_version_spec_flexible_dots;

    struct VersionSpecLeadingVTest : TestCase
    {
        VersionSpecLeadingVTest() : TestCase("leading v") { }

        void run()
        {
            TEST_CHECK_THROWS(VersionSpec("v1.2.3", VersionSpecOptions()), BadVersionSpecError);
            VersionSpec v1("v1.2.3", VersionSpecOptions() + vso_ignore_leading_v);
            VersionSpec v2("1.2.3", VersionSpecOptions());
            TEST_CHECK(v1 == v2);
            TEST_CHECK(v1.hash() == v2.hash());
        }
    } test_version_spec_leading_v;
}

