/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/fake/dep_parser.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/visitor-impl.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace test;

namespace
{
    struct QuickPrinter :
        ConstVisitor<DependencySpecTree>
    {
        std::stringstream str;

        void visit_leaf(const PackageDepSpec & s)
        {
            str << "p<" << stringify(s) << ">";
        }

        void visit_leaf(const NamedSetDepSpec & s)
        {
            str << "s<" << stringify(s) << ">";
        }

        void visit_leaf(const BlockDepSpec & s)
        {
            str << "b<" << stringify(s) << ">";
        }

        void visit_leaf(const DependencyLabelsDepSpec & s)
        {
            str << "l<" << stringify(s) << ">";
        }

        void visit_sequence(const AllDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            str << "all<";
            std::for_each(cur, end, accept_visitor(*this));
            str << ">";
        }

        void visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            str << "any<";
            std::for_each(cur, end, accept_visitor(*this));
            str << ">";
        }

        void visit_sequence(const ConditionalDepSpec & s,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            str << "cond<" << s << ",";
            std::for_each(cur, end, accept_visitor(*this));
            str << ">";
        }
    };
}

namespace test_cases
{
    struct DepParserTest : TestCase
    {
        DepParserTest() : TestCase("dep parser") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<DependencySpecTree::ConstItem> d(fakerepository::parse_depend(
                        "( ( a/a b/b ) )", &env, std::tr1::shared_ptr<const PackageID>()));

            QuickPrinter p;
            d->accept(p);
            TEST_CHECK_EQUAL(p.str.str(), "all<all<all<p<a/a>p<b/b>>>>");
        }
    } dep_parser_test;
}

