/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/file_permissions_check.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct FilePermissionsCheckFileTest : TestCase
    {
        FilePermissionsCheckFileTest() : TestCase("file permissions file") { }

        void run()
        {
            FSEntry e("file_permissions_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "ok_file");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f1));
            TEST_CHECK(r1.empty());

            FSEntry f2(e / "no_read_file");
            TEST_CHECK(f2.exists());
            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f2));
            TEST_CHECK(! r2.empty());

            FSEntry f3(e / "exec_file");
            TEST_CHECK(f3.exists());
            CheckResult r3((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f3));
            TEST_CHECK(! r3.empty());
        }
    } test_file_permissions_check_file;

    struct FilePermissionsCheckDirTest : TestCase
    {
        FilePermissionsCheckDirTest() : TestCase("file permissions dir") { }

        void run()
        {
            FSEntry e("file_permissions_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "ok_dir");
            TEST_CHECK(f1.exists());
            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f1));
            TEST_CHECK(r1.empty());

            FSEntry f2(e / "no_read_dir");
            TEST_CHECK(f2.exists());
            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f2));
            TEST_CHECK(! r2.empty());

            FSEntry f3(e / "no_exec_dir");
            TEST_CHECK(f3.exists());
            CheckResult r3((*(*FileCheckMaker::get_instance()->find_maker(
                            FilePermissionsCheck::identifier()))())(f3));
            TEST_CHECK(! r3.empty());
        }
    } test_file_permissions_check_dir;
}

