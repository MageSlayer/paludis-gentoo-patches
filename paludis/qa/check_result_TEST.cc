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

#include "check_result.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct CheckResultTest : TestCase
    {
        CheckResultTest() : TestCase("check result") { }

        void run()
        {
            CheckResult r(FSEntry("/"), "blah");
            TEST_CHECK_EQUAL(r.item(), "/");
            TEST_CHECK_EQUAL(r.rule(), "blah");
            TEST_CHECK(r.empty());
            TEST_CHECK_EQUAL(r.most_severe_level(), qal_info);
            r << Message(qal_info, "moo");
            TEST_CHECK(! r.empty());
            TEST_CHECK_EQUAL(r.most_severe_level(), qal_info);
            r << Message(qal_minor, "moo");
            TEST_CHECK_EQUAL(r.most_severe_level(), qal_minor);
            r << Message(qal_major, "moo");
            TEST_CHECK_EQUAL(r.most_severe_level(), qal_major);
            TEST_CHECK_EQUAL(std::distance(r.begin(), r.end()), 3);
        }
    } test_check_result;
}

