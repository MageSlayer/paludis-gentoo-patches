/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2011 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#include <paludis/util/iterator_funcs.hh>

#include <vector>
#include <list>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Iterator, Next)
{
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    std::vector<int>::iterator iter(v.begin());

    ASSERT_TRUE(*(next(iter)) == 2);
    ASSERT_TRUE(next(next(next(iter))) == v.end());
    ASSERT_TRUE(next(iter, 3) == v.end());
    iter = next(iter);
    ASSERT_TRUE(*(next(iter, 1)) == 3);
    iter = next(iter);
    ASSERT_TRUE(++iter == v.end());
}

TEST(Iterator, Previous)
{
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    std::vector<int>::iterator iter(v.end());

    ASSERT_TRUE(*(prev(iter)) == 2);
    ASSERT_TRUE(prev(prev(iter)) == v.begin());
    iter = prev(iter);
    ASSERT_TRUE(--iter == v.begin());
}

TEST(CappedDistance, Works)
{
    std::list<int> v;
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 10));

    v.push_back(1);
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 10));

    v.push_back(2);
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(2, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(2, capped_distance(v.begin(), v.end(), 10));

    v.push_back(3);
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(3, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(3, capped_distance(v.begin(), v.end(), 10));

    v.push_back(4);
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(3, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(4, capped_distance(v.begin(), v.end(), 10));

    v.push_back(5);
    EXPECT_EQ(0, capped_distance(v.begin(), v.end(), 0));
    EXPECT_EQ(1, capped_distance(v.begin(), v.end(), 1));
    EXPECT_EQ(3, capped_distance(v.begin(), v.end(), 3));
    EXPECT_EQ(5, capped_distance(v.begin(), v.end(), 10));
}

