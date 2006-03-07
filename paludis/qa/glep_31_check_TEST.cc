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

#include <paludis/qa/glep_31_check.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct Utf8Test : TestCase
    {
        Utf8Test() : TestCase("utf8") { }

        void check_valid(const std::string & s)
        {
            std::stringstream ss(s);
            CheckResult r("glep_31_check_TEST.cc", "test");
            TEST_CHECK(r.empty());
            Glep31Check::check_utf8(ss, r);
            TEST_CHECK(r.empty());
        }

        void check_invalid(const std::string & s)
        {
            std::stringstream ss(s);
            CheckResult r("glep_31_check_TEST.cc", "test");
            TEST_CHECK(r.empty());
            Glep31Check::check_utf8(ss, r);
            TEST_CHECK(! r.empty());
        }

        void run()
        {
            check_valid("");
            check_valid("abcde");
            check_valid("abc""\xc2""\xa3""de");
            check_valid("abc""\xd7""\x90""de");
            check_valid("abc""\xe2""\x82""\xac""de");
            check_valid("abc""\xf0""\xa1""\xa1""\xa1""de");

            check_invalid("abc""\xff""de");
            check_invalid("abc""\xc2""\x2a""de");
            check_invalid("abc""\xe1""\x2a""\x2a""de");
            check_invalid("abc""\xe1""\xaa""\x2a""de");
            check_invalid("abc""\xf0""\x2a""\x2a""\x2a""de");
            check_invalid("abc""\xf0""\xaa""\x2a""\x2a""de");
            check_invalid("abc""\xf0""\xa1""\xa1""\x2a""de");

            check_invalid("abc""\xc2");
            check_invalid("abc""\xd7");
            check_invalid("abc""\xe2""\x82");
            check_invalid("abc""\xf0""\xa1""\xa1");
        }
    } test_utf8;
}

