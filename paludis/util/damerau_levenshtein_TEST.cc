/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include <paludis/util/damerau_levenshtein.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(DamerauLevenshtein, Distance)
{
    DamerauLevenshtein dl("foo");

    EXPECT_EQ(0u, dl.distance_with("foo"));
    EXPECT_EQ(1u, dl.distance_with("foo1"));
    EXPECT_EQ(1u, dl.distance_with("fo"));
    EXPECT_EQ(1u, dl.distance_with("fao"));
    EXPECT_EQ(1u, dl.distance_with("ofo"));
    EXPECT_EQ(2u, dl.distance_with("fie"));
    EXPECT_EQ(3u, dl.distance_with("ife"));
    EXPECT_EQ(3u, dl.distance_with("bar"));
    EXPECT_EQ(3u, dl.distance_with(""));

    DamerauLevenshtein de("");

    EXPECT_EQ(3u, de.distance_with("foo"));
    EXPECT_EQ(0u, de.distance_with(""));
}

