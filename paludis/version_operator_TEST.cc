/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2011 Ciaran McCreesh
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

#include <paludis/version_operator.hh>
#include <paludis/util/stringify.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(VersionOperator, Comparisons)
{
    VersionOperator v1(vo_greater);
    VersionOperator v2(vo_less_equal);
    EXPECT_TRUE(v1 != v2);
    EXPECT_TRUE(v1 == vo_greater);
    EXPECT_TRUE(v1 != vo_less_equal);
    EXPECT_TRUE(v2 == vo_less_equal);
    EXPECT_TRUE(v2 != vo_greater);

    VersionOperator v3(v1);
    EXPECT_TRUE(v1 == vo_greater);
    EXPECT_TRUE(v3 == vo_greater);
    EXPECT_TRUE(v1 == v3);

    v3 = v2;
    EXPECT_TRUE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
    EXPECT_TRUE(v2 == v3);
    EXPECT_TRUE(v1 == vo_greater);
    EXPECT_TRUE(v2 == vo_less_equal);
    EXPECT_TRUE(v3 == vo_less_equal);
}

TEST(VersionOperator, FromString)
{
    VersionOperator v1(">");
    VersionOperator v2("<=");
    EXPECT_TRUE(v1 != v2);
    EXPECT_TRUE(v1 == vo_greater);
    EXPECT_TRUE(v1 != vo_less_equal);
    EXPECT_TRUE(v2 == vo_less_equal);
    EXPECT_TRUE(v2 != vo_greater);

    EXPECT_EQ(VersionOperator("<"),  VersionOperator(vo_less));
    EXPECT_EQ(VersionOperator("<="), VersionOperator(vo_less_equal));
    EXPECT_EQ(VersionOperator("="),  VersionOperator(vo_equal));
    EXPECT_EQ(VersionOperator("~"),  VersionOperator(vo_tilde));
    EXPECT_EQ(VersionOperator(">"),  VersionOperator(vo_greater));
    EXPECT_EQ(VersionOperator(">="), VersionOperator(vo_greater_equal));
    EXPECT_EQ(VersionOperator("~>"), VersionOperator(vo_tilde_greater));
}

TEST(VersionOperator, Stringify)
{
    EXPECT_EQ(">", stringify(VersionOperator(vo_greater)));
    EXPECT_EQ("=", stringify(VersionOperator(vo_equal)));
    EXPECT_EQ("~", stringify(VersionOperator(vo_tilde)));
    EXPECT_EQ("~>", stringify(VersionOperator(vo_tilde_greater)));
}

