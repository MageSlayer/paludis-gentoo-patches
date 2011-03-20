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
#include <paludis/util/fs_stat.hh>
#include <paludis/util/log.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/options.hh>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <ctime>

#include <gtest/gtest.h>

using namespace paludis;

TEST(FSStat, Time)
{
    FSPath a("fs_stat_TEST_dir");
    FSPath b("fs_stat_TEST_dir/no_perms");
    FSPath c("fs_stat_TEST_dir/no_such_file");
    FSPath d("fs_stat_TEST_dir/dir_a/dir_in_a");

    FSStat a_stat(a);
    FSStat b_stat(b);
    FSStat c_stat(c);
    FSStat d_stat(d);

#if !defined(__FreeBSD__)
    EXPECT_TRUE(a_stat.ctim() < Timestamp::now());
    EXPECT_TRUE(a_stat.mtim() < Timestamp::now());
#endif
    EXPECT_TRUE(b_stat.ctim() < Timestamp::now());
    EXPECT_TRUE(b_stat.mtim() < Timestamp::now());
    EXPECT_TRUE(d_stat.ctim() < Timestamp::now());
    EXPECT_TRUE(d_stat.mtim() < Timestamp::now());

    EXPECT_THROW(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c_stat.ctim(), FSError);
    EXPECT_THROW(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c_stat.mtim(), FSError);

    EXPECT_TRUE(b_stat.mtim() < b_stat.ctim());
    EXPECT_TRUE(d_stat.mtim() == d_stat.ctim());
}

TEST(FSStat, Manipulation)
{
    FSPath f("/foo/bar");
    FSPath c(f);
    EXPECT_EQ(f, FSPath("/foo/bar"));
    EXPECT_EQ(c, FSPath("/foo/bar"));
    f = FSPath("/baz");
    EXPECT_EQ(f, FSPath("/baz"));
    EXPECT_EQ(c, FSPath("/foo/bar"));
    c /= "moo";
    EXPECT_EQ(f, FSPath("/baz"));
    EXPECT_EQ(c, FSPath("/foo/bar/moo"));
    f = c / f;
    EXPECT_EQ(f, FSPath("/foo/bar/moo/baz"));
    EXPECT_EQ(c, FSPath("/foo/bar/moo"));

    f = FSPath::cwd();

    EXPECT_EQ(f, f / FSPath("/"));
}

TEST(FSStat, Realpath)
{
    FSPath d("fs_stat_TEST_dir");
    EXPECT_TRUE(d.stat().is_directory());

    d /= "all_perms";
    EXPECT_TRUE(d.stat().is_regular_file());

    FSPath e("fs_stat_TEST_dir/nosuchfile");
    EXPECT_TRUE(! e.stat().is_regular_file());
    d = e;
    EXPECT_TRUE(! d.stat().is_regular_file());
    EXPECT_TRUE(! d.stat().exists());
    EXPECT_THROW(e.readlink(), FSError);

    d = FSPath("fs_stat_TEST_dir/all_perms");
    EXPECT_TRUE(! e.stat().is_regular_file());
    EXPECT_TRUE(d.stat().is_regular_file());
    EXPECT_TRUE(d.stat().exists());

    d = FSPath("fs_stat_TEST_dir/symlink_to_dir_a");
    EXPECT_TRUE(d.stat().is_symlink());
    EXPECT_TRUE(! d.stat().is_directory());
    EXPECT_TRUE(d.stat().is_directory_or_symlink_to_directory());
    EXPECT_TRUE(! d.stat().is_regular_file());
    EXPECT_TRUE(! d.stat().is_regular_file_or_symlink_to_regular_file());

    e = FSPath("fs_stat_TEST_dir/doesnotexist_symlink");
    EXPECT_TRUE(e.stat().is_symlink());
    EXPECT_TRUE(e.stat().exists());
    EXPECT_TRUE(! e.stat().is_directory());
    EXPECT_TRUE(! e.stat().is_directory_or_symlink_to_directory());
    EXPECT_TRUE(! e.stat().is_regular_file());
    EXPECT_TRUE(! e.stat().is_regular_file_or_symlink_to_regular_file());

    FSPath f("fs_stat_TEST_dir/symlink_to_dir_a/file_in_a");
    EXPECT_TRUE(f.stat().is_regular_file());
    EXPECT_TRUE(! f.stat().is_symlink());
    FSPath r(f.realpath());
    EXPECT_TRUE(r.stat().is_regular_file());
    std::string g("fs_stat_TEST_dir/dir_a/file_in_a");
    EXPECT_EQ(g, stringify(r).substr(stringify(r).length() - g.length()));

    FSPath h("fs_stat_TEST_dir/symlink_to_file_in_a");
    EXPECT_TRUE(h.stat().is_symlink());
    EXPECT_TRUE(! h.stat().is_regular_file());
    EXPECT_TRUE(h.stat().is_regular_file_or_symlink_to_regular_file());
    EXPECT_EQ("dir_a/file_in_a", h.readlink());

    FSPath i("fs_stat_TEST_dir/dir_to_make");
    EXPECT_TRUE(i.mkdir(0755, { fspmkdo_ok_if_exists }));
    EXPECT_TRUE(! i.mkdir(0755, { fspmkdo_ok_if_exists }));
    EXPECT_TRUE(i.rmdir());
    FSPath j("fs_stat_TEST_dir/dir_to_make");
    EXPECT_TRUE(! j.stat().exists());
    EXPECT_TRUE(! j.stat().is_directory());

    FSPath k("fs_stat_TEST_dir/dir_a/file_in_a");
    EXPECT_THROW(k.mkdir(0755, { fspmkdo_ok_if_exists }), FSError);

    FSPath l("fs_stat_TEST_dir/file_a/file_that_triggers_ENOTDIR");
    EXPECT_TRUE(! l.stat().exists());
}

