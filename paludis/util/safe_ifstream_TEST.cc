/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_path.hh>

#include <unistd.h>
#include <sys/types.h>

#include <gtest/gtest.h>

using namespace paludis;

TEST(SafeIFStream, Existing)
{
    SafeIFStream s(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing");
    ASSERT_TRUE(bool(s));
    std::string t;
    s >> t;
    ASSERT_TRUE(bool(s));
    EXPECT_EQ("first", t);
    s >> t;
    EXPECT_EQ(std::string(1000, 'x'), t);

    s.clear();
    s.seekg(0, std::ios::beg);

    ASSERT_TRUE(bool(s));
    s >> t;
    ASSERT_TRUE(bool(s));
    EXPECT_EQ("first", t);
    s >> t;
    EXPECT_EQ(std::string(1000, 'x'), t);
}

TEST(SafeIFStream, ExistingSym)
{
    SafeIFStream s(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing");
    ASSERT_TRUE(bool(s));
    std::string t;
    s >> t;
    ASSERT_TRUE(bool(s));
    EXPECT_EQ("first", t);
    s >> t;
    EXPECT_EQ(std::string(1000, 'x'), t);
}

TEST(SafeIFStream, ExistingDir)
{
    EXPECT_THROW(SafeIFStream(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing_dir"), SafeIFStreamError);
}

TEST(SafeIFStream, ExistingPerm)
{
    if (0 != getuid())
    {
        EXPECT_THROW(SafeIFStream(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing_perm"), SafeIFStreamError);
    }
}

TEST(SafeIFStream, ExistingNoEnt)
{
    EXPECT_THROW(SafeIFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "noent"), SafeIFStreamError);
}

