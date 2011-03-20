/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#include <paludis/util/tribool.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Tribool, DefaultCtor)
{
    Tribool b;
    EXPECT_TRUE(b.is_false());
    EXPECT_TRUE(! b.is_true());
    EXPECT_TRUE(! b.is_indeterminate());
}

TEST(Tribool, TrueCtor)
{
    Tribool b(true);
    EXPECT_TRUE(! b.is_false());
    EXPECT_TRUE(b.is_true());
    EXPECT_TRUE(! b.is_indeterminate());
}

TEST(Tribool, FalseCtor)
{
    Tribool f(false);
    EXPECT_TRUE(f.is_false());
    EXPECT_TRUE(! f.is_true());
    EXPECT_TRUE(! f.is_indeterminate());
}

TEST(Tribool, IndetCtor)
{
    Tribool b(indeterminate);
    EXPECT_TRUE(! b.is_false());
    EXPECT_TRUE(! b.is_true());
    EXPECT_TRUE(b.is_indeterminate());
}

