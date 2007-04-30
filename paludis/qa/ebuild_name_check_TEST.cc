/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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

#include <paludis/qa/ebuild_name_check.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct EbuildNameCheckValidTest : TestCase
    {
        EbuildNameCheckValidTest() : TestCase("ebuild name check valid") { }

        void run()
        {
            FSEntry e("ebuild_name_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "valid-cat" / "valid-pkg");
            TEST_CHECK(f1.exists());
            TEST_CHECK(f1.is_directory());

            FSEntry f3(e / "valid-cat" / "valid-pkg" / "valid-pkg-0.ebuild");
            TEST_CHECK(f3.exists());

            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            EbuildNameCheck::identifier()))())(f3));
            TEST_CHECK(r1.empty());

            FSEntry f4(e / "valid-cat" / "valid-pkg" / "ChangeLog");
            TEST_CHECK(f4.exists());

            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            EbuildNameCheck::identifier()))())(f4));
            TEST_CHECK(r2.empty());
        }
    } test_ebuild_name_check_valid;

    struct EbuildNameCheckInvalidTest : TestCase
    {
        EbuildNameCheckInvalidTest() : TestCase("ebuild name check invalid") { }

        void run()
        {
            FSEntry e("ebuild_name_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry f1(e / "valid-cat" / "valid-pkg");
            TEST_CHECK(f1.exists());
            TEST_CHECK(f1.is_directory());

            FSEntry f2(e / "valid-cat" / "valid-pkg" / "invalid-pkg-0.ebuild");
            TEST_CHECK(f2.exists());

            FSEntry f3(e / "valid-cat" / "valid-pkg" / "valid-pkg-not-0.ebuild");
            TEST_CHECK(f3.exists());

            FSEntry f4(e / "valid-cat" / "valid-pkg" / "valid-pkg.ebuild");
            TEST_CHECK(f4.exists());

            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            EbuildNameCheck::identifier()))())(f2));
            TEST_CHECK(! r1.empty());

            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            EbuildNameCheck::identifier()))())(f3));
            TEST_CHECK(! r2.empty());

            CheckResult r3((*(*FileCheckMaker::get_instance()->find_maker(
                            EbuildNameCheck::identifier()))())(f4));
            TEST_CHECK(! r3.empty());
        }
    } test_ebuild_name_check_invalid;

}
