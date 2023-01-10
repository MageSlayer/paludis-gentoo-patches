/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2022 Marvin Schmidt
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

#include <paludis/util/iterator_range.hh>
#include <list>
#include <vector>
#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace paludis;
using testing::ElementsAre;

TEST(IteratorRange, FromContainer)
{
    std::list<int> v = {1, 2, 3, 4, 5};
    auto range = IteratorRange<std::list<int>::iterator>{v};
    EXPECT_EQ(range.begin(), v.begin());
    EXPECT_EQ(range.end(), v.end());
    EXPECT_EQ(5, range.size());
    EXPECT_THAT(range, ElementsAre(1, 2, 3, 4, 5));
}

TEST(IteratorRange, FromIterators)
{
    std::list<int> v = {1, 2, 3, 4, 5};
    auto range = IteratorRange{v.begin(), std::prev(v.end())};
    EXPECT_EQ(range.begin(), v.begin());
    EXPECT_EQ(range.end(), std::prev(v.end()));
    EXPECT_EQ(4, range.size());
    EXPECT_THAT(range, ElementsAre(1, 2, 3, 4));
}

TEST(IteratorRange, MakeRange)
{
    std::vector<std::string> v = {"one", "two", "three"};
    auto range = make_range(v.begin(), v.end());
    EXPECT_EQ(range.begin(), v.begin());
    EXPECT_EQ(range.end(), v.end());
    EXPECT_EQ(3, range.size());
    EXPECT_THAT(range, ElementsAre("one", "two", "three"));
}

TEST(IteratorRange, MakeRangeFromPair)
{
    std::list<int> v = {1, 2, 3, 3, 3, 4, 5};
    auto range = make_range(std::equal_range(v.begin(), v.end(), 3));
    EXPECT_EQ(3, range.size());
    EXPECT_THAT(range, ElementsAre(3, 3, 3));
}

TEST(IteratorRange, Empty)
{
    std::vector<std::string> v1;
    auto range1 = make_range(v1.begin(), v1.end());
    EXPECT_EQ(0, range1.size());
    EXPECT_TRUE(range1.empty());

    std::list<int> v2 = {1, 2, 3, 3, 3, 4, 5};
    auto range2 = make_range(std::equal_range(v2.begin(), v2.end(), 8));
    EXPECT_EQ(0, range2.size());
    EXPECT_TRUE(range2.empty());
}
