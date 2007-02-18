/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda <ferdy@gentoo.org>
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
#include "function_check.hh"
#include <paludis/util/fs_entry.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct FunctionCheckTest : TestCase
    {
        FunctionCheckTest() : TestCase("function") { }

        void run()
        {
            FSEntry d("function_check_TEST_dir");
            TEST_CHECK(d.is_directory());

            FSEntry w1(d / "eclass/with1.eclass");
            TEST_CHECK(w1.is_regular_file());
            CheckResult r1((*(*FileCheckMaker::get_instance()->find_maker(
                            FunctionCheck::identifier()))())(w1));
            TEST_CHECK(! r1.empty());

            FSEntry w2(d / "eclass/with2.eclass");
            TEST_CHECK(w2.is_regular_file());
            CheckResult r2((*(*FileCheckMaker::get_instance()->find_maker(
                            FunctionCheck::identifier()))())(w2));
            TEST_CHECK(! r2.empty());

            FSEntry w3(d / "eclass/with3.eclass");
            TEST_CHECK(w3.is_regular_file());
            CheckResult r3((*(*FileCheckMaker::get_instance()->find_maker(
                            FunctionCheck::identifier()))())(w3));
            TEST_CHECK(! r3.empty());


            FSEntry wo(d / "eclass/without.eclass");
            TEST_CHECK(wo.is_regular_file());
            CheckResult r4((*(*FileCheckMaker::get_instance()->find_maker(
                            FunctionCheck::identifier()))())(wo));
            TEST_CHECK(r4.empty());
        }
    } function_check_test;
}
