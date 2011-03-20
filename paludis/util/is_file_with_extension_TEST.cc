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

#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/options.hh>

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

TEST(IsFileWithExtension, Works)
{
    FSPath c("teh.foo");
    FSPath d("is_file_with_extension_TEST_file.goat");

    ASSERT_TRUE(d.stat().exists());

    EXPECT_TRUE(! is_file_with_extension(c, "foo", { }));
    EXPECT_TRUE(! is_file_with_extension(d, "foo", { }));
    EXPECT_TRUE(! is_file_with_extension(c, "goat", { }));
    EXPECT_TRUE(is_file_with_extension(d, "goat", { }));

}

TEST(IsFileWithPrefixExtension, Works)
{
    FSPath d("teh.foo");
    FSPath e("is_file_with_extension_TEST_file.goat");

    ASSERT_TRUE(e.stat().exists());

    EXPECT_TRUE(! is_file_with_prefix_extension(d, "teh", "foo", { }));
    EXPECT_TRUE(! is_file_with_prefix_extension(e, "teh", "foo", { }));
    EXPECT_TRUE(! is_file_with_prefix_extension(d, "is", "goat", { }));
    EXPECT_TRUE(is_file_with_prefix_extension(e, "is", "goat", { }));
    EXPECT_TRUE(! is_file_with_prefix_extension(e, "with", "goat", { }));
}

