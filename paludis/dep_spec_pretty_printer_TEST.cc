/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "dep_spec_pretty_printer.hh"
#include "portage_dep_parser.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/eapi.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct PrettyPrinterNoIndentTest : TestCase
    {
        PrettyPrinterNoIndentTest() : TestCase("pretty printer no indent") { }

        void run()
        {
            DepSpecPrettyPrinter p1(0, false);
            PortageDepParser::parse_depend("foo/bar bar/baz", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p1);
            TEST_CHECK_STRINGIFY_EQUAL(p1, "foo/bar bar/baz");

            DepSpecPrettyPrinter p2(0, false);
            PortageDepParser::parse_depend("foo/bar moo? ( bar/baz )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p2);
            TEST_CHECK_STRINGIFY_EQUAL(p2, "foo/bar moo? ( bar/baz )");

            DepSpecPrettyPrinter p3(0, false);
            PortageDepParser::parse_depend("|| ( a/b ( c/d e/f ) )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p3);
            TEST_CHECK_STRINGIFY_EQUAL(p3, "|| ( a/b ( c/d e/f ) )");

            DepSpecPrettyPrinter p4(0, false);
            PortageDepParser::parse_license("( ( ( ) a ) b )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p4);
            TEST_CHECK_STRINGIFY_EQUAL(p4, "a b");

            DepSpecPrettyPrinter p5(0, false);
            PortageDepParser::parse_uri("( a -> b c x? ( d e ) )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p5);
            TEST_CHECK_STRINGIFY_EQUAL(p5, "a -> b c x? ( d e )");
        }
    } test_pretty_printer_no_indent;

    struct PrettyPrinterIndentTest : TestCase
    {
        PrettyPrinterIndentTest() : TestCase("pretty printer indent") { }

        void run()
        {
            DepSpecPrettyPrinter p1(4);
            PortageDepParser::parse_depend("foo/bar bar/baz", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p1);
            TEST_CHECK_STRINGIFY_EQUAL(p1, "    foo/bar\n    bar/baz\n");

            DepSpecPrettyPrinter p2(4);
            PortageDepParser::parse_depend("foo/bar moo? ( bar/baz )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p2);
            TEST_CHECK_STRINGIFY_EQUAL(p2, "    foo/bar\n    moo? (\n        bar/baz\n    )\n");

            DepSpecPrettyPrinter p3(4);
            PortageDepParser::parse_depend("|| ( a/b ( c/d e/f ) )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p3);
            TEST_CHECK_STRINGIFY_EQUAL(p3, "    || (\n        a/b\n        (\n            c/d\n"
                    "            e/f\n        )\n    )\n");

            DepSpecPrettyPrinter p4(4);
            PortageDepParser::parse_license("( ( ( ) a ) b )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(p4);
            TEST_CHECK_STRINGIFY_EQUAL(p4, "    a\n    b\n");
        }
    } test_pretty_printer_indent;
}

