/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/options.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    enum MyOption { mo_one, mo_two, mo_three, mo_four, mo_five, mo_six, mo_seven, mo_eight, mo_nine, mo_ten, last_mo };
    typedef Options<MyOption> MyOptions;
}

TEST(Options, Works)
{
    MyOptions options, second;

    EXPECT_TRUE(! options.any());

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
        EXPECT_TRUE(! options[o]);

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
        options = options + o;

    EXPECT_TRUE(options.any());

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
        EXPECT_TRUE(options[o]);

    for (MyOption o(static_cast<MyOption>(1)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
        EXPECT_TRUE(! options[o]);

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
        second += o;

    options |= second;

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
        EXPECT_TRUE(options[o]);

    for (MyOption o(static_cast<MyOption>(1)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
        options -= o;

    EXPECT_TRUE(options.any());

    for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
        options = options - o;

    EXPECT_TRUE(! options.any());

    second = { mo_three, mo_eight };
    options |= second;
    options += mo_seven;
    EXPECT_TRUE(options.any());
    options.subtract(second);
    EXPECT_TRUE(options.any());
    options -= mo_seven;
    EXPECT_TRUE(! options.any());
    EXPECT_TRUE(options.highest_bit() >= static_cast<int>(last_mo));
}

TEST(Options, Initialiser)
{
    MyOptions a({});
    EXPECT_TRUE(! a.any());

    MyOptions b({ mo_two, mo_four, mo_six });
    EXPECT_TRUE(! b[mo_one]);
    EXPECT_TRUE(b[mo_two]);
    EXPECT_TRUE(! b[mo_three]);
    EXPECT_TRUE(b[mo_four]);
    EXPECT_TRUE(! b[mo_five]);
    EXPECT_TRUE(b[mo_six]);
    EXPECT_TRUE(! b[mo_seven]);
}

