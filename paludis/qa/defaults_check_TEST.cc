/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "check_result.hh"
#include "defaults_check.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct DefaultsCheckSrcUnpackTest : TestCase
    {
        DefaultsCheckSrcUnpackTest() : TestCase("src_unpack") { }

        void run()
        {
            FSEntry f1(FSEntry::cwd() / "defaults_check_TEST_dir/repo1/cat-one/pkg-one/pkg-one-1.ebuild");
            TEST_CHECK(f1.is_regular_file());
            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            DefaultsCheck::identifier()))())(f1));
            {
                TestMessageSuffix suffix("r1=" + r1.item() + ": " + join(r1.begin(), r1.end(), "; "), false);
                TEST_CHECK(r1.empty());
            }

            FSEntry f2(FSEntry::cwd() / "defaults_check_TEST_dir/repo1/cat-one/pkg-one/pkg-one-2.ebuild");
            TEST_CHECK(f2.is_regular_file());
            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            DefaultsCheck::identifier()))())(f2));
            {
                TestMessageSuffix suffix("r2=" + r2.item() + ": " + join(r2.begin(), r2.end(), "; "), false);
                TEST_CHECK(! r2.empty());
            }

            FSEntry f3(FSEntry::cwd() / "defaults_check_TEST_dir/repo1/cat-one/pkg-one/pkg-one-3.ebuild");
            TEST_CHECK(f3.is_regular_file());
            CheckResult r3((*(*FileCheckMaker::get_instance()->find_maker(
                            DefaultsCheck::identifier()))())(f3));
            {
                TestMessageSuffix suffix("r3=" + r2.item() + ": " + join(r3.begin(), r3.end(), "; "), false);
                TEST_CHECK(r3.empty());
            }
        }
    } qa_environment_defaults_check_src_unpack_test;
}


