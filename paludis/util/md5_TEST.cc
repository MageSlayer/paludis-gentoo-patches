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

#include <paludis/util/md5.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string md5(const std::string & data)
    {
        std::stringstream ss(data);
        MD5 s(ss);
        return s.hexsum();
    }
}

TEST(MD5, t0)
{
    EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", md5(""));
}

TEST(MD5, t1)
{
    EXPECT_EQ("0cc175b9c0f1b6a831c399e269772661", md5("a"));
}

TEST(MD5, t2)
{
    EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", md5("abc"));
}

TEST(MD5, t3)
{
    EXPECT_EQ("f96b697d7cb7938d525a2f31aaf161d0", md5("message digest"));
}

TEST(MD5, t4)
{
    EXPECT_EQ("c3fcd3d76192e4007dfb496cca67e13b", md5("abcdefghijklmnopqrstuvwxyz"));
}

TEST(MD5, t6)
{
    EXPECT_EQ("d174ab98d277d9f5a5611c2c9f419d9f", md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
}

TEST(MD5, t7)
{
    EXPECT_EQ("57edf4a22be3c955ac49da2e2107b67a", md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

TEST(MD5, t8)
{
    EXPECT_EQ("7707d6ae4e027c70eea2a935c2296f21", md5(std::string(1000000, 'a')));
}

