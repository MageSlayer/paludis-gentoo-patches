/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <sstream>
#include <algorithm>

using namespace paludis;
using namespace test;

namespace
{
    struct QuickPrinter
    {
        std::stringstream str;

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            str << "p<" << stringify(*node.spec()) << ">";
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            str << "s<" << stringify(*node.spec()) << ">";
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            str << "b<" << stringify(*node.spec()) << ">";
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            str << "l<" << stringify(*node.spec()) << ">";
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            str << "all<";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << ">";
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            str << "any<";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            str << ">";
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            str << "cond<" << *node.spec() << ",";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
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
            std::shared_ptr<DependencySpecTree> d(fakerepository::parse_depend(
                        "( ( a/a b/b ) )", &env, std::shared_ptr<const PackageID>()));

            QuickPrinter p;
            d->root()->accept(p);
            TEST_CHECK_EQUAL(p.str.str(), "all<all<all<p<a/a>p<b/b>>>>");
        }
    } dep_parser_test;
}

