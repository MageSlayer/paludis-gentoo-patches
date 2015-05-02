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

#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_path.hh>

#include <unistd.h>
#include <sys/types.h>

#include <gtest/gtest.h>

using namespace paludis;

TEST(SafeOFStream, New)
{
    SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "new", -1, false);
    ASSERT_TRUE(bool(s));
    s << "foo";
    ASSERT_TRUE(bool(s));
}

TEST(SafeOFStream, Existing)
{
    SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing", -1, false);
    ASSERT_TRUE(bool(s));
    s << "foo";
    ASSERT_TRUE(bool(s));
}

TEST(SafeOFStream, ExistingSym)
{
    SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_sym", -1, false);
    ASSERT_TRUE(bool(s));
    s << "foo";
    ASSERT_TRUE(bool(s));
}

TEST(SafeOFStream, ExistingDir)
{
    EXPECT_THROW(SafeOFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_dir", -1, false), SafeOFStreamError);
}

TEST(SafeOFStream, ExistingPerm)
{
    if (0 != getuid())
    {
        EXPECT_THROW(SafeOFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_perm", -1, false), SafeOFStreamError);
    }
}

TEST(SafeOFStream, WriteFailure)
{
    bool threw(false);
    try
    {
        SafeOFStream s(FSPath("/dev/full"), -1, false);
        ASSERT_TRUE(bool(s));
        s << "foo";
        ASSERT_TRUE(! s);
    }
    catch (const SafeOFStreamError &)
    {
        threw = true;
    }

    ASSERT_TRUE(threw);
}

