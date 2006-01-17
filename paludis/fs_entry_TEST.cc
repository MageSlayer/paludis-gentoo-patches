/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "fs_entry.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for fs_entry.hh.
 *
 * \todo this is nowhere near complete.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test FSEntry construction and manipulation.
     */
    struct FSEntryManipulationTest : TestCase
    {
        FSEntryManipulationTest() : TestCase("construction and manipulation") { }

        void run()
        {
            FSEntry f("/foo/bar");
            FSEntry c(f);
            TEST_CHECK_EQUAL(f, FSEntry("/foo/bar"));
            TEST_CHECK_EQUAL(c, FSEntry("/foo/bar"));
            f = FSEntry("/baz");
            TEST_CHECK_EQUAL(f, FSEntry("/baz"));
            TEST_CHECK_EQUAL(c, FSEntry("/foo/bar"));
            c /= "moo";
            TEST_CHECK_EQUAL(f, FSEntry("/baz"));
            TEST_CHECK_EQUAL(c, FSEntry("/foo/bar/moo"));
            f = c / f;
            TEST_CHECK_EQUAL(f, FSEntry("/foo/bar/moo/baz"));
            TEST_CHECK_EQUAL(c, FSEntry("/foo/bar/moo"));
        }
    } test_fs_entry_manipulation;

    /**
     * \test Test FSEntry realpath.
     */
    struct FSEntryRealpathTest : TestCase
    {
        FSEntryRealpathTest() : TestCase("realpath") { }

        void run()
        {
            FSEntry d("fs_entry_TEST_dir");
            TEST_CHECK(d.is_directory());

            FSEntry f("fs_entry_TEST_dir/symlink_to_dir_a/file_in_a");
            TEST_CHECK(f.is_regular_file());
            FSEntry r(f.realpath());
            TEST_CHECK(r.is_regular_file());
            std::string e("fs_entry_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_EQUAL(std::string(r).substr(std::string(r).length() - e.length()), e);
        }
    } test_fs_entry_realpath;
}
