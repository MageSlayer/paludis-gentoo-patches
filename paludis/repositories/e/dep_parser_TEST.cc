/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/visitor-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;
using namespace paludis::erepository;

/** \file
 * Test cases for DepParser.
 *
 */

namespace test_cases
{
    /**
     * \test Test DepParser with an empty input.
     *
     */
    struct DepParserEmptyTest : TestCase
    {
        DepParserEmptyTest() : TestCase("empty") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_empty;

    /**
     * \test Test DepParser with a blank input.
     *
     */
    struct DepParserBlankTest : TestCase
    {
        DepParserBlankTest() : TestCase("blank") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("   \n   \t",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_blank;

    /**
     * \test Test DepParser with a package.
     *
     */
    struct DepParserPackageTest : TestCase
    {
        DepParserPackageTest() : TestCase("package") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("app-editors/vim",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim");
        }
    } test_dep_spec_parser_package;

    /**
     * \test Test DepParser with a decorated package.
     *
     */
    struct DepParserDecoratedPackageTest : TestCase
    {
        DepParserDecoratedPackageTest() : TestCase("decorated package") { }

        void run()
        {
            DepSpecPrettyPrinter d1(0, false);
            DepParser::parse_depend(">=app-editors/vim-6.4_alpha",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d1);
            TEST_CHECK_EQUAL(stringify(d1), ">=app-editors/vim-6.4_alpha");

            DepSpecPrettyPrinter d2(0, false);
            DepParser::parse_depend("=app-editors/vim-6.4_alpha-r1",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d2);
            TEST_CHECK_EQUAL(stringify(d2), "=app-editors/vim-6.4_alpha-r1");

            DepSpecPrettyPrinter d3(0, false);
            DepParser::parse_depend(">=app-editors/vim-6.4_alpha:one",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d3);
            TEST_CHECK_EQUAL(stringify(d3), ">=app-editors/vim-6.4_alpha:one");
        }
    } test_dep_spec_parser_decorated_package;

    /**
     * \test Test DepParser with a sequence of packages.
     *
     */
    struct DepParserPackagesTest : TestCase
    {
        DepParserPackagesTest() : TestCase("packages") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("app-editors/vim app-misc/hilite   \nsys-apps/findutils",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim app-misc/hilite sys-apps/findutils");
        }
    } test_dep_spec_parser_packages;

    struct DepParserAnyTest : TestCase
    {
        DepParserAnyTest() : TestCase("any") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("|| ( one/one two/two )",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one two/two )");
        }
    } test_dep_spec_parser_any;

    struct DepParserAnyUseTest : TestCase
    {
        DepParserAnyUseTest() : TestCase("any use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("|| ( one/one foo? ( two/two ) )",
                    *EAPIData::get_instance()->eapi_from_string("0"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one foo? ( two/two ) )");

            TEST_CHECK_THROWS(DepParser::parse_depend("|| ( one/one foo? ( two/two ) )",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);

            DepSpecPrettyPrinter e(0, false);
            DepParser::parse_depend("|| ( one/one ( foo? ( two/two ) ) )",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "|| ( one/one ( foo? ( two/two ) ) )");
        }
    } test_dep_spec_parser_any_use;

    /**
     * \test Test DepParser with an all group.
     *
     */
    struct DepParserAllTest : TestCase
    {
        DepParserAllTest() : TestCase("all") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend(" ( one/one two/two )    ",
                    *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "one/one two/two");
        }
    } test_dep_spec_parser_all;

    /**
     * \test Test DepParser with a use group.
     *
     */
    struct DepParserUseTest : TestCase
    {
        DepParserUseTest() : TestCase("use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("foo? ( one/one )", *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "foo? ( one/one )");
        }
    } test_dep_spec_parser_use;

    /**
     * \test Test DepParser with an inverse use group.
     *
     */
    struct DepParserInvUseTest : TestCase
    {
        DepParserInvUseTest() : TestCase("!use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            DepParser::parse_depend("!foo? ( one/one )", *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "!foo? ( one/one )");
        }
    } test_dep_spec_parser_inv_use;

    struct DepParserURITest : TestCase
    {
        DepParserURITest() : TestCase("uri") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, true);
            DepParser::parse_uri("a\n->\tb", *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "a -> b\n");

            DepSpecPrettyPrinter e(0, true);
            DepParser::parse_uri("a-> b", *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "a->\nb\n");

            TEST_CHECK_THROWS(DepParser::parse_uri("a -> b",
                        *EAPIData::get_instance()->eapi_from_string("0"))->accept(d), DepStringError);
        }
    } test_dep_spec_parser_uri;

    /**
     * \test Test DepParser nesting errors.
     *
     */
    struct DepParserBadNestingTest : TestCase
    {
        DepParserBadNestingTest() : TestCase("bad nesting") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            TEST_CHECK_THROWS(DepParser::parse_depend("!foo? ( one/one",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("!foo? ( one/one ) )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("( ( ( ) )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("( ( ( ) ) ) )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend(")",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
        }
    } test_dep_spec_parser_bad_nesting;

    /**
     * \test Test DepParser weird errors.
     *
     */
    struct DepParserBadValuesTest : TestCase
    {
        DepParserBadValuesTest() : TestCase("bad values") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            TEST_CHECK_THROWS(DepParser::parse_depend("||",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("|| ",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("foo?",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("!foo? ||",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("(((",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend(")",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_depend("(foo/bar)",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_license("a -> b",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);

            TEST_CHECK_THROWS(DepParser::parse_uri("( -> )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("( -> )",
                        *EAPIData::get_instance()->eapi_from_string("0"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("foo? -> bar",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("a ->",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("a -> ( )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("a -> )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("a -> || ( )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(DepParser::parse_uri("a -> foo? ( )",
                        *EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
        }
    } test_dep_spec_parser_bad_values;
}

