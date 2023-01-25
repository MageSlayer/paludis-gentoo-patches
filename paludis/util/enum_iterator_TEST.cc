/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2011 Ciaran McCreesh
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

#include <paludis/util/enum_iterator.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    enum Numbers
    {
        one,
        two,
        three,
        last_number
    };
}

TEST(EnumIterator, Works)
{
    EnumIterator<Numbers> n;
    EnumIterator<Numbers> n_end(last_number);

    ASSERT_TRUE(n != n_end);
    EXPECT_EQ(one, *n);
    ++n;

    ASSERT_TRUE(n != n_end);
    EXPECT_EQ(two, *n);
    ++n;

    ASSERT_TRUE(n != n_end);
    EXPECT_EQ(three, *n);
    ++n;

    ASSERT_TRUE(n == n_end);
    EXPECT_EQ(last_number, *n);
}

