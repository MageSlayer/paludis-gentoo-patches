/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <ctime>

#include <gtest/gtest.h>

using namespace paludis;

TEST(FSPath, Manipulation)
{
    FSPath f("/foo/bar");
    FSPath c(f);
    EXPECT_EQ(FSPath("/foo/bar"), f);
    EXPECT_EQ(FSPath("/foo/bar"), c);
    f = FSPath("/baz");
    EXPECT_EQ(FSPath("/baz"), f);
    EXPECT_EQ(FSPath("/foo/bar"), c);
    c /= "moo";
    EXPECT_EQ(FSPath("/baz"), f);
    EXPECT_EQ(FSPath("/foo/bar/moo"), c);
    f = c / f;
    EXPECT_EQ(FSPath("/foo/bar/moo/baz"), f);
    EXPECT_EQ(FSPath("/foo/bar/moo"), c);

    f = FSPath::cwd();

    EXPECT_EQ(f, f / FSPath("/"));
}

TEST(FSPath, Realpath)
{
    FSPath f("fs_path_TEST_dir/symlink_to_dir_a/file_in_a");
    FSPath r(f.realpath());
    std::string g("fs_path_TEST_dir/dir_a/file_in_a");
    EXPECT_EQ(g, stringify(r).substr(stringify(r).length() - g.length()));
}

TEST(FSPath, Symlink)
{
    FSPath f("fs_path_TEST_dir/new_sym");
    EXPECT_TRUE(f.symlink("the_target"));
    EXPECT_EQ("the_target", f.readlink());
    f.unlink();
}

TEST(FSPath, BaseDirName)
{
    FSPath a("/foo/bar");
    FSPath b("/moo/went/the/cow");
    FSPath c("/");
    FSPath d(".");
    FSPath e("..");

    EXPECT_TRUE(a.basename() == "bar");
    EXPECT_TRUE(stringify(a.dirname()) == "/foo");
    EXPECT_TRUE(b.basename() == "cow");
    EXPECT_TRUE(stringify(b.dirname()) == "/moo/went/the");
    EXPECT_TRUE(c.basename() == "/");
    EXPECT_TRUE(stringify(c.dirname()) == "/");
    EXPECT_TRUE(d.basename() == ".");
    EXPECT_TRUE(stringify(d.dirname()) == ".");
    EXPECT_TRUE(e.basename() == "..");
    EXPECT_TRUE(stringify(e.dirname()) == "..");
}

TEST(FSPath, StripLeading)
{
    FSPath root1("/stairway/to/heaven/");
    FSPath root2("");
    FSPath root3("/");

    FSPath a(root1);
    FSPath b(root1 / "usr" / "share");
    FSPath c(root2 / "my" / "directory");
    FSPath d(root3 / "my" / "directory");

    EXPECT_TRUE(stringify(a.strip_leading(root1)) == "/");
    EXPECT_TRUE(stringify(b.strip_leading(root1)) == "/usr/share");
    EXPECT_TRUE(stringify(c.strip_leading(root2)) == "/my/directory");
    EXPECT_TRUE(stringify(d.strip_leading(root3)) == "/my/directory");
}

TEST(FSPath, OStream)
{
    std::string n("fs_path_TEST_dir/no_perms");
    std::ostringstream s;
    FSPath a(n);

    s << a;

    EXPECT_EQ(n, s.str());
}

