/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/qa/has_misc_files_check.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct HasMiscFilesCheckTest : TestCase
    {
        HasMiscFilesCheckTest() : TestCase("has misc files") { }

        void run()
        {
            FSEntry e("has_misc_files_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "yes");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*PackageDirCheckMaker::get_instance()->find_maker(
                            HasMiscFilesCheck::identifier()))())(f1));
            TEST_CHECK(r1.empty());
        }
    } test_has_misc_files_check;

    struct HasMiscFilesCheckNoChangeLogTest : TestCase
    {
        HasMiscFilesCheckNoChangeLogTest() : TestCase("has misc files no ChangeLog") { }

        void run()
        {
            FSEntry e("has_misc_files_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "no-changelog");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*PackageDirCheckMaker::get_instance()->find_maker(
                            HasMiscFilesCheck::identifier()))())(f1));
            TEST_CHECK(! r1.empty());
        }
    } test_has_misc_files_check_no_changelog;

    struct HasMiscFilesCheckNoMetadataTest : TestCase
    {
        HasMiscFilesCheckNoMetadataTest() : TestCase("has misc files no metadata.xml") { }

        void run()
        {
            FSEntry e("has_misc_files_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "no-metadata");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*PackageDirCheckMaker::get_instance()->find_maker(
                            HasMiscFilesCheck::identifier()))())(f1));
            TEST_CHECK(! r1.empty());
        }
    } test_has_misc_files_check_no_metadata;

    struct HasMiscFilesCheckNoFilesTest : TestCase
    {
        HasMiscFilesCheckNoFilesTest() : TestCase("has misc files no files/") { }

        void run()
        {
            FSEntry e("has_misc_files_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "cat" / "no-files");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*PackageDirCheckMaker::get_instance()->find_maker(
                            HasMiscFilesCheck::identifier()))())(f1));
            TEST_CHECK(! r1.empty());
        }
    } test_has_misc_files_check_no_files;
}

