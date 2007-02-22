/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/portage_dep_lexer.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageDepLexer.
 *
 */

namespace test_cases
{
    /**
     * \test Test PortageDepLexer with an empty input.
     *
     */
    struct PortageDepLexerEmptyTest : TestCase
    {
        PortageDepLexerEmptyTest() : TestCase("empty") { }

        void run()
        {
            PortageDepLexer l("");
            PortageDepLexer::Iterator i(l.begin());
            TEST_CHECK(i == l.end());
        }
    } test_dep_spec_parser_lexer_empty;

    /**
     * \test Test PortageDepLexer with a blank input.
     *
     */
    struct PortageDepLexerBlankTest : TestCase
    {
        PortageDepLexerBlankTest() : TestCase("blank") { }

        void run()
        {
            PortageDepLexer l("   \n   \t");
            PortageDepLexer::Iterator i(l.begin());
            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, "   \n   \t");
            TEST_CHECK(++i == l.end());
        }
    } test_dep_spec_parser_lexer_blank;

    /**
     * \test Test PortageDepLexer with a package.
     *
     */
    struct PortageDepLexerPackageTest : TestCase
    {
        PortageDepLexerPackageTest() : TestCase("package") { }

        void run()
        {
            PortageDepLexer l("app-editors/vim");
            PortageDepLexer::Iterator i(l.begin());
            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "app-editors/vim");
            TEST_CHECK(++i == l.end());
        }
    } test_dep_spec_parser_lexer_package;

    /**
     * \test Test DepParser with a sequence of packages.
     *
     */
    struct PortageDepLexerPackagesTest : TestCase
    {
        PortageDepLexerPackagesTest() : TestCase("packages") { }

        void run()
        {
            PortageDepLexer l("app-editors/vim app-misc/hilite   \nsys-apps/findutils");
            PortageDepLexer::Iterator i(l.begin());

            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "app-editors/vim");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "app-misc/hilite");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, "   \n");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "sys-apps/findutils");

            TEST_CHECK(++i == l.end());
        }
    } test_dep_spec_parser_lexer_packages;

    /**
     * \test Test PortageDepLexer with an any group.
     *
     */
    struct PortageDepLexerAnyTest : TestCase
    {
        PortageDepLexerAnyTest() : TestCase("any") { }

        void run()
        {
            PortageDepLexer l("|| ( one/one two/two )");
            PortageDepLexer::Iterator i(l.begin());

            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_double_bar);
            TEST_CHECK_EQUAL(i->second, "||");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_open_paren);
            TEST_CHECK_EQUAL(i->second, "(");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "one/one");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "two/two");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_close_paren);
            TEST_CHECK_EQUAL(i->second, ")");

            TEST_CHECK(++i == l.end());
        }
    } test_dep_spec_parser_lexer_any;

    /**
     * \test Test PortageDepLexer with a use group.
     *
     */
    struct PortageDepLexerUseTest : TestCase
    {
        PortageDepLexerUseTest() : TestCase("use") { }

        void run()
        {
            PortageDepLexer l("foo? ( one/one )");
            PortageDepLexer::Iterator i(l.begin());

            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_use_flag);
            TEST_CHECK_EQUAL(i->second, "foo?");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_open_paren);
            TEST_CHECK_EQUAL(i->second, "(");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "one/one");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, " ");

            TEST_CHECK(++i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_close_paren);
            TEST_CHECK_EQUAL(i->second, ")");

            TEST_CHECK(++i == l.end());
        }
    } test_dep_spec_parser_lexer_use;

    /**
     * \test Test PortageDepLexer with bad input.
     *
     */
    struct PortageDepLexerBadTest : TestCase
    {
        PortageDepLexerBadTest() : TestCase("bad") { }

        void run()
        {
            TEST_CHECK_THROWS(PortageDepLexer("(moo)"), DepStringError);
            TEST_CHECK_THROWS(PortageDepLexer("|foo"), DepStringError);
            TEST_CHECK_THROWS(PortageDepLexer("( moo )bar"), DepStringError);
        }
    } test_dep_spec_parser_lexer_bad;

}