TEST(FSStat, Size)
{
    FSPath f("fs_stat_TEST_dir/ten_bytes");
    FSPath d("fs_stat_TEST_dir/dir_a");
    FSPath e("fs_stat_TEST_dir/no_such_file");

    EXPECT_EQ(10, f.stat().file_size());
    EXPECT_THROW(size_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().file_size(), FSError);
    EXPECT_THROW(size_t PALUDIS_ATTRIBUTE((unused)) x = e.stat().file_size(), FSError);
}

TEST(FSStat, Symlink)
{
    FSPath f("fs_stat_TEST_dir/new_sym");
    EXPECT_TRUE(f.symlink("the_target"));
    EXPECT_TRUE(f.stat().is_symlink());
    EXPECT_EQ("the_target", f.readlink());
    f.unlink();
}

TEST(FSStat, Perms)
{
    FSPath a("fs_stat_TEST_dir/no_perms");

    uid_t my_uid = geteuid();
    a.chown(my_uid, -1);
    EXPECT_EQ(my_uid, a.stat().owner());

    mode_t all_perms(S_IRUSR | S_IWUSR | S_IXUSR |
                     S_IRGRP | S_IWGRP | S_IXGRP |
                     S_IROTH | S_IWOTH | S_IXOTH);
    a.chmod(all_perms);

    FSPath b("fs_stat_TEST_dir/no_perms");

    EXPECT_EQ(all_perms, static_cast<mode_t>(b.stat().permissions() & 0xFFF));

    mode_t no_perms(0);
    b.chmod(no_perms);

    FSPath c("fs_stat_TEST_dir/no_perms");
    EXPECT_EQ(no_perms, static_cast<mode_t>(c.stat().permissions() & 0xFFF));

    FSPath d("fs_stat_TEST_dir/i_dont_exist");

    EXPECT_THROW(mode_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().permissions(), FSError);
    EXPECT_THROW(d.chmod(all_perms), FSError);
    EXPECT_THROW(d.chown(static_cast<uid_t>(-1), -1), FSError);
    EXPECT_THROW(uid_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().owner(), FSError);
    EXPECT_THROW(gid_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().group(), FSError);

    if (0 == my_uid)
    {
        struct passwd *pw = getpwent();

        if (! pw)
            throw InternalError(PALUDIS_HERE, "getpwent returned NULL");

        std::string my_file("fs_stat_TEST_dir/all_perms");
        FSPath e(my_file);

        uid_t my_owner = e.stat().owner();
        gid_t my_group = e.stat().group();

        if (pw->pw_uid == my_owner)
        {
            pw = getpwent();

            if (! pw)
                throw InternalError(PALUDIS_HERE, "getpwent returned NULL");
        }

        uid_t new_owner(pw->pw_uid);

        e.chown(new_owner, -1);

        EXPECT_EQ(new_owner, FSPath(my_file).stat().owner());
        EXPECT_EQ(my_group, FSPath(my_file).stat().group());

        gid_t new_group(pw->pw_gid);

        endpwent();

        e.chown(static_cast<uid_t>(-1), new_group);

        EXPECT_EQ(new_owner, FSPath(my_file).stat().owner());
        EXPECT_EQ(new_group, FSPath(my_file).stat().group());

        e.chown(static_cast<uid_t>(-1), -1);

        EXPECT_EQ(new_owner, FSPath(my_file).stat().owner());
        EXPECT_EQ(new_group, FSPath(my_file).stat().group());

        e.chown(my_owner, my_group);

        EXPECT_EQ(my_owner, FSPath(my_file).stat().owner());
        EXPECT_EQ(my_group, FSPath(my_file).stat().group());
    }
    else
    {
        FSPath e("fs_stat_TEST_dir/all_perms");

        EXPECT_THROW(e.chown(0, 0), FSError);
    }
}

