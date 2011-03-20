/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2011 Ciaran McCreesh
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

#include <paludis/about.hh>

#include <gtest/gtest.h>

TEST(About, Version)
{
    EXPECT_TRUE(PALUDIS_VERSION_MAJOR >= 0);
    EXPECT_TRUE(PALUDIS_VERSION_MAJOR <= 9);

    EXPECT_TRUE(PALUDIS_VERSION_MINOR >= 0);
    EXPECT_TRUE(PALUDIS_VERSION_MINOR <= 99);

    EXPECT_TRUE(PALUDIS_VERSION_MICRO >= 0);
    EXPECT_TRUE(PALUDIS_VERSION_MICRO <= 99);

    EXPECT_TRUE(PALUDIS_VERSION >= 0);
    EXPECT_TRUE(PALUDIS_VERSION <= 99999);
    EXPECT_EQ(PALUDIS_VERSION, 10000 * PALUDIS_VERSION_MAJOR + 100 * PALUDIS_VERSION_MINOR + PALUDIS_VERSION_MICRO);

    EXPECT_TRUE(std::string(PALUDIS_GIT_HEAD) != "i am a fish");
}

TEST(About, BuildInfo)
{
    EXPECT_TRUE(! std::string(PALUDIS_BUILD_CXX).empty());
    EXPECT_TRUE(! std::string(PALUDIS_BUILD_CXXFLAGS).empty());
    EXPECT_TRUE(std::string(PALUDIS_BUILD_LDFLAGS) != "i am a fish");

    EXPECT_TRUE(! std::string(PALUDIS_BUILD_USER).empty());
    EXPECT_TRUE(! std::string(PALUDIS_BUILD_HOST).empty());
    EXPECT_TRUE(! std::string(PALUDIS_BUILD_DATE).empty());
}

