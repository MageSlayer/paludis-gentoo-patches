/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/dep_lexer.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for DepLexer.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test DepLexer with an empty input.
     *
     * \ingroup Test
     */
    struct DepLexerEmptyTest : TestCase
    {
        DepLexerEmptyTest() : TestCase("empty") { }

        void run()
        {
            DepLexer l("");
            DepLexer::Iterator i(l.begin());
            TEST_CHECK(i == l.end());
        }
    } test_dep_atom_parser_lexer_empty;

    /**
     * \test Test DepLexer with a blank input.
     *
     * \ingroup Test
     */
    struct DepLexerBlankTest : TestCase
    {
        DepLexerBlankTest() : TestCase("blank") { }

        void run()
        {
            DepLexer l("   \n   \t");
            DepLexer::Iterator i(l.begin());
            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_whitespace);
            TEST_CHECK_EQUAL(i->second, "   \n   \t");
            TEST_CHECK(++i == l.end());
        }
    } test_dep_atom_parser_lexer_blank;

    /**
     * \test Test DepLexer with a package.
     *
     * \ingroup Test
     */
    struct DepLexerPackageTest : TestCase
    {
        DepLexerPackageTest() : TestCase("package") { }

        void run()
        {
            DepLexer l("app-editors/vim");
            DepLexer::Iterator i(l.begin());
            TEST_CHECK(i != l.end());
            TEST_CHECK_EQUAL(i->first, dpl_text);
            TEST_CHECK_EQUAL(i->second, "app-editors/vim");
            TEST_CHECK(++i == l.end());
        }
    } test_dep_atom_parser_lexer_package;

    /**
     * \test Test DepParser with a sequence of packages.
     *
     * \ingroup Test
     */
    struct DepLexerPackagesTest : TestCase
    {
        DepLexerPackagesTest() : TestCase("packages") { }

        void run()
        {
            DepLexer l("app-editors/vim app-misc/hilite   \nsys-apps/findutils");
            DepLexer::Iterator i(l.begin());

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
    } test_dep_atom_parser_lexer_packages;

    /**
     * \test Test DepLexer with an any group.
     *
     * \ingroup Test
     */
    struct DepLexerAnyTest : TestCase
    {
        DepLexerAnyTest() : TestCase("any") { }

        void run()
        {
            DepLexer l("|| ( one/one two/two )");
            DepLexer::Iterator i(l.begin());

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
    } test_dep_atom_parser_lexer_any;

    /**
     * \test Test DepLexer with a use group.
     *
     * \ingroup Test
     */
    struct DepLexerUseTest : TestCase
    {
        DepLexerUseTest() : TestCase("use") { }

        void run()
        {
            DepLexer l("foo? ( one/one )");
            DepLexer::Iterator i(l.begin());

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
    } test_dep_atom_parser_lexer_use;

    /**
     * \test Test DepLexer with bad input.
     *
     * \ingroup Test
     */
    struct DepLexerBadTest : TestCase
    {
        DepLexerBadTest() : TestCase("bad") { }

        void run()
        {
            TEST_CHECK_THROWS(DepLexer("(moo)"), DepStringError);
            TEST_CHECK_THROWS(DepLexer("|foo"), DepStringError);
            TEST_CHECK_THROWS(DepLexer("( moo )bar"), DepStringError);
        }
    } test_dep_atom_parser_lexer_bad;

}


