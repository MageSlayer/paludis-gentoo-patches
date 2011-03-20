/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2011 Ciaran McCreesh
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

#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/join.hh>

#include <map>

#include <gtest/gtest.h>

using namespace paludis;

TEST(SecondIterator, Works)
{
    typedef std::map<std::string, std::string> M;

    M m;
    m["I"] = "one";
    m["II"] = "two";
    m["III"] = "three";
    m["IV"] = "four";
    m["V"] = "five";

    SecondIteratorTypes<M::iterator>::Type it = second_iterator(m.begin());
    ASSERT_TRUE(it == it);
    ASSERT_TRUE(! (it != it));
    EXPECT_EQ("one", *it);
    EXPECT_EQ(3, it->length());

    SecondIteratorTypes<M::iterator>::Type it2(it);
    ASSERT_TRUE(it == it2);
    ASSERT_TRUE(! (it != it2));
    EXPECT_EQ("two", *++it2);
    EXPECT_EQ("two", *it2);
    EXPECT_EQ(3, it2->length());
    ASSERT_TRUE(it != it2);
    ASSERT_TRUE(! (it == it2));

    SecondIteratorTypes<M::iterator>::Type it3(it2);
    ASSERT_TRUE(it2 == it3++);
    ASSERT_TRUE(it2 != it3);
    EXPECT_EQ("three", *it3);
    EXPECT_EQ(5, it3->length());

    it3 = it2;
    ASSERT_TRUE(it2 == it3);
    EXPECT_EQ("two", *it3);
    EXPECT_EQ("two", *it3++);

    EXPECT_EQ("one two three four five", join(second_iterator(m.begin()), second_iterator(m.end()), " "));
}

