/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2011 Ciaran McCreesh
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

#include <paludis/util/join.hh>

#include <list>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string make_purdy(const int x)
    {
        return "<" + stringify(x) + ">";
    }
}

TEST(Join, Vector)
{
    std::vector<std::string> v;
    v.push_back("one");
    EXPECT_EQ("one", join(v.begin(), v.end(), "/"));
    v.push_back("two");
    EXPECT_EQ("one/two", join(v.begin(), v.end(), "/"));
    v.push_back("three");
    EXPECT_EQ("one/two/three", join(v.begin(), v.end(), "/"));
}

TEST(Join, List)
{
    std::list<std::string> v;
    v.push_back("one");
    EXPECT_EQ("one", join(v.begin(), v.end(), "/"));
    v.push_back("two");
    EXPECT_EQ("one/two", join(v.begin(), v.end(), "/"));
    v.push_back("three");
    EXPECT_EQ("one/two/three", join(v.begin(), v.end(), "/"));
}

TEST(Join, IntList)
{
    std::list<int> v;
    v.push_back(1);
    EXPECT_EQ("1", join(v.begin(), v.end(), "/"));
    v.push_back(2);
    EXPECT_EQ("1/2", join(v.begin(), v.end(), "/"));
    v.push_back(3);
    EXPECT_EQ("1/2/3", join(v.begin(), v.end(), "/"));
}

TEST(Join, Empty)
{
    std::list<std::string> v;
    EXPECT_EQ("", join(v.begin(), v.end(), ""));
    EXPECT_EQ("", join(v.begin(), v.end(), "*"));
    v.push_back("");
    EXPECT_EQ("", join(v.begin(), v.end(), ""));
    EXPECT_EQ("", join(v.begin(), v.end(), "*"));
    v.push_back("");
    EXPECT_EQ("", join(v.begin(), v.end(), ""));
    EXPECT_EQ("*", join(v.begin(), v.end(), "*"));
}

TEST(Join, Function)
{
    std::list<int> v;
    v.push_back(1);
    EXPECT_EQ("<1>", join(v.begin(), v.end(), "/", &make_purdy));
    v.push_back(2);
    EXPECT_EQ("<1>/<2>", join(v.begin(), v.end(), "/", &make_purdy));
    v.push_back(3);
    EXPECT_EQ("<1>/<2>/<3>", join(v.begin(), v.end(), "/", &make_purdy));
}

