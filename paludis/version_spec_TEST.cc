/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>

#include <vector>
#include <iterator>

#include <gtest/gtest.h>

using namespace paludis;

TEST(VersionSpec, Construction)
{
    VersionSpec v("1", { });
    VersionSpec v1("1b", { });
    VersionSpec v2("1_alpha", { });
    VersionSpec v3("1_beta", { });
    VersionSpec v4("1_pre", { });
    VersionSpec v5("1_rc", { });
    VersionSpec v6("1_p", { });
    VersionSpec v7("1_alpha1", { });
    VersionSpec v8("1_beta1", { });
    VersionSpec v9("1_pre1", { });
    VersionSpec v10("1_rc1", { });
    VersionSpec v11("1_p1", { });
    VersionSpec v12("1_alpha-r1", { });
    VersionSpec v13("1_beta-r1", { });
    VersionSpec v14("1_pre-r1", { });
    VersionSpec v15("1_rc-r1", { });
    VersionSpec v16("1_p-r1", { });
    VersionSpec v17("1_alpha1-r1", { });
    VersionSpec v18("1_beta1-r1", { });
    VersionSpec v19("1_pre1-r1", { });
    VersionSpec v20("1_pre1-r1.2", { });
    VersionSpec v21("1_rc1-r1", { });
    VersionSpec v22("1_p1-r1", { });
    VersionSpec v23("1_alpha_p", { });
    VersionSpec v24("1_p3_alpha", { });
    VersionSpec v25("1_p4_p-r2", { });
    VersionSpec v26("scm", { });

    SUCCEED();
}

