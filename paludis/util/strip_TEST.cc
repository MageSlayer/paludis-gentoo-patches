/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser
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


#include <paludis/util/strip.hh>

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

TEST(StripLeadingString, Works)
{
    EXPECT_TRUE("bar" == strip_leading_string("foobar", "foo"));
    EXPECT_TRUE("fishbar" == strip_leading_string("fishbar", "foo"));
    EXPECT_TRUE("" == strip_leading_string("foo", "foo"));
    EXPECT_TRUE("fishfoobar" == strip_leading_string("fishfoobar", "foo"));
    EXPECT_TRUE("blahfoo" == strip_leading_string("blahfoo", "foo"));
}

TEST(StripLeading, Works)
{
    EXPECT_TRUE("bar" == strip_leading("foobar", "foo"));
    EXPECT_TRUE("ishbar" == strip_leading("fishbar", "foo"));
    EXPECT_TRUE("" == strip_leading("foo", "foo"));
    EXPECT_TRUE("ishfoobar" == strip_leading("fishfoobar", "foo"));
    EXPECT_TRUE("blahfoo" == strip_leading("blahfoo", "foo"));
}

TEST(StripTrailingString, Works)
{
    EXPECT_TRUE("foobar" == strip_trailing_string("foobar", "foo"));
    EXPECT_TRUE("fishbar" == strip_trailing_string("fishbar", "foo"));
    EXPECT_TRUE("" == strip_trailing_string("foo", "foo"));
    EXPECT_TRUE("fishfoobar" == strip_trailing_string("fishfoobar", "foo"));
    EXPECT_TRUE("blah" == strip_trailing_string("blahfoo", "foo"));
}

TEST(StripTrailing, Works)
{
    EXPECT_TRUE("foobar" == strip_trailing("foobar", "foo"));
    EXPECT_TRUE("fishbar" == strip_trailing("fishbar", "foo"));
    EXPECT_TRUE("" == strip_trailing("foo", "foo"));
    EXPECT_TRUE("fishfoobar" == strip_trailing("fishfoobar", "foo"));
    EXPECT_TRUE("blah" == strip_trailing("blahfoo", "foo"));
}

