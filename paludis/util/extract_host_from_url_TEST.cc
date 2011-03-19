/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2011 Ciaran McCreesh
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

#include <paludis/util/extract_host_from_url.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(ExtractHostFromURL, Empty)
{
    EXPECT_EQ("", extract_host_from_url(""));
}

TEST(ExtractHostFromURL, NoPath)
{
    EXPECT_EQ("foo", extract_host_from_url("http://foo"));
}

TEST(ExtractHostFromURL, WithPath)
{
    EXPECT_EQ("foo", extract_host_from_url("http://foo/bar"));
    EXPECT_EQ("foo", extract_host_from_url("http://foo/bar/baz"));
}