TEST(VersionSpec, BadConstruction)
{
    ASSERT_THROW(VersionSpec v1("", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("b", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-r1_pre", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-pre", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1_blah", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1_pre-r2b", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1_pre-r2-r2", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-try-try", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-try_alpha", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-scm-scm", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-scm-try", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-scm_alpha", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-r2_pre", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1.", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1.1.", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1.-r", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1.-r1", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-r.0", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1-r1.", { }), BadVersionSpecError);
    ASSERT_THROW(VersionSpec v1("1_p1.", { }), BadVersionSpecError);
}

TEST(VersionSpec, TrickyParses)
{
    VersionSpec v("1.2.3", { });
    ASSERT_TRUE(v == VersionSpec("1.2.3", { }));
    VersionSpec v1("1.2_pre2-r1", { });
    ASSERT_TRUE(v1 == VersionSpec("1.2_pre2-r1", { }));
    VersionSpec v2("1.2_pre2_rc5_p6-r1", { });
    ASSERT_TRUE(v2 == VersionSpec("1.2_pre2_rc5_p6-r1", { }));
    VersionSpec v3("1.2_pre2_pre3_pre4", { });
    ASSERT_TRUE(v3 == VersionSpec("1.2_pre2_pre3_pre4", { }));
}

TEST(VersionSpec, StupidStar)
{
    ASSERT_TRUE(VersionSpec("1.2", { }).stupid_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(VersionSpec("1.2", { }).stupid_equal_star_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2.1", { }).stupid_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(VersionSpec("1.2.1", { }).stupid_equal_star_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("2.2", { }).stupid_equal_star_compare(VersionSpec("2", { })));
    ASSERT_TRUE(VersionSpec("2", { }).stupid_equal_star_compare(VersionSpec("2", { })));
    ASSERT_TRUE(VersionSpec("2.59", { }).stupid_equal_star_compare(VersionSpec("2.5", { })));
    ASSERT_TRUE(VersionSpec("2.59_alpha5-r1", { }).stupid_equal_star_compare(VersionSpec("2.59_alpha", { })));
    ASSERT_TRUE(! VersionSpec("2", { }).stupid_equal_star_compare(VersionSpec("2.5", { })));
    ASSERT_TRUE(! VersionSpec("2.59", { }).stupid_equal_star_compare(VersionSpec("2.50", { })));
    ASSERT_TRUE(! VersionSpec("1", { }).stupid_equal_star_compare(VersionSpec("2", { })));

    ASSERT_TRUE(! VersionSpec("01", { }).stupid_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(! VersionSpec("1.02", { }).stupid_equal_star_compare(VersionSpec("1.020", { })));
    ASSERT_TRUE(VersionSpec("1.020", { }).stupid_equal_star_compare(VersionSpec("1.02", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha1", { }).stupid_equal_star_compare(VersionSpec("1_alpha01", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha01", { }).stupid_equal_star_compare(VersionSpec("1_alpha1", { })));
    ASSERT_TRUE(VersionSpec("1_alpha01", { }).stupid_equal_star_compare(VersionSpec("1_alpha0", { })));
    ASSERT_TRUE(VersionSpec("1_pre1", { }).stupid_equal_star_compare(VersionSpec("1_p", { })));
    ASSERT_TRUE(VersionSpec("1_pre-scm", { }).stupid_equal_star_compare(VersionSpec("1_pre", { })));
    ASSERT_TRUE(! VersionSpec("1_pre1", { }).stupid_equal_star_compare(VersionSpec("1_pre0", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha1", { }).stupid_equal_star_compare(VersionSpec("1_alpha-r1", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha1", { }).stupid_equal_star_compare(VersionSpec("1_beta", { })));

    ASSERT_TRUE(VersionSpec("010", { }).stupid_equal_star_compare(VersionSpec("010", { })));
    ASSERT_TRUE(VersionSpec("010", { }).stupid_equal_star_compare(VersionSpec("01", { })));
    ASSERT_TRUE(VersionSpec("2.010", { }).stupid_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(VersionSpec("2.0105", { }).stupid_equal_star_compare(VersionSpec("2.010", { })));
    ASSERT_TRUE(! VersionSpec("2.0135", { }).stupid_equal_star_compare(VersionSpec("2.010", { })));
    ASSERT_TRUE(VersionSpec("2.010.1", { }).stupid_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(VersionSpec("2.011.1", { }).stupid_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(! VersionSpec("2.010.1", { }).stupid_equal_star_compare(VersionSpec("2.01.1", { })));
    ASSERT_TRUE(! VersionSpec("2.011.1", { }).stupid_equal_star_compare(VersionSpec("2.01.1", { })));
    ASSERT_TRUE(VersionSpec("2.10", { }).stupid_equal_star_compare(VersionSpec("2.1", { })));
}

TEST(VersionSpec, NiceStar)
{
    ASSERT_TRUE(VersionSpec("1.2", { }).nice_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(VersionSpec("1.2", { }).nice_equal_star_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2.1", { }).nice_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(VersionSpec("1.2.1", { }).nice_equal_star_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("2.2", { }).nice_equal_star_compare(VersionSpec("2", { })));
    ASSERT_TRUE(VersionSpec("2", { }).nice_equal_star_compare(VersionSpec("2", { })));
    ASSERT_TRUE(! VersionSpec("2.59", { }).nice_equal_star_compare(VersionSpec("2.5", { })));
    ASSERT_TRUE(VersionSpec("2.59_alpha5-r1", { }).nice_equal_star_compare(VersionSpec("2.59_alpha", { })));
    ASSERT_TRUE(! VersionSpec("2", { }).nice_equal_star_compare(VersionSpec("2.5", { })));
    ASSERT_TRUE(! VersionSpec("2.59", { }).nice_equal_star_compare(VersionSpec("2.50", { })));
    ASSERT_TRUE(! VersionSpec("1", { }).nice_equal_star_compare(VersionSpec("2", { })));

    ASSERT_TRUE(VersionSpec("01", { }).nice_equal_star_compare(VersionSpec("1", { })));
    ASSERT_TRUE(VersionSpec("1.02", { }).nice_equal_star_compare(VersionSpec("1.020", { })));
    ASSERT_TRUE(VersionSpec("1.020", { }).nice_equal_star_compare(VersionSpec("1.02", { })));
    ASSERT_TRUE(VersionSpec("1_alpha1", { }).nice_equal_star_compare(VersionSpec("1_alpha01", { })));
    ASSERT_TRUE(VersionSpec("1_alpha01", { }).nice_equal_star_compare(VersionSpec("1_alpha1", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha01", { }).nice_equal_star_compare(VersionSpec("1_alpha0", { })));
    ASSERT_TRUE(! VersionSpec("1_pre1", { }).nice_equal_star_compare(VersionSpec("1_p", { })));
    ASSERT_TRUE(VersionSpec("1_pre-scm", { }).nice_equal_star_compare(VersionSpec("1_pre", { })));
    ASSERT_TRUE(! VersionSpec("1_pre1", { }).nice_equal_star_compare(VersionSpec("1_pre0", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha1", { }).nice_equal_star_compare(VersionSpec("1_alpha-r1", { })));
    ASSERT_TRUE(! VersionSpec("1_alpha1", { }).nice_equal_star_compare(VersionSpec("1_beta", { })));

    ASSERT_TRUE(VersionSpec("010", { }).nice_equal_star_compare(VersionSpec("010", { })));
    ASSERT_TRUE(! VersionSpec("010", { }).nice_equal_star_compare(VersionSpec("01", { })));
    ASSERT_TRUE(VersionSpec("2.010", { }).nice_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(VersionSpec("2.0105", { }).nice_equal_star_compare(VersionSpec("2.010", { })));
    ASSERT_TRUE(! VersionSpec("2.0135", { }).nice_equal_star_compare(VersionSpec("2.010", { })));
    ASSERT_TRUE(VersionSpec("2.010.1", { }).nice_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(VersionSpec("2.011.1", { }).nice_equal_star_compare(VersionSpec("2.01", { })));
    ASSERT_TRUE(VersionSpec("2.010.1", { }).nice_equal_star_compare(VersionSpec("2.01.1", { })));
    ASSERT_TRUE(! VersionSpec("2.011.1", { }).stupid_equal_star_compare(VersionSpec("2.01.1", { })));
    ASSERT_TRUE(! VersionSpec("2.10", { }).nice_equal_star_compare(VersionSpec("2.1", { })));
}

TEST(VersionSpec, Tilde)
{
    ASSERT_TRUE(! VersionSpec("1.4-r1", { }).tilde_compare(VersionSpec("1.3-r1", { })));
    ASSERT_TRUE(! VersionSpec("1.4", { }).tilde_compare(VersionSpec("1.3-r1", { })));
    ASSERT_TRUE(! VersionSpec("1.2", { }).tilde_compare(VersionSpec("1.3-r1", { })));
    ASSERT_TRUE(! VersionSpec("1.3", { }).tilde_compare(VersionSpec("1.3-r1", { })));

    ASSERT_TRUE(VersionSpec("1.2", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2-r1", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2-r1.2.3", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(! VersionSpec("1.3", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2-r2", { }).tilde_compare(VersionSpec("1.2-r1", { })));
    ASSERT_TRUE(VersionSpec("1.2-r2.3", { }).tilde_compare(VersionSpec("1.2-r1", { })));
    ASSERT_TRUE(VersionSpec("1.2-r2", { }).tilde_compare(VersionSpec("1.2-r2", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r1", { }).tilde_compare(VersionSpec("1.2-r2", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r1.3", { }).tilde_compare(VersionSpec("1.2-r2", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r2", { }).tilde_compare(VersionSpec("1.2-r2.3", { })));
    ASSERT_TRUE(VersionSpec("1.2-r2.4", { }).tilde_compare(VersionSpec("1.2-r2.3", { })));

    ASSERT_TRUE(VersionSpec("1.2-r0", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2", { }).tilde_compare(VersionSpec("1.2-r0", { })));
    ASSERT_TRUE(VersionSpec("1.2-r1", { }).tilde_compare(VersionSpec("1.2-r0", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r0", { }).tilde_compare(VersionSpec("1.2-r1", { })));
    ASSERT_TRUE(VersionSpec("1.2-r0.0", { }).tilde_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2", { }).tilde_compare(VersionSpec("1.2-r0.0", { })));
    ASSERT_TRUE(VersionSpec("1.2-r0.0", { }).tilde_compare(VersionSpec("1.2-r0", { })));
    ASSERT_TRUE(VersionSpec("1.2-r0", { }).tilde_compare(VersionSpec("1.2-r0.0", { })));
    ASSERT_TRUE(VersionSpec("1.2-r0.1", { }).tilde_compare(VersionSpec("1.2-r0", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r0", { }).tilde_compare(VersionSpec("1.2-r0.1", { })));
    ASSERT_TRUE(VersionSpec("1.2-r1", { }).tilde_compare(VersionSpec("1.2-r0.1", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r0.1", { }).tilde_compare(VersionSpec("1.2-r1", { })));

    ASSERT_TRUE(! VersionSpec("1.2.3", { }).tilde_compare(VersionSpec("1.2-r3", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r3", { }).tilde_compare(VersionSpec("1.2.3", { })));
    ASSERT_TRUE(! VersionSpec("1.2", { }).tilde_compare(VersionSpec("1.2-r0.2", { })));
    ASSERT_TRUE(! VersionSpec("1.2-r0.1", { }).tilde_compare(VersionSpec("1.2-r0.2", { })));
}

TEST(VersionSpec, TildeGreater)
{
    ASSERT_TRUE(VersionSpec("1.2", { }).tilde_greater_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(VersionSpec("1.2.1", { }).tilde_greater_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(! VersionSpec("1.1", { }).tilde_greater_compare(VersionSpec("1.2", { })));
    ASSERT_TRUE(! VersionSpec("2.0", { }).tilde_greater_compare(VersionSpec("1.2", { })));
}


TEST(VersionSpec, RemoveRevision)
{
    EXPECT_EQ("1.2", stringify(VersionSpec("1.2", { }).remove_revision()));
    EXPECT_EQ("1.2", stringify(VersionSpec("1.2-r", { }).remove_revision()));
    EXPECT_EQ("1.2", stringify(VersionSpec("1.2-r99", { }).remove_revision()));
    EXPECT_EQ("1.2", stringify(VersionSpec("1.2-r3.4", { }).remove_revision()));

    EXPECT_EQ(VersionSpec("1.2", { }).remove_revision(), VersionSpec("1.2", { }));
    EXPECT_EQ(VersionSpec("1.2-r", { }).remove_revision(), VersionSpec("1.2", { }));
    EXPECT_EQ(VersionSpec("1.2-r99", { }).remove_revision(), VersionSpec("1.2", { }));
    EXPECT_EQ(VersionSpec("1.2-r3.4", { }).remove_revision(), VersionSpec("1.2", { }));
}

TEST(VersionSpec, Bump)
{
    EXPECT_EQ("2", stringify(VersionSpec("1.2", { }).bump()));
    EXPECT_EQ("2", stringify(VersionSpec("1.2-r99", { }).bump()));
    EXPECT_EQ("1.3", stringify(VersionSpec("1.2.3", { }).bump()));
    EXPECT_EQ("2", stringify(VersionSpec("1", { }).bump()));
    EXPECT_EQ("1.100", stringify(VersionSpec("1.99.0", { }).bump()));
    EXPECT_EQ("1.100", stringify(VersionSpec("1.099.0", { }).bump()));
    EXPECT_EQ("1.0100", stringify(VersionSpec("1.0099.0", { }).bump()));
    EXPECT_EQ("scm", stringify(VersionSpec("scm", { }).bump()));
}

TEST(VersionSpec, RevisionOnly)
{
    EXPECT_EQ("r0", stringify(VersionSpec("1.2", { }).revision_only()));
    EXPECT_EQ("r0", stringify(VersionSpec("1.2-r", { }).revision_only()));
    EXPECT_EQ("r99", stringify(VersionSpec("1.2-r99", { }).revision_only()));
    EXPECT_EQ("r3.4", stringify(VersionSpec("1.2-r3.4", { }).revision_only()));
}

TEST(VersionSpec, IsScm)
{
    ASSERT_TRUE(! VersionSpec("1.2", { }).is_scm());
    ASSERT_TRUE(VersionSpec("1.2-scm-r99", { }).is_scm());

    ASSERT_TRUE(! VersionSpec("1.2-r9998", { }).is_scm());
    ASSERT_TRUE(VersionSpec("1.2-r9999", { }).is_scm());

    ASSERT_TRUE(! VersionSpec("9998", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("9999_alpha2", { }).is_scm());
    ASSERT_TRUE(VersionSpec("9999", { }).is_scm());
    ASSERT_TRUE(VersionSpec("9999-r4", { }).is_scm());

    ASSERT_TRUE(VersionSpec("99999999-r4", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("99999998-r4", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("999", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("1.9999", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("9999.1", { }).is_scm());
    ASSERT_TRUE(! VersionSpec("9999.9999", { }).is_scm());

}

TEST(VersionSpec, Has)
{
    ASSERT_TRUE(! VersionSpec("1.2", { }).has_scm_part());
    ASSERT_TRUE(VersionSpec("1.2-scm", { }).has_scm_part());
    ASSERT_TRUE(VersionSpec("1.2-scm-r99", { }).has_scm_part());
    ASSERT_TRUE(! VersionSpec("9999", { }).has_scm_part());
    ASSERT_TRUE(VersionSpec("scm", { }).has_scm_part());

    ASSERT_TRUE(! VersionSpec("1", { }).has_try_part());
    ASSERT_TRUE(VersionSpec("1-try2", { }).has_try_part());
    ASSERT_TRUE(VersionSpec("1.2-try3-r4", { }).has_try_part());

    ASSERT_TRUE(! VersionSpec("1.2", { }).has_local_revision());
    ASSERT_TRUE(! VersionSpec("1.2-r0", { }).has_local_revision());
    ASSERT_TRUE(! VersionSpec("1.2-r3", { }).has_local_revision());
    ASSERT_TRUE(VersionSpec("1.2-r3.0", { }).has_local_revision());
    ASSERT_TRUE(VersionSpec("1.2-r3.4", { }).has_local_revision());
    ASSERT_TRUE(VersionSpec("1.2-r3.4.5", { }).has_local_revision());
}

TEST(VersionSpec, Hash)
{
    ASSERT_TRUE(VersionSpec("0", { }).hash() != VersionSpec("0.0", { }).hash());
    ASSERT_TRUE(VersionSpec("1", { }).hash() != VersionSpec("1.0", { }).hash());
    ASSERT_TRUE(VersionSpec("1.0", { }).hash() != VersionSpec("1", { }).hash());
    ASSERT_TRUE(VersionSpec("1.0_alpha", { }).hash() != VersionSpec("1_alpha", { }).hash());
    ASSERT_TRUE(VersionSpec("1_alpha", { }).hash() != VersionSpec("1.0_alpha", { }).hash());
}

TEST(VersionSpec, Ordering)
{
    ASSERT_TRUE(VersionSpec("1.0", { }) > VersionSpec("1", { }));
    ASSERT_TRUE(VersionSpec("1", { }) < VersionSpec("1.0", { }));
    ASSERT_TRUE(VersionSpec("1.0_alpha", { }) > VersionSpec("1_alpha", { }));
    ASSERT_TRUE(VersionSpec("1.0_alpha", { }) > VersionSpec("1", { }));
    ASSERT_TRUE(VersionSpec("1.0_alpha", { }) < VersionSpec("1.0", { }));
    ASSERT_TRUE(VersionSpec("1.2.0.0_alpha7-r4", { }) > VersionSpec("1.2_alpha7-r4", { }));

    ASSERT_TRUE(VersionSpec("0001", { }) == VersionSpec("1", { }));
    ASSERT_TRUE(VersionSpec("01", { }) == VersionSpec("001", { }));
    ASSERT_TRUE(VersionSpec("0001.1", { }) == VersionSpec("1.1", { }));
    ASSERT_TRUE(VersionSpec("01.01", { }) == VersionSpec("1.01", { }));
    ASSERT_TRUE(VersionSpec("1.010", { }) == VersionSpec("1.01", { }));
    ASSERT_TRUE(VersionSpec("1.00", { }) == VersionSpec("1.0", { }));
    ASSERT_TRUE(VersionSpec("1.0100", { }) == VersionSpec("1.010", { }));
    ASSERT_TRUE(VersionSpec("1", { }) == VersionSpec("1-r0", { }));
    ASSERT_TRUE(VersionSpec("1-r00", { }) == VersionSpec("1-r0", { }));
    ASSERT_TRUE(VersionSpec("1.2", { }) == VersionSpec("1.2-r", { }));
    ASSERT_TRUE(VersionSpec("1.2-r3", { }) == VersionSpec("1.2-r3.0", { }));
    ASSERT_TRUE(VersionSpec("1.2", { }) == VersionSpec("1.2-r0.0", { }));
    ASSERT_TRUE(VersionSpec("1.2", { }) != VersionSpec("1.2-r0.1", { }));
    ASSERT_TRUE(VersionSpec("1.2-r0.1", { }) != VersionSpec("1.2", { }));

    ASSERT_TRUE(VersionSpec("1_alpha_beta-scm", { }) == VersionSpec("1_alpha0_beta-scm", { }));
    ASSERT_TRUE(VersionSpec("1_alpha_beta000_rc3-scm", { }) == VersionSpec("1_alpha00_beta_rc3-scm", { }));

    ASSERT_TRUE(VersionSpec("0001", { }).hash() == VersionSpec("1", { }).hash());
    ASSERT_TRUE(VersionSpec("01", { }).hash() == VersionSpec("001", { }).hash());
    ASSERT_TRUE(VersionSpec("0001.1", { }).hash() == VersionSpec("1.1", { }).hash());
    ASSERT_TRUE(VersionSpec("01.01", { }).hash() == VersionSpec("1.01", { }).hash());
    ASSERT_TRUE(VersionSpec("1.010", { }).hash() == VersionSpec("1.01", { }).hash());
    ASSERT_TRUE(VersionSpec("1.00", { }).hash() == VersionSpec("1.0", { }).hash());
    ASSERT_TRUE(VersionSpec("1.0100", { }).hash() == VersionSpec("1.010", { }).hash());
    ASSERT_TRUE(VersionSpec("1", { }).hash() == VersionSpec("1-r0", { }).hash());
    ASSERT_TRUE(VersionSpec("1.2", { }).hash() == VersionSpec("1.2-r", { }).hash());
    ASSERT_TRUE(VersionSpec("1.2-r3", { }).hash() == VersionSpec("1.2-r3.0", { }).hash());
    ASSERT_TRUE(VersionSpec("1.2", { }).hash() == VersionSpec("1.2-r0.0", { }).hash());

    std::vector<VersionSpec> v;
    v.push_back(VersionSpec("1_alpha_alpha", { }));
    v.push_back(VersionSpec("1_alpha", { }));
    v.push_back(VersionSpec("1_alpha1_alpha", { }));
    v.push_back(VersionSpec("1_alpha1_beta_pre", { }));
    v.push_back(VersionSpec("1_alpha1_beta", { }));
    v.push_back(VersionSpec("1_alpha1", { }));
    v.push_back(VersionSpec("1_alpha1-r1", { }));
    v.push_back(VersionSpec("1_alpha10", { }));
    v.push_back(VersionSpec("1_alpha10-r1", { }));
    v.push_back(VersionSpec("1_alpha10-r1.1", { }));
    v.push_back(VersionSpec("1_alpha10-r1.2", { }));
    v.push_back(VersionSpec("1_alpha10-r2", { }));
    v.push_back(VersionSpec("1_alpha10_p1", { }));
    v.push_back(VersionSpec("1_alpha10_p1-r1", { }));
    v.push_back(VersionSpec("1_alpha11", { }));
    v.push_back(VersionSpec("1_beta", { }));
    v.push_back(VersionSpec("1_beta10", { }));
    v.push_back(VersionSpec("1_beta10-r1", { }));
    v.push_back(VersionSpec("1_beta10_p1", { }));
    v.push_back(VersionSpec("1_beta10_p1-r1", { }));
    v.push_back(VersionSpec("1_beta11", { }));
    v.push_back(VersionSpec("1_pre", { }));
    v.push_back(VersionSpec("1_pre10", { }));
    v.push_back(VersionSpec("1_pre10-r1", { }));
    v.push_back(VersionSpec("1_pre10_p1", { }));
    v.push_back(VersionSpec("1_pre10_p1-r1", { }));
    v.push_back(VersionSpec("1_pre11", { }));
    v.push_back(VersionSpec("1_rc", { }));
    v.push_back(VersionSpec("1_rc10", { }));
    v.push_back(VersionSpec("1_rc10-r1", { }));
    v.push_back(VersionSpec("1_rc10_p1", { }));
    v.push_back(VersionSpec("1_rc10_p1-r1", { }));
    v.push_back(VersionSpec("1_rc11", { }));
    v.push_back(VersionSpec("1", { }));
    v.push_back(VersionSpec("1-r1", { }));
    v.push_back(VersionSpec("1_p1", { }));
    v.push_back(VersionSpec("1-try2", { }));
    v.push_back(VersionSpec("1p", { }));
    v.push_back(VersionSpec("1.0", { }));
    v.push_back(VersionSpec("1.0a", { }));
    v.push_back(VersionSpec("1.0.0", { }));
    v.push_back(VersionSpec("1.001", { }));
    v.push_back(VersionSpec("1.01", { }));
    v.push_back(VersionSpec("1.0101", { }));
    v.push_back(VersionSpec("1.1_alpha3", { }));
    v.push_back(VersionSpec("1.1", { }));
    v.push_back(VersionSpec("1.1-r1", { }));
    v.push_back(VersionSpec("1.1.1", { }));
    v.push_back(VersionSpec("1.1.1-scm", { }));
    v.push_back(VersionSpec("1.1.2", { }));
    v.push_back(VersionSpec("1.1-scm", { }));
    v.push_back(VersionSpec("1.1-scm-r1", { }));
    v.push_back(VersionSpec("1.2_alpha", { }));
    v.push_back(VersionSpec("1.2_alpha-scm", { }));
    v.push_back(VersionSpec("1.2_beta", { }));
    v.push_back(VersionSpec("1.2_beta_p0-scm", { }));
    v.push_back(VersionSpec("1.2_beta_p1-scm", { }));
    v.push_back(VersionSpec("1.2_beta_p-scm", { }));
    v.push_back(VersionSpec("1.2_beta1_p-scm", { }));
    v.push_back(VersionSpec("1.2_beta10", { }));
    v.push_back(VersionSpec("1.2_beta10_p", { }));
    v.push_back(VersionSpec("1.2_beta10_p1", { }));
    v.push_back(VersionSpec("1.2_beta10_p1-scm", { }));
    v.push_back(VersionSpec("1.2_beta10_p10", { }));
    v.push_back(VersionSpec("1.2_beta10-scm", { }));
    v.push_back(VersionSpec("1.2_beta11", { }));
    v.push_back(VersionSpec("1.2_beta11-scm", { }));
    v.push_back(VersionSpec("1.2_beta-scm", { }));
    v.push_back(VersionSpec("1.2", { }));
    v.push_back(VersionSpec("1.2-r1", { }));
    v.push_back(VersionSpec("1.2_p3_pre", { }));
    v.push_back(VersionSpec("1.2_p3", { }));
    v.push_back(VersionSpec("1.2_p3_p", { }));
    v.push_back(VersionSpec("1.2_p3-try4", { }));
    v.push_back(VersionSpec("1.2-scm", { }));
    v.push_back(VersionSpec("1.2-scm-r1", { }));
    v.push_back(VersionSpec("1-scm", { }));
    v.push_back(VersionSpec("2_alpha", { }));
    v.push_back(VersionSpec("09", { }));
    v.push_back(VersionSpec("10", { }));
    v.push_back(VersionSpec("100", { }));
    v.push_back(VersionSpec("scm", { }));
    v.push_back(VersionSpec("scm-r3", { }));
    v.push_back(VersionSpec("scm-r3.4", { }));

    std::vector<VersionSpec>::iterator v1(v.begin()), v_end(v.end());
    for ( ; v1 != v_end ; ++v1)
    {
        std::vector<VersionSpec>::iterator v2(v.begin());
        for ( ; v2 != v_end ; ++v2)
        {
            if (std::distance(v.begin(), v1) < std::distance(v.begin(), v2))
            {
                ASSERT_LT(*v1, *v2);
                ASSERT_GT(*v2, *v1);
                ASSERT_NE(*v1, *v2);
                ASSERT_NE(*v2, *v1);
                ASSERT_NE(v1->hash(), v2->hash());
                ASSERT_NE(v2->hash(), v1->hash());
            }
            else if (std::distance(v.begin(), v1) > std::distance(v.begin(), v2))
            {
                ASSERT_LT(*v2, *v1);
                ASSERT_GT(*v1, *v2);
                ASSERT_NE(*v2, *v1);
                ASSERT_NE(*v1, *v2);
                ASSERT_NE(v1->hash(), v2->hash());
                ASSERT_NE(v2->hash(), v1->hash());
            }
            else
            {
                ASSERT_EQ(*v2, *v1);
                ASSERT_EQ(*v1, *v2);
                ASSERT_EQ(v1->hash(), v2->hash());
                ASSERT_EQ(v2->hash(), v1->hash());
            }
        }
    }
}

TEST(VersionSpec, Components)
{
    VersionSpec v1("1.2x_pre3_rc-scm", { });
    VersionSpec::ConstIterator i(v1.begin()), i_end(v1.end());

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_number, i->type());
    EXPECT_EQ("1", i->number_value());
    EXPECT_EQ("1", i->text());
    ++i;

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_number, i->type());
    EXPECT_EQ("2", i->number_value());
    EXPECT_EQ(".2", i->text());
    ++i;

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_letter, i->type());
    EXPECT_EQ("x", i->number_value());
    EXPECT_EQ("x", i->text());
    ++i;

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_pre, i->type());
    EXPECT_EQ("3", i->number_value());
    EXPECT_EQ("_pre3", i->text());
    ++i;

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_rc, i->type());
    EXPECT_EQ("MAX", i->number_value());
    EXPECT_EQ("_rc", i->text());
    ++i;

    ASSERT_TRUE(i != i_end);
    EXPECT_EQ(vsct_scm, i->type());
    EXPECT_EQ("0", i->number_value());
    EXPECT_EQ("-scm", i->text());
    ++i;
}

TEST(VersionSpec, IgnoreCase)
{
    ASSERT_THROW(VersionSpec("1.2A", { }), BadVersionSpecError);
    VersionSpec v1("1.2A", { vso_ignore_case });
    VersionSpec v2("1.2a", { });
    ASSERT_TRUE(v1 == v2);
    ASSERT_TRUE(v1.hash() == v2.hash());

    ASSERT_THROW(VersionSpec("1_ALPHA3", { }), BadVersionSpecError);
    VersionSpec v3("1_ALPHA3", { vso_ignore_case });
    VersionSpec v4("1_alpha3", { });
    ASSERT_TRUE(v3 == v4);
    ASSERT_TRUE(v3.hash() == v4.hash());

    ASSERT_THROW(VersionSpec("SCM", { }), BadVersionSpecError);
    VersionSpec v5("SCM", { vso_ignore_case });
    VersionSpec v6("scm", { });
    ASSERT_TRUE(v5 == v6);
    ASSERT_TRUE(v5.hash() == v6.hash());
}

TEST(VersionSpec, FlexibleDash)
{
    ASSERT_THROW(VersionSpec("1.2-alpha1", { }), BadVersionSpecError);
    VersionSpec v1("1.2-alpha1", { vso_flexible_dashes });
    VersionSpec v2("1.2_alpha1", { });
    ASSERT_TRUE(v1 == v2);
    ASSERT_TRUE(v1.hash() == v2.hash());

    ASSERT_THROW(VersionSpec("1_scm", { }), BadVersionSpecError);
    VersionSpec v3("1_scm", { vso_flexible_dashes });
    VersionSpec v4("1-scm", { });
    ASSERT_TRUE(v3 == v4);
    ASSERT_TRUE(v3.hash() == v4.hash());

    ASSERT_THROW(VersionSpec("1.2_r3", { }), BadVersionSpecError);
    VersionSpec v5("1.2_r3", { vso_flexible_dashes });
    VersionSpec v6("1.2-r3", { });
    ASSERT_TRUE(v5 == v6);
    ASSERT_TRUE(v5.hash() == v6.hash());

    ASSERT_THROW(VersionSpec("1.23alpha4rc5", { }), BadVersionSpecError);
    VersionSpec v7("1.23alpha4rc5", { vso_flexible_dashes });
    VersionSpec v8("1.23_alpha4_rc5", { });
    ASSERT_TRUE(v7 == v8);
    ASSERT_TRUE(v7.hash() == v8.hash());
}

TEST(VersionSpec, FlexibleDots)
{
    ASSERT_THROW(VersionSpec("1.2-3_alpha4", { }), BadVersionSpecError);
    VersionSpec v1("1.2-3_alpha4", { vso_flexible_dots });
    VersionSpec v2("1.2.3_alpha4", { });
    ASSERT_TRUE(v1 == v2);
    ASSERT_TRUE(v1.hash() == v2.hash());

    ASSERT_THROW(VersionSpec("1_2-3-4.5", { vso_flexible_dots }), BadVersionSpecError);
    VersionSpec v3("1_2-3-4.5", { vso_flexible_dots, vso_flexible_dashes });
    VersionSpec v4("1.2.3.4.5", { });
    ASSERT_TRUE(v3 == v4);
    ASSERT_TRUE(v3.hash() == v4.hash());

}

TEST(VersionSpec, LeadingV)
{
    ASSERT_THROW(VersionSpec("v1.2.3", { }), BadVersionSpecError);
    VersionSpec v1("v1.2.3", { vso_ignore_leading_v });
    VersionSpec v2("1.2.3", { });
    VersionSpec v3("v.1.2.3", { vso_ignore_leading_v, vso_allow_leading_dot });
    ASSERT_TRUE(v1 == v2);
    ASSERT_TRUE(v1.hash() == v2.hash());
    ASSERT_TRUE(v2 == v3);
    ASSERT_TRUE(v2.hash() == v3.hash());
}

