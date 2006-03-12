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

#include <paludis/util/match_sequence.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_case
{
    struct MatchRuleTest : TestCase
    {
        MatchRuleTest() : TestCase("match rule") { }

        void run()
        {
            typedef MatchRule X;

            TEST_CHECK(X::eol().match(""));
            TEST_CHECK(! X::eol().match("a"));

            X r1(*X("\t") >> X("econf") >> *X(" ") >> (X::eol() || (X("||") >> *X(" ") >> X("die"))));

            TEST_CHECK(r1.match("econf"));
            TEST_CHECK(r1.match("\teconf"));
            TEST_CHECK(r1.match("\t\teconf"));
            TEST_CHECK(r1.match("\t\teconf  "));
            TEST_CHECK(! r1.match(" econf"));
            TEST_CHECK(! r1.match("\tecong"));
            TEST_CHECK(! r1.match("\t"));
            TEST_CHECK(! r1.match("econfmoo"));
            TEST_CHECK(! r1.match("econf moo"));

            TEST_CHECK(r1.match("\teconf || die"));
            TEST_CHECK(r1.match("\teconf || die \"moo\""));
            TEST_CHECK(! r1.match("\teconf ||"));
            TEST_CHECK(r1.match("\teconf   ||   die"));
            TEST_CHECK(! r1.match("\texonf   ||   die"));
        }
    } test_match_rule;
}

