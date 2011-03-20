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

#include <paludis/util/wildcard_expander.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/join.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string expand(const std::string & pattern)
    {
        return join(WildcardExpander(pattern, FSPath::cwd() / "wildcard_expander_TEST_dir"),
                    WildcardExpander(), " ");
    }
}

TEST(WildcardExpander, Works)
{
    EXPECT_EQ("/xyz1zy /xyz22zy /xyzzy", expand("/xyz*zy"));
    EXPECT_EQ("/plugh", expand("/plugh"));
    EXPECT_EQ("/quux", expand("/quux"));
    EXPECT_EQ("/quux*quux", expand("/quux*quux"));
    EXPECT_EQ("/meh/1 /meh/2 /meh/3", expand("/meh/?"));
    EXPECT_EQ("/foo* /foo123*", expand("/foo*\\*"));
    EXPECT_EQ("/foo*", expand("/foo\\*"));
}

TEST(WildcardExpander, IteratorSanity)
{
    WildcardExpander it("/foo*bar", FSPath::cwd() / "wildcard_expander_TEST_dir");
    ASSERT_TRUE(it == it);
    ASSERT_TRUE(! (it != it));
    ASSERT_TRUE(it != WildcardExpander());
    EXPECT_EQ("/fooAbar", stringify(*it));
    EXPECT_EQ(it->basename(), "fooAbar");

    WildcardExpander it2(it);
    ASSERT_TRUE(it == it2);
    ASSERT_TRUE(! (it != it2));
    ASSERT_TRUE(it2 != WildcardExpander());
    EXPECT_EQ("/fooBbar", stringify(*++it2));
    EXPECT_EQ("/fooBbar", stringify(*it2));
    EXPECT_EQ(it2->basename(), "fooBbar");
    ASSERT_TRUE(it != it2);
    ASSERT_TRUE(! (it == it2));
    ASSERT_TRUE(it2 != WildcardExpander());

    WildcardExpander it3(it2);
    ASSERT_TRUE(it2 == it3++);
    ASSERT_TRUE(it2 != it3);
    ASSERT_TRUE(it3 != WildcardExpander());
    EXPECT_EQ("/fooCbar", stringify(*it3));
    EXPECT_EQ(it3->basename(), "fooCbar");

    it3 = it2;
    ASSERT_TRUE(it2 == it3);
    EXPECT_EQ("/fooBbar", stringify(*it3));
    EXPECT_EQ("/fooBbar", stringify(*it3++));
    ASSERT_TRUE(it3 != WildcardExpander());

    ASSERT_TRUE(++it3 != WildcardExpander());
    ASSERT_TRUE(++it3 != WildcardExpander());
    ASSERT_TRUE(++it3 == WildcardExpander());

    EXPECT_EQ("/fooAbar /fooBbar /fooCbar /fooDbar /fooEbar",
            join(WildcardExpander("/foo*bar", FSPath::cwd() / "wildcard_expander_TEST_dir"), WildcardExpander(), " "));
}

