/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>

#include <algorithm>
#include <vector>
#include <list>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    struct Deleter
    {
        template <typename T_>
        void operator() (T_ t)
        {
            delete t;
        }
    };
}

TEST(IndirectIterator, VectorSharedInt)
{
    std::vector<std::shared_ptr<int> > v;
    v.push_back(std::make_shared<int>(5));
    v.push_back(std::make_shared<int>(10));
    IndirectIterator<std::vector<std::shared_ptr<int> >::iterator, int> vi(v.begin());
    IndirectIterator<std::vector<std::shared_ptr<int> >::iterator, int> vi_end(v.end());
    ASSERT_TRUE(vi != vi_end);
    ASSERT_TRUE(vi < vi_end);
    ASSERT_TRUE(! (vi > vi_end));
    EXPECT_EQ(5, *vi);
    ASSERT_TRUE(++vi != vi_end);
    ASSERT_TRUE(vi < vi_end);
    ASSERT_TRUE(! (vi > vi_end));
    EXPECT_EQ(10, *vi);
    ASSERT_TRUE(++vi == vi_end);
}

TEST(IndirectIterator, ListSharedInt)
{
    std::list<std::shared_ptr<int> > v;
    v.push_back(std::make_shared<int>(5));
    v.push_back(std::make_shared<int>(10));
    IndirectIterator<std::list<std::shared_ptr<int> >::iterator> vi(v.begin());
    IndirectIterator<std::list<std::shared_ptr<int> >::iterator> vi_end(v.end());
    ASSERT_TRUE(vi != vi_end);
    EXPECT_EQ(5, *vi);
    ASSERT_TRUE(++vi != vi_end);
    EXPECT_EQ(10, *vi);
    ASSERT_TRUE(++vi == vi_end);
}

TEST(IndirectIterator, VectorIntStar)
{
    std::vector<int *> v;
    v.push_back(new int(5));
    v.push_back(new int(10));
    IndirectIterator<std::vector<int *>::iterator, int> vi(v.begin());
    IndirectIterator<std::vector<int *>::iterator, int> vi_end(v.end());
    ASSERT_TRUE(vi != vi_end);
    ASSERT_TRUE(vi < vi_end);
    ASSERT_TRUE(! (vi > vi_end));
    EXPECT_EQ(5, *vi);
    ASSERT_TRUE(++vi != vi_end);
    ASSERT_TRUE(vi < vi_end);
    ASSERT_TRUE(! (vi > vi_end));
    EXPECT_EQ(10, *vi);
    ASSERT_TRUE(++vi == vi_end);

    std::for_each(v.begin(), v.end(), Deleter());
}

TEST(IndirectIterator, ListIntStar)
{
    std::list<int *> v;
    v.push_back(new int(5));
    v.push_back(new int(10));
    IndirectIterator<std::list<int *>::iterator, int> vi(v.begin());
    IndirectIterator<std::list<int *>::iterator, int> vi_end(v.end());
    ASSERT_TRUE(vi != vi_end);
    EXPECT_EQ(5, *vi);
    ASSERT_TRUE(++vi != vi_end);
    EXPECT_EQ(10, *vi);
    ASSERT_TRUE(++vi == vi_end);

    std::for_each(v.begin(), v.end(), Deleter());
}

TEST(IndirectIterator, ListIntListIterator)
{
    std::list<int> v;
    v.push_back(5);
    v.push_back(10);

    std::list<std::list<int>::iterator> w;
    w.push_back(v.begin());
    w.push_back(next(v.begin()));

    EXPECT_EQ("5, 10", join(indirect_iterator(w.begin()), indirect_iterator(w.end()), ", "));
}

