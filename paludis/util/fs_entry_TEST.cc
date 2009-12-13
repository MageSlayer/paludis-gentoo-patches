/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
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

/** \file
 * Test cases for fs_entry.hh.
 *
 * \todo this is nowhere near complete.
 *
 * \ingroup grpfilesystem
 */

namespace test_cases
{
    /**
     * \test Test FSEntry ctime and mtime methods
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryTime : TestCase
    {
        FSEntryTime() : TestCase("ctime and mtime") {}

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            FSEntry a("fs_entry_TEST_dir");
            FSEntry b("fs_entry_TEST_dir/no_perms");
            FSEntry c("fs_entry_TEST_dir/no_such_file");
            FSEntry d("fs_entry_TEST_dir/dir_a/dir_in_a");

#if !defined(__FreeBSD__)
            TEST_CHECK(a.ctim() < Timestamp::now());
            TEST_CHECK(a.mtim() < Timestamp::now());
#endif
            TEST_CHECK(b.ctim() < Timestamp::now());
            TEST_CHECK(b.mtim() < Timestamp::now());
            TEST_CHECK(d.ctim() < Timestamp::now());
            TEST_CHECK(d.mtim() < Timestamp::now());

            TEST_CHECK(b.mtim() < b.ctim());
            TEST_CHECK(d.mtim() == d.ctim());

            TEST_CHECK_THROWS(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c.ctim(), FSError);
            TEST_CHECK_THROWS(Timestamp PALUDIS_ATTRIBUTE((unused)) x = c.mtim(), FSError);
        }
    } test_fs_entry_time;

    /**
     * \test Test FSEntry construction and manipulation.
     *
     * \ingroup grpfilesystem
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

            f = FSEntry::cwd();

            TEST_CHECK_EQUAL(f, f / FSEntry("/"));
        }
    } test_fs_entry_manipulation;

    /**
     * \test Test FSEntry behavior.
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryRealpathTest : TestCase
    {
        FSEntryRealpathTest() : TestCase("behavior") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            FSEntry d("fs_entry_TEST_dir");
            TEST_CHECK(d.is_directory());

            d /= "all_perms";
            TEST_CHECK(d.is_regular_file());

            FSEntry e("fs_entry_TEST_dir/nosuchfile");
            TEST_CHECK(! e.is_regular_file());
            d = e;
            TEST_CHECK(! d.is_regular_file());
            TEST_CHECK(! d.exists());
            TEST_CHECK_THROWS(e.readlink(), FSError);

            d = FSEntry("fs_entry_TEST_dir/all_perms");
            TEST_CHECK(! e.is_regular_file());
            TEST_CHECK(d.is_regular_file());
            TEST_CHECK(d.exists());

            d = FSEntry("fs_entry_TEST_dir/symlink_to_dir_a");
            TEST_CHECK(d.is_symbolic_link());
            TEST_CHECK(! d.is_directory());
            TEST_CHECK(d.is_directory_or_symlink_to_directory());
            TEST_CHECK(! d.is_regular_file());
            TEST_CHECK(! d.is_regular_file_or_symlink_to_regular_file());

            e = FSEntry("fs_entry_TEST_dir/doesnotexist_symlink");
            TEST_CHECK(e.is_symbolic_link());
            TEST_CHECK(e.exists());
            TEST_CHECK(! e.is_directory());
            TEST_CHECK(! e.is_directory_or_symlink_to_directory());
            TEST_CHECK(! e.is_regular_file());
            TEST_CHECK(! e.is_regular_file_or_symlink_to_regular_file());

            FSEntry f("fs_entry_TEST_dir/symlink_to_dir_a/file_in_a");
            TEST_CHECK(f.is_regular_file());
            TEST_CHECK(! f.is_symbolic_link());
            FSEntry r(f.realpath());
            TEST_CHECK(r.is_regular_file());
            std::string g("fs_entry_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_EQUAL(stringify(r).substr(stringify(r).length() - g.length()), g);

            FSEntry h("fs_entry_TEST_dir/symlink_to_file_in_a");
            TEST_CHECK(h.is_symbolic_link());
            TEST_CHECK(! h.is_regular_file());
            TEST_CHECK(h.is_regular_file_or_symlink_to_regular_file());
            TEST_CHECK_EQUAL(h.readlink(), "dir_a/file_in_a");

            FSEntry i("fs_entry_TEST_dir/dir_to_make");
            TEST_CHECK(i.mkdir());
            TEST_CHECK(! i.mkdir());
            TEST_CHECK(i.rmdir());
            FSEntry j("fs_entry_TEST_dir/dir_to_make");
            TEST_CHECK(! j.exists());
            TEST_CHECK(! j.is_directory());

            FSEntry k("fs_entry_TEST_dir/dir_a/file_in_a");
            TEST_CHECK_THROWS(k.mkdir(), FSError);

            FSEntry l("fs_entry_TEST_dir/file_a/file_that_triggers_ENOTDIR");
            TEST_CHECK(! l.exists());
        }
    } test_fs_entry_behaviour;

    /**
     * \test Test FSEntry has_permission methods.
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryHasPermission: TestCase
    {
        FSEntryHasPermission() : TestCase("has_permission") {}

        void run()
        {
            FSEntry a("fs_entry_TEST_dir/all_perms");
            FSEntry b("fs_entry_TEST_dir/no_perms");
            FSEntry c("fs_entry_TEST_dir/no_such_file");

            TEST_CHECK(a.has_permission(fs_ug_owner, fs_perm_read));
            TEST_CHECK(a.has_permission(fs_ug_owner, fs_perm_write));
            TEST_CHECK(a.has_permission(fs_ug_owner, fs_perm_execute));
            TEST_CHECK(a.has_permission(fs_ug_group, fs_perm_read));
            TEST_CHECK(a.has_permission(fs_ug_group, fs_perm_write));
            TEST_CHECK(a.has_permission(fs_ug_group, fs_perm_execute));
            TEST_CHECK(a.has_permission(fs_ug_others, fs_perm_read));
            TEST_CHECK(a.has_permission(fs_ug_others, fs_perm_write));
            TEST_CHECK(a.has_permission(fs_ug_others, fs_perm_execute));

            TEST_CHECK(!b.has_permission(fs_ug_owner, fs_perm_read));
            TEST_CHECK(!b.has_permission(fs_ug_owner, fs_perm_write));
            TEST_CHECK(!b.has_permission(fs_ug_owner, fs_perm_execute));
            TEST_CHECK(!b.has_permission(fs_ug_group, fs_perm_read));
            TEST_CHECK(!b.has_permission(fs_ug_group, fs_perm_write));
            TEST_CHECK(!b.has_permission(fs_ug_group, fs_perm_execute));
            TEST_CHECK(!b.has_permission(fs_ug_others, fs_perm_read));
            TEST_CHECK(!b.has_permission(fs_ug_others, fs_perm_write));
            TEST_CHECK(!b.has_permission(fs_ug_others, fs_perm_execute));

            TEST_CHECK_THROWS(bool PALUDIS_ATTRIBUTE((unused)) x =
                    c.has_permission(fs_ug_owner, fs_perm_read), FSError);
        }
    } test_fs_entry_permission;

    /**
     * \test Test FSEntry file_size
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryFileSize : TestCase
    {
        FSEntryFileSize() : TestCase("file size") {}

        void run()
        {
            FSEntry f("fs_entry_TEST_dir/ten_bytes");
            FSEntry d("fs_entry_TEST_dir/dir_a");
            FSEntry e("fs_entry_TEST_dir/no_such_file");

            TEST_CHECK_EQUAL(f.file_size(), 10);
            TEST_CHECK_THROWS(size_t PALUDIS_ATTRIBUTE((unused)) x = d.file_size(), FSError);
            TEST_CHECK_THROWS(size_t PALUDIS_ATTRIBUTE((unused)) x = e.file_size(), FSError);
        }
    } test_fs_entry_size;

    struct FSEntrySymlink : TestCase
    {
        FSEntrySymlink() : TestCase("symlink") {}

        void run()
        {
            FSEntry f("fs_entry_TEST_dir/new_sym");
            TEST_CHECK(f.symlink("the_target"));
            TEST_CHECK(f.is_symbolic_link());
            TEST_CHECK_EQUAL(f.readlink(), "the_target");
            f.unlink();
        }
    } test_fs_symlink;

    /**
     * \test Test FSEntry basename and dirname methods
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryBaseDirName : TestCase
    {
        FSEntryBaseDirName() : TestCase("basename and dirname") {}

        void run()
        {
            FSEntry a("/foo/bar");
            FSEntry b("/moo/went/the/cow");
            FSEntry c("/");
            FSEntry d(".");
            FSEntry e("..");

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
    } test_fs_entry_dir_base_name;

    /**
     * \test Test FSEntry strip_leading method
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryStripLeading : TestCase
    {
        FSEntryStripLeading() : TestCase("strip_leading") {}

        void run()
        {
            FSEntry root1("/stairway/to/heaven/");
            FSEntry root2("");
            FSEntry root3("/");

            FSEntry a(root1);
            FSEntry b(root1 / "usr" / "share");
            FSEntry c(root2 / "my" / "directory");
            FSEntry d(root3 / "my" / "directory");

            TEST_CHECK(stringify(a.strip_leading(root1)) == "/");
            TEST_CHECK(stringify(b.strip_leading(root1)) == "/usr/share");
            TEST_CHECK(stringify(c.strip_leading(root2)) == "/my/directory");
            TEST_CHECK(stringify(d.strip_leading(root3)) == "/my/directory");
        }
    } test_fs_entry_strip_leading;

    /**
     * \test Test FSEntry chmod, chown and permissions methods
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryChangePerms : TestCase
    {
        FSEntryChangePerms() : TestCase("chmod, chown and permissions") {}

        void run()
        {
            FSEntry a("fs_entry_TEST_dir/no_perms");

            uid_t my_uid = geteuid();
            a.chown(my_uid);
            TEST_CHECK_EQUAL(a.owner(), my_uid);

            mode_t all_perms(S_IRUSR | S_IWUSR | S_IXUSR |
                             S_IRGRP | S_IWGRP | S_IXGRP |
                             S_IROTH | S_IWOTH | S_IXOTH);
            a.chmod(all_perms);

            FSEntry b("fs_entry_TEST_dir/no_perms");

            TEST_CHECK_EQUAL(static_cast<mode_t>(b.permissions() & 0xFFF), all_perms);

            mode_t no_perms(0);
            b.chmod(no_perms);

            FSEntry c("fs_entry_TEST_dir/no_perms");
            TEST_CHECK_EQUAL(static_cast<mode_t>(c.permissions() & 0xFFF), no_perms);

            FSEntry d("fs_entry_TEST_dir/i_dont_exist");

            TEST_CHECK_THROWS(mode_t PALUDIS_ATTRIBUTE((unused)) x = d.permissions(), FSError);
            TEST_CHECK_THROWS(d.chmod(all_perms), FSError);
            TEST_CHECK_THROWS(d.chown(static_cast<uid_t>(-1)), FSError);
            TEST_CHECK_THROWS(uid_t PALUDIS_ATTRIBUTE((unused)) x = d.owner(), FSError);
            TEST_CHECK_THROWS(gid_t PALUDIS_ATTRIBUTE((unused)) x = d.group(), FSError);

            if (0 == my_uid)
            {
                struct passwd *pw = getpwent();

                if (! pw)
                    throw InternalError(PALUDIS_HERE, "getpwent returned NULL");

                std::string my_file("fs_entry_TEST_dir/all_perms");
                FSEntry e(my_file);

                uid_t my_owner = e.owner();
                gid_t my_group = e.group();

                if (pw->pw_uid == my_owner)
                {
                    pw = getpwent();

                    if (! pw)
                        throw InternalError(PALUDIS_HERE, "getpwent returned NULL");
                }

                uid_t new_owner(pw->pw_uid);

                e.chown(new_owner);

                TEST_CHECK_EQUAL(FSEntry(my_file).owner(), new_owner);
                TEST_CHECK_EQUAL(FSEntry(my_file).group(), my_group);

                gid_t new_group(pw->pw_gid);

                endpwent();

                e.chown(static_cast<uid_t>(-1), new_group);

                TEST_CHECK_EQUAL(FSEntry(my_file).owner(), new_owner);
                TEST_CHECK_EQUAL(FSEntry(my_file).group(), new_group);

                e.chown(static_cast<uid_t>(-1));

                TEST_CHECK_EQUAL(FSEntry(my_file).owner(), new_owner);
                TEST_CHECK_EQUAL(FSEntry(my_file).group(), new_group);

                e.chown(my_owner, my_group);

                TEST_CHECK_EQUAL(FSEntry(my_file).owner(), my_owner);
                TEST_CHECK_EQUAL(FSEntry(my_file).group(), my_group);
            }
            else
            {
                FSEntry e("fs_entry_TEST_dir/all_perms");

                TEST_CHECK_THROWS(e.chown(0, 0), FSError);
            }
        }
    } test_fs_entry_change_perms;

    /**
     * \test Test operator<<
     *
     * \ingroup grpfilesystem
     */
    struct FSEntryToOstreamOperator : TestCase
    {
        FSEntryToOstreamOperator() : TestCase("operator<<") {}

        void run()
        {
            std::string n("fs_entry_TEST_dir/no_perms");
            std::ostringstream s;
            FSEntry a(n);

            s << a;

            TEST_CHECK_EQUAL(s.str(), n);
        }
    } test_fs_entry_to_ostream_operator;
}

