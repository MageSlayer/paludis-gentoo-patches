/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2006 Mark Loeser
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

#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/timestamp.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <ctime>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct FSPathManipulationTest : TestCase
    {
        FSPathManipulationTest() : TestCase("construction and manipulation") { }

        void run()
        {
            FSPath f("/foo/bar");
            FSPath c(f);
            TEST_CHECK_EQUAL(f, FSPath("/foo/bar"));
            TEST_CHECK_EQUAL(c, FSPath("/foo/bar"));
            f = FSPath("/baz");
            TEST_CHECK_EQUAL(f, FSPath("/baz"));
            TEST_CHECK_EQUAL(c, FSPath("/foo/bar"));
            c /= "moo";
            TEST_CHECK_EQUAL(f, FSPath("/baz"));
            TEST_CHECK_EQUAL(c, FSPath("/foo/bar/moo"));
            f = c / f;
            TEST_CHECK_EQUAL(f, FSPath("/foo/bar/moo/baz"));
            TEST_CHECK_EQUAL(c, FSPath("/foo/bar/moo"));

            f = FSPath::cwd();

            TEST_CHECK_EQUAL(f, f / FSPath("/"));
        }
    } test_fs_path_manipulation;

    struct FSPathRealpathTest : TestCase
    {
        FSPathRealpathTest() : TestCase("realpath") { }

        void run()
        {
            FSPath f("fs_path_TEST_dir/symlink_to_dir_a/file_in_a");
            FSPath r(f.realpath());
            std::string g("fs_path_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_EQUAL(stringify(r).substr(stringify(r).length() - g.length()), g);
        }
    } test_fs_path_realpath;

    struct FSPathSymlink : TestCase
    {
        FSPathSymlink() : TestCase("symlink") {}

        void run()
        {
            FSPath f("fs_path_TEST_dir/new_sym");
            TEST_CHECK(f.symlink("the_target"));
            TEST_CHECK_EQUAL(f.readlink(), "the_target");
            f.unlink();
        }
    } test_fs_symlink;

    struct FSPathBaseDirName : TestCase
    {
        FSPathBaseDirName() : TestCase("basename and dirname") {}

        void run()
        {
            FSPath a("/foo/bar");
            FSPath b("/moo/went/the/cow");
            FSPath c("/");
            FSPath d(".");
            FSPath e("..");

            TEST_CHECK(a.basename() == "bar");
            TEST_CHECK(stringify(a.dirname()) == "/foo");
            TEST_CHECK(b.basename() == "cow");
            TEST_CHECK(stringify(b.dirname()) == "/moo/went/the");
            TEST_CHECK(c.basename() == "/");
            TEST_CHECK(stringify(c.dirname()) == "/");
            TEST_CHECK(d.basename() == ".");
            TEST_CHECK(stringify(d.dirname()) == ".");
            TEST_CHECK(e.basename() == "..");
            TEST_CHECK(stringify(e.dirname()) == "..");
        }
    } test_fs_path_dir_base_name;

    struct FSPathStripLeading : TestCase
    {
        FSPathStripLeading() : TestCase("strip_leading") {}

        void run()
        {
            FSPath root1("/stairway/to/heaven/");
            FSPath root2("");
            FSPath root3("/");

            FSPath a(root1);
            FSPath b(root1 / "usr" / "share");
            FSPath c(root2 / "my" / "directory");
            FSPath d(root3 / "my" / "directory");

            TEST_CHECK(stringify(a.strip_leading(root1)) == "/");
            TEST_CHECK(stringify(b.strip_leading(root1)) == "/usr/share");
            TEST_CHECK(stringify(c.strip_leading(root2)) == "/my/directory");
            TEST_CHECK(stringify(d.strip_leading(root3)) == "/my/directory");
        }
    } test_fs_path_strip_leading;

    struct FSPathToOstreamOperator : TestCase
    {
        FSPathToOstreamOperator() : TestCase("operator<<") {}

        void run()
        {
            std::string n("fs_path_TEST_dir/no_perms");
            std::ostringstream s;
            FSPath a(n);

            s << a;

            TEST_CHECK_EQUAL(s.str(), n);
        }
    } test_fs_path_to_ostream_operator;
}

