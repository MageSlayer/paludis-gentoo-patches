/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/repositories/unpackaged/dep_printer.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/util/visitor-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::unpackaged_repositories;
using namespace test;

namespace test_cases
{
    struct DepParserTest : TestCase
    {
        DepParserTest() : TestCase("dep parser") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<const DependencySpecTree::ConstItem> spec(
                    DepParser::parse(&env, "cat/one  , cat/two, cat/three\n"));
            StringifyFormatter f;
            DepPrinter p(&env, f, true);
            spec->accept(p);
            TEST_CHECK_EQUAL(p.result(), "cat/one, cat/two, cat/three");
        }
    } test_dep_parser;
}

