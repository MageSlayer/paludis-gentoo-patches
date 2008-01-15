/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006
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

#include <paludis/repositories/e/qa/misc_files.hh>
#include <paludis/qa.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::erepository;
using namespace test;

namespace
{
    struct TestReporter :
        QAReporter
    {
        unsigned count;

        TestReporter() :
            count(0)
        {
        }

        void message(const QAMessage &)
        {
            ++count;
        }

        void status(const std::string &)
        {
        }
    };
}

namespace test_cases
{
    struct HasMiscFilesCheckTest : TestCase
    {
        HasMiscFilesCheckTest() : TestCase("has misc files") { }

        void run()
        {
            FSEntry e("misc_files_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "yes");
            TEST_CHECK(f1.exists());

            TestReporter r;
            TEST_CHECK(misc_files_check(r, f1, "misc_files"));
            TEST_CHECK_EQUAL(r.count, 0u);
        }
    } test_has_misc_files_check;

    struct HasMiscFilesCheckNoChangeLogTest : TestCase
    {
        HasMiscFilesCheckNoChangeLogTest() : TestCase("has misc files no ChangeLog") { }

        void run()
        {
            FSEntry e("misc_files_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "no-changelog");
            TEST_CHECK(f1.exists());

            TestReporter r;
            TEST_CHECK(misc_files_check(r, f1, "misc_files"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_has_misc_files_check_no_changelog;

    struct HasMiscFilesCheckNoMetadataTest : TestCase
    {
        HasMiscFilesCheckNoMetadataTest() : TestCase("has misc files no metadata.xml") { }

        void run()
        {
            FSEntry e("misc_files_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "no-metadata");
            TEST_CHECK(f1.exists());

            TestReporter r;
            TEST_CHECK(misc_files_check(r, f1, "misc_files"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_has_misc_files_check_no_metadata;

    struct HasMiscFilesCheckBadFilesTest : TestCase
    {
        HasMiscFilesCheckBadFilesTest() : TestCase("has misc files bad files/") { }

        void run()
        {
            FSEntry e("misc_files_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "bad-files");
            TEST_CHECK(f1.exists());

            TestReporter r;
            TEST_CHECK(misc_files_check(r, f1, "misc_files"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_has_misc_files_check_bad_files;
}

