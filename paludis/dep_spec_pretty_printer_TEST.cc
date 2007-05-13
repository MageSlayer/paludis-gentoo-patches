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
            std::tr1::shared_ptr<const DepSpec> s1(PortageDepParser::parse("foo/bar bar/baz",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p1(0, false);
            s1->accept(&p1);
            TEST_CHECK_STRINGIFY_EQUAL(p1, "foo/bar bar/baz");

            std::tr1::shared_ptr<const DepSpec> s2(PortageDepParser::parse("foo/bar moo? ( bar/baz )",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p2(0, false);
            s2->accept(&p2);
            TEST_CHECK_STRINGIFY_EQUAL(p2, "foo/bar moo? ( bar/baz )");

            std::tr1::shared_ptr<const DepSpec> s3(PortageDepParser::parse("|| ( a/b ( c/d e/f ) )",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p3(0, false);
            s3->accept(&p3);
            TEST_CHECK_STRINGIFY_EQUAL(p3, "|| ( a/b ( c/d e/f ) )");

            std::tr1::shared_ptr<const DepSpec> s4(PortageDepParser::parse("( ( ( ) a ) b )",
                        PortageDepParser::Policy::text_is_text_dep_spec(true)));
            DepSpecPrettyPrinter p4(0, false);
            s4->accept(&p4);
            TEST_CHECK_STRINGIFY_EQUAL(p4, "a b");
        }
    } test_pretty_printer_no_indent;

    struct PrettyPrinterIndentTest : TestCase
    {
        PrettyPrinterIndentTest() : TestCase("pretty printer indent") { }

        void run()
        {
            std::tr1::shared_ptr<const DepSpec> s1(PortageDepParser::parse("foo/bar bar/baz",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p1(4);
            s1->accept(&p1);
            TEST_CHECK_STRINGIFY_EQUAL(p1, "    foo/bar\n    bar/baz\n");

            std::tr1::shared_ptr<const DepSpec> s2(PortageDepParser::parse("foo/bar moo? ( bar/baz )",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p2(4);
            s2->accept(&p2);
            TEST_CHECK_STRINGIFY_EQUAL(p2, "    foo/bar\n    moo? (\n        bar/baz\n    )\n");

            std::tr1::shared_ptr<const DepSpec> s3(PortageDepParser::parse("|| ( a/b ( c/d e/f ) )",
                        PortageDepParser::Policy::text_is_package_dep_spec(true, pds_pm_permissive)));
            DepSpecPrettyPrinter p3(4);
            s3->accept(&p3);
            TEST_CHECK_STRINGIFY_EQUAL(p3, "    || (\n        a/b\n        (\n            c/d\n"
                    "            e/f\n        )\n    )\n");

            std::tr1::shared_ptr<const DepSpec> s4(PortageDepParser::parse("( ( ( ) a ) b )",
                        PortageDepParser::Policy::text_is_text_dep_spec(true)));
            DepSpecPrettyPrinter p4(4);
            s4->accept(&p4);
            TEST_CHECK_STRINGIFY_EQUAL(p4, "    a\n    b\n");
        }
    } test_pretty_printer_indent;
}

