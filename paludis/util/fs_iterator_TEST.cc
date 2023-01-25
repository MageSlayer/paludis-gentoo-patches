/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/options.hh>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <algorithm>

using namespace paludis;
using testing::UnorderedElementsAre;

TEST(FSIterator, Manipulation)
{
    EXPECT_THROW(FSIterator(FSPath("/i/dont/exist/"), { }), FSError);

    FSIterator iter(FSPath("fs_iterator_TEST_dir"), { });
    FSIterator iter1(iter);
    ASSERT_TRUE(iter == iter1);
    ASSERT_TRUE(!(iter != iter1));
}

TEST(FSIterator, Iterate)
{
    const FSPath test_dir("fs_iterator_TEST_dir/iterate");
    FSIterator iter(test_dir, { });
    FSIterator iter1(test_dir, { });
    FSIterator iter2(test_dir, { fsio_include_dotfiles });
    FSIterator iter3(test_dir, { fsio_inode_sort });

    ASSERT_TRUE(iter != FSIterator());
    ASSERT_TRUE(FSIterator() != iter);

    EXPECT_EQ("file1", iter->basename());
    ASSERT_TRUE(++iter != FSIterator());
    EXPECT_EQ("file2", iter->basename());
    ASSERT_TRUE(++iter != FSIterator());
    EXPECT_EQ("file4", iter->basename());
    ASSERT_TRUE(++iter == FSIterator());
    ASSERT_TRUE(FSIterator() == iter);

    while (iter1 != FSIterator())
        ++iter1;
    ASSERT_TRUE(iter1 == FSIterator());
    ASSERT_TRUE(iter == iter1);

    EXPECT_EQ(".file3", iter2->basename());
    ASSERT_TRUE(++iter2 != FSIterator());
    EXPECT_EQ("file1", iter2->basename());
    ASSERT_TRUE(++iter2 != FSIterator());
    EXPECT_EQ("file2", iter2->basename());
    ASSERT_TRUE(++iter2 != FSIterator());
    EXPECT_EQ("file4", iter2->basename());
    ASSERT_TRUE(++iter2 == FSIterator());
    ASSERT_TRUE(FSIterator() == iter2);
    ASSERT_TRUE(iter2 == FSIterator());

    ASSERT_TRUE(iter1 == iter2);
    ASSERT_TRUE(iter2 == iter1);

    EXPECT_EQ(3, std::distance(iter3, FSIterator()));
}

TEST(FSIterator, SpecialWants)
{
    const FSPath test_dir("fs_iterator_TEST_dir/special-wants");
    FSIterator iter_want_dirs(test_dir, { fsio_want_directories });
    FSIterator iter_want_files(test_dir, { fsio_want_regular_files });
    FSIterator iter_symlinks(test_dir, { fsio_want_regular_files, fsio_deref_symlinks_for_wants });
    FSIterator iter_dotfiles(test_dir, { fsio_want_regular_files, fsio_include_dotfiles });

    const auto list_entries = [](const FSIterator & iter) {
        std::vector<std::string> result;
        std::transform(iter, FSIterator(), std::back_inserter(result), [](const FSPath & path) {
            return path.basename();
        });

        return result;
    };

    EXPECT_THAT(list_entries(iter_want_dirs), UnorderedElementsAre("dir1", "dir2"));
    EXPECT_THAT(list_entries(iter_want_files), UnorderedElementsAre("file1", "file2", "hardlink1"));
    EXPECT_THAT(
            list_entries(iter_symlinks),
            UnorderedElementsAre("file1", "file2", "hardlink1", "symlink1"));
    EXPECT_THAT(
            list_entries(iter_dotfiles),
            UnorderedElementsAre("file1", "file2", "hardlink1", ".file3"));
}
