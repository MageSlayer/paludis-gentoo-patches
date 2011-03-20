/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/util/byte_swap.hh>

#include <stdint.h>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    int64_t int64(int64_t a, int64_t b)
    {
        return (a << 32) | b;
    }

    uint64_t uint64(uint64_t a, uint64_t b)
    {
        return (a << 32) | b;
    }
}

#define CHECK(T, X, Y) do { EXPECT_EQ(T(Y), byte_swap(T(X))); EXPECT_EQ(T(X), byte_swap(T(Y))); } while (0)

TEST(ByteSwap, Works)
{
    CHECK(int8_t,   0x42, 0x42);
    CHECK(uint8_t,  0x42, 0x42);
    CHECK(int8_t,   0x92, 0x92);
    CHECK(uint8_t,  0x92, 0x92);

    CHECK(int16_t,  0x4218, 0x1842);
    CHECK(uint16_t, 0x4218, 0x1842);
    CHECK(int16_t,  0x4298, 0x9842);
    CHECK(uint16_t, 0x4298, 0x9842);
    CHECK(int16_t,  0x9218, 0x1892);
    CHECK(uint16_t, 0x9218, 0x1892);

    CHECK(int32_t,  0x4218A43F, 0x3FA41842);
    CHECK(uint32_t, 0x4218A43F, 0x3FA41842);
    CHECK(int32_t,  0x4218A4CF, 0xCFA41842);
    CHECK(uint32_t, 0x4218A4CF, 0xCFA41842);
    CHECK(int32_t,  0x9218A43F, 0x3FA41892);
    CHECK(uint32_t, 0x9218A43F, 0x3FA41892);

    CHECK(int64_t,  int64(0x42A57104,  0x32D61259), int64(0x5912D632,  0x0471A542));
    CHECK(uint64_t, uint64(0x42A57104, 0x32D61259), uint64(0x5912D632, 0x0471A542));
    CHECK(int64_t,  int64(0x42A57104,  0x32D612E9), int64(0xE912D632,  0x0471A542));
    CHECK(uint64_t, uint64(0x42A57104, 0x32D612E9), uint64(0xE912D632, 0x0471A542));
    CHECK(int64_t,  int64(0x92A57104,  0x32D61259), int64(0x5912D632,  0x0471A592));
    CHECK(uint64_t, uint64(0x92A57104, 0x32D61259), uint64(0x5912D632, 0x0471A592));
}

