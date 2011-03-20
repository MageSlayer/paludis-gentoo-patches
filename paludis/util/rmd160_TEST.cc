/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2011 Ciaran McCreesh
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

#include <paludis/util/rmd160.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string rmd160(const std::string & data)
    {
        std::stringstream ss(data);
        RMD160 s(ss);
        return s.hexsum();
    }
}

TEST(RMD160, t0)
{
    EXPECT_EQ("9c1185a5c5e9fc54612808977ee8f548b2258d31", rmd160(""));
}

TEST(RMD160, t1)
{
    EXPECT_EQ("0bdc9d2d256b3ee9daae347be6f4dc835a467ffe", rmd160("a"));
}

TEST(RMD160, t2)
{
    EXPECT_EQ("8eb208f7e05d987a9b044a8e98c6b087f15a0bfc", rmd160("abc"));
}

TEST(RMD160, t3)
{
    EXPECT_EQ("5d0689ef49d2fae572b881b123a85ffa21595f36", rmd160("message digest"));
}

TEST(RMD160, t4)
{
    EXPECT_EQ("f71c27109c692c1b56bbdceb5b9d2865b3708dbc", rmd160("abcdefghijklmnopqrstuvwxyz"));
}

TEST(RMD160, t5)
{
    EXPECT_EQ("12a053384a9c0c88e405a06c27dcf49ada62eb2b", rmd160("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"));
}

TEST(RMD160, t6)
{
    EXPECT_EQ("b0e20b6e3116640286ed3a87a5713079b21f5189", rmd160("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
}

TEST(RMD160, t7)
{
    EXPECT_EQ("9b752e45573d4b39f4dbd3323cab82bf63326bfb", rmd160("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

TEST(RMD160, t8)
{
    EXPECT_EQ("52783243c1697bdbe16d37f97f68f08325dc1528", rmd160(std::string(1000000, 'a')));
}

