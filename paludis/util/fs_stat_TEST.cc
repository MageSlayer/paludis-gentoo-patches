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
#include <paludis/util/fs_stat.hh>
#include <paludis/util/log.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/options.hh>
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
    struct FSPathTime : TestCase
    {
        FSPathTime() : TestCase("ctime and mtime") {}

        bool repeatable() const
        {
            return false;
        }

        void run()
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
            TEST_CHECK(a_stat.ctim() < Timestamp::now());
            TEST_CHECK(a_stat.mtim() < Timestamp::now());
#endif
            TEST_CHECK(b_stat.ctim() < Timestamp::now());
            TEST_CHECK(b_stat.mtim() < Timestamp::now());
            TEST_CHECK(d_stat.ctim() < Timestamp::now());
            TEST_CHECK(d_stat.mtim() < Timestamp::now());

            TEST_CHECK_THROWS(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c_stat.ctim(), FSError);
            TEST_CHECK_THROWS(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c_stat.mtim(), FSError);

            TEST_CHECK(b_stat.mtim() < b_stat.ctim());
            TEST_CHECK(d_stat.mtim() == d_stat.ctim());
        }
    } test_fs_stat_time;

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
    } test_fs_stat_manipulation;

    struct FSPathRealpathTest : TestCase
    {
        FSPathRealpathTest() : TestCase("behavior") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            FSPath d("fs_stat_TEST_dir");
            TEST_CHECK(d.stat().is_directory());

            d /= "all_perms";
            TEST_CHECK(d.stat().is_regular_file());

            FSPath e("fs_stat_TEST_dir/nosuchfile");
            TEST_CHECK(! e.stat().is_regular_file());
            d = e;
            TEST_CHECK(! d.stat().is_regular_file());
            TEST_CHECK(! d.stat().exists());
            TEST_CHECK_THROWS(e.readlink(), FSError);

            d = FSPath("fs_stat_TEST_dir/all_perms");
            TEST_CHECK(! e.stat().is_regular_file());
            TEST_CHECK(d.stat().is_regular_file());
            TEST_CHECK(d.stat().exists());

            d = FSPath("fs_stat_TEST_dir/symlink_to_dir_a");
            TEST_CHECK(d.stat().is_symlink());
            TEST_CHECK(! d.stat().is_directory());
            TEST_CHECK(d.stat().is_directory_or_symlink_to_directory());
            TEST_CHECK(! d.stat().is_regular_file());
            TEST_CHECK(! d.stat().is_regular_file_or_symlink_to_regular_file());

            e = FSPath("fs_stat_TEST_dir/doesnotexist_symlink");
            TEST_CHECK(e.stat().is_symlink());
            TEST_CHECK(e.stat().exists());
            TEST_CHECK(! e.stat().is_directory());
            TEST_CHECK(! e.stat().is_directory_or_symlink_to_directory());
            TEST_CHECK(! e.stat().is_regular_file());
            TEST_CHECK(! e.stat().is_regular_file_or_symlink_to_regular_file());

            FSPath f("fs_stat_TEST_dir/symlink_to_dir_a/file_in_a");
            TEST_CHECK(f.stat().is_regular_file());
            TEST_CHECK(! f.stat().is_symlink());
            FSPath r(f.realpath());
            TEST_CHECK(r.stat().is_regular_file());
            std::string g("fs_stat_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_EQUAL(stringify(r).substr(stringify(r).length() - g.length()), g);

            FSPath h("fs_stat_TEST_dir/symlink_to_file_in_a");
            TEST_CHECK(h.stat().is_symlink());
            TEST_CHECK(! h.stat().is_regular_file());
            TEST_CHECK(h.stat().is_regular_file_or_symlink_to_regular_file());
            TEST_CHECK_EQUAL(h.readlink(), "dir_a/file_in_a");

            FSPath i("fs_stat_TEST_dir/dir_to_make");
            TEST_CHECK(i.mkdir(0755, { fspmkdo_ok_if_exists }));
            TEST_CHECK(! i.mkdir(0755, { fspmkdo_ok_if_exists }));
            TEST_CHECK(i.rmdir());
            FSPath j("fs_stat_TEST_dir/dir_to_make");
            TEST_CHECK(! j.stat().exists());
            TEST_CHECK(! j.stat().is_directory());

            FSPath k("fs_stat_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_THROWS(k.mkdir(0755, { fspmkdo_ok_if_exists }), FSError);

            FSPath l("fs_stat_TEST_dir/file_a/file_that_triggers_ENOTDIR");
            TEST_CHECK(! l.stat().exists());
        }
    } test_fs_stat_behaviour;

    struct FSPathFileSize : TestCase
    {
        FSPathFileSize() : TestCase("file size") {}

        void run()
        {
            FSPath f("fs_stat_TEST_dir/ten_bytes");
            FSPath d("fs_stat_TEST_dir/dir_a");
            FSPath e("fs_stat_TEST_dir/no_such_file");

            TEST_CHECK_EQUAL(f.stat().file_size(), 10);
            TEST_CHECK_THROWS(size_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().file_size(), FSError);
            TEST_CHECK_THROWS(size_t PALUDIS_ATTRIBUTE((unused)) x = e.stat().file_size(), FSError);
        }
    } test_fs_stat_size;

    struct FSPathSymlink : TestCase
    {
        FSPathSymlink() : TestCase("symlink") {}

        void run()
        {
            FSPath f("fs_stat_TEST_dir/new_sym");
            TEST_CHECK(f.symlink("the_target"));
            TEST_CHECK(f.stat().is_symlink());
            TEST_CHECK_EQUAL(f.readlink(), "the_target");
            f.unlink();
        }
    } test_fs_symlink;

    struct FSPathChangePerms : TestCase
    {
        FSPathChangePerms() : TestCase("chmod, chown and permissions") {}

        void run()
        {
            FSPath a("fs_stat_TEST_dir/no_perms");

            uid_t my_uid = geteuid();
            a.chown(my_uid, -1);
            TEST_CHECK_EQUAL(a.stat().owner(), my_uid);

            mode_t all_perms(S_IRUSR | S_IWUSR | S_IXUSR |
                             S_IRGRP | S_IWGRP | S_IXGRP |
                             S_IROTH | S_IWOTH | S_IXOTH);
            a.chmod(all_perms);

            FSPath b("fs_stat_TEST_dir/no_perms");

            TEST_CHECK_EQUAL(static_cast<mode_t>(b.stat().permissions() & 0xFFF), all_perms);

            mode_t no_perms(0);
            b.chmod(no_perms);

            FSPath c("fs_stat_TEST_dir/no_perms");
            TEST_CHECK_EQUAL(static_cast<mode_t>(c.stat().permissions() & 0xFFF), no_perms);

            FSPath d("fs_stat_TEST_dir/i_dont_exist");

            TEST_CHECK_THROWS(mode_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().permissions(), FSError);
            TEST_CHECK_THROWS(d.chmod(all_perms), FSError);
            TEST_CHECK_THROWS(d.chown(static_cast<uid_t>(-1), -1), FSError);
            TEST_CHECK_THROWS(uid_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().owner(), FSError);
            TEST_CHECK_THROWS(gid_t PALUDIS_ATTRIBUTE((unused)) x = d.stat().group(), FSError);

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

                TEST_CHECK_EQUAL(FSPath(my_file).stat().owner(), new_owner);
                TEST_CHECK_EQUAL(FSPath(my_file).stat().group(), my_group);

                gid_t new_group(pw->pw_gid);

                endpwent();

                e.chown(static_cast<uid_t>(-1), new_group);

                TEST_CHECK_EQUAL(FSPath(my_file).stat().owner(), new_owner);
                TEST_CHECK_EQUAL(FSPath(my_file).stat().group(), new_group);

                e.chown(static_cast<uid_t>(-1), -1);

                TEST_CHECK_EQUAL(FSPath(my_file).stat().owner(), new_owner);
                TEST_CHECK_EQUAL(FSPath(my_file).stat().group(), new_group);

                e.chown(my_owner, my_group);

                TEST_CHECK_EQUAL(FSPath(my_file).stat().owner(), my_owner);
                TEST_CHECK_EQUAL(FSPath(my_file).stat().group(), my_group);
            }
            else
            {
                FSPath e("fs_stat_TEST_dir/all_perms");

                TEST_CHECK_THROWS(e.chown(0, 0), FSError);
            }
        }
    } test_fs_stat_change_perms;
}

