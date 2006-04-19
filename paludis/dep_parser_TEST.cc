/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/paludis.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for DepParser.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test DepParser with an empty input.
     *
     * \ingroup grptestcases
     */
    struct DepParserEmptyTest : TestCase
    {
        DepParserEmptyTest() : TestCase("empty") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all></all>");
        }
    } test_dep_atom_parser_empty;

    /**
     * \test Test DepParser with a blank input.
     *
     * \ingroup grptestcases
     */
    struct DepParserBlankTest : TestCase
    {
        DepParserBlankTest() : TestCase("blank") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("   \n   \t")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all></all>");
        }
    } test_dep_atom_parser_blank;

    /**
     * \test Test DepParser with a package.
     *
     * \ingroup grptestcases
     */
    struct DepParserPackageTest : TestCase
    {
        DepParserPackageTest() : TestCase("package") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("app-editors/vim")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><package>app-editors/vim</package></all>");
        }
    } test_dep_atom_parser_package;

    /**
     * \test Test DepParser with a decorated package.
     *
     * \ingroup grptestcases
     */
    struct DepParserDecoratedPackageTest : TestCase
    {
        DepParserDecoratedPackageTest() : TestCase("decorated package") { }

        void run()
        {
            std::stringstream s1;
            DepAtomDumper d1(&s1);
            DepParser::parse(">=app-editors/vim-6.4_alpha")->accept(&d1);
            TEST_CHECK_EQUAL(s1.str(), "<all><package version=\">=6.4_alpha\">"
                    "app-editors/vim</package></all>");

            std::stringstream s2;
            DepAtomDumper d2(&s2);
            DepParser::parse("=app-editors/vim-6.4_alpha-r1")->accept(&d2);
            TEST_CHECK_EQUAL(s2.str(), "<all><package version=\"=6.4_alpha-r1\">"
                    "app-editors/vim</package></all>");

            std::stringstream s3;
            DepAtomDumper d3(&s3);
            DepParser::parse(">=app-editors/vim-6.4_alpha:one")->accept(&d3);
            TEST_CHECK_EQUAL(s3.str(), "<all><package slot=\"one\" version=\">=6.4_alpha\">"
                    "app-editors/vim</package></all>");
        }
    } test_dep_atom_parser_decorated_package;

    /**
     * \test Test DepParser with a sequence of packages.
     *
     * \ingroup grptestcases
     */
    struct DepParserPackagesTest : TestCase
    {
        DepParserPackagesTest() : TestCase("packages") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("app-editors/vim app-misc/hilite   \nsys-apps/findutils")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><package>app-editors/vim</package>"
                    "<package>app-misc/hilite</package><package>sys-apps/findutils</package></all>");
        }
    } test_dep_atom_parser_packages;

    /**
     * \test Test DepParser with an any group.
     *
     * \ingroup grptestcases
     */
    struct DepParserAnyTest : TestCase
    {
        DepParserAnyTest() : TestCase("any") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("|| ( one/one two/two )")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><any><package>one/one</package>"
                    "<package>two/two</package></any></all>");
        }
    } test_dep_atom_parser_any;

    /**
     * \test Test DepParser with an all group.
     *
     * \ingroup grptestcases
     */
    struct DepParserAllTest : TestCase
    {
        DepParserAllTest() : TestCase("all") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse(" ( one/one two/two )    ")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><all><package>one/one</package>"
                    "<package>two/two</package></all></all>");
        }
    } test_dep_atom_parser_all;

    /**
     * \test Test DepParser with a use group.
     *
     * \ingroup grptestcases
     */
    struct DepParserUseTest : TestCase
    {
        DepParserUseTest() : TestCase("use") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("foo? ( one/one )")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><use flag=\"foo\" inverse=\"false\"><package>one/one</package>"
                    "</use></all>");
        }
    } test_dep_atom_parser_use;

    /**
     * \test Test DepParser with an inverse use group.
     *
     * \ingroup grptestcases
     */
    struct DepParserInvUseTest : TestCase
    {
        DepParserInvUseTest() : TestCase("!use") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            DepParser::parse("!foo? ( one/one )")->accept(&d);
            TEST_CHECK_EQUAL(s.str(), "<all><use flag=\"foo\" inverse=\"true\"><package>one/one</package>"
                    "</use></all>");
        }
    } test_dep_atom_parser_inv_use;

    /**
     * \test Test DepParser nesting errors.
     *
     * \ingroup grptestcases
     */
    struct DepParserBadNestingTest : TestCase
    {
        DepParserBadNestingTest() : TestCase("bad nesting") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            TEST_CHECK_THROWS(DepParser::parse("!foo? ( one/one")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse("!foo? ( one/one ) )")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse("( ( ( ) )")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse("( ( ( ) ) ) )")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse(")")->accept(&d), DepStringError);
        }
    } test_dep_atom_parser_bad_nesting;

    /**
     * \test Test DepParser weird errors.
     *
     * \ingroup grptestcases
     */
    struct DepParserBadValuesTest : TestCase
    {
        DepParserBadValuesTest() : TestCase("bad values") { }

        void run()
        {
            std::stringstream s;
            DepAtomDumper d(&s);
            TEST_CHECK_THROWS(DepParser::parse("!foo? ||")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse("(((")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse(")")->accept(&d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse("(foo/bar)")->accept(&d), DepStringError);
        }
    } test_dep_atom_parser_bad_values;
}

