/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett
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

#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>

#include <string>

#include <gtest/gtest.h>

using namespace paludis;

TEST(DestringifyInt, Works)
{
    EXPECT_EQ(0, destringify<int>("0"));
    EXPECT_EQ(1, destringify<int>("1"));
    EXPECT_EQ(99, destringify<int>("99"));
    EXPECT_EQ(-99, destringify<int>("-99"));
    EXPECT_EQ(12345, destringify<int>(" 12345"));
}

TEST(DestringifyInt, Throws)
{
    EXPECT_THROW(destringify<int>(""), DestringifyError);
    EXPECT_THROW(destringify<int>("x"), DestringifyError);
    EXPECT_THROW(destringify<int>("10000000000000000000000000000000000000000000000"), DestringifyError);
}

TEST(DestringifyFloat, Works)
{
    EXPECT_FLOAT_EQ(0.0f, destringify<float>("0"));
    EXPECT_FLOAT_EQ(0.0f, destringify<float>("0.0"));
    EXPECT_FLOAT_EQ(0.1f, destringify<float>("0.1"));
    EXPECT_FLOAT_EQ(-1.54f, destringify<float>("-1.54"));
}

TEST(DestringifyFloat, Throws)
{
    EXPECT_THROW(destringify<float>("I am a fish"), DestringifyError);
    EXPECT_THROW(destringify<float>(""), DestringifyError);
}

TEST(DestringifyString, Works)
{
    EXPECT_EQ("asdf", destringify<std::string>("asdf"));
    EXPECT_EQ( "  a f     e b ", destringify<std::string>("  a f     e b "));
}

TEST(DestringifyString, Throws)
{
    EXPECT_THROW(destringify<std::string>(""), DestringifyError);
}

TEST(DestringifyBool, Works)
{
    EXPECT_TRUE(destringify<bool>("true"));
    EXPECT_TRUE(destringify<bool>("1"));
    EXPECT_TRUE(destringify<bool>("5"));

    EXPECT_FALSE(destringify<bool>("false"));
    EXPECT_FALSE(destringify<bool>("0"));
    EXPECT_FALSE(destringify<bool>("-1"));
}

TEST(DestringifyBool, Throws)
{
    EXPECT_THROW(destringify<bool>("flase"), DestringifyError);
    EXPECT_THROW(destringify<bool>("432.2413"), DestringifyError);
    EXPECT_THROW(destringify<bool>(""), DestringifyError);
}

TEST(DestringifyChar, Works)
{
    EXPECT_EQ('x', destringify<char>("x"));
    EXPECT_EQ('0', destringify<char>("0"));
}

TEST(DestringifyChar, Throws)
{
    EXPECT_THROW(destringify<char>("aa"), DestringifyError);
    EXPECT_THROW(destringify<char>("a a"), DestringifyError);
    EXPECT_THROW(destringify<char>("11"), DestringifyError);
    EXPECT_THROW(destringify<char>(""), DestringifyError);
}

