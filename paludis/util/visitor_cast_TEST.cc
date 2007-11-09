/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/util/visitor.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>
#include <vector>

using namespace paludis;
using namespace test;

namespace
{
    class Node;
    class FooNode;
    class BarNode;

    struct NodeVisitorTypes :
        VisitorTypes<
            NodeVisitorTypes,
            Node,
            TreeLeaf<NodeVisitorTypes, FooNode>,
            TreeLeaf<NodeVisitorTypes, BarNode> >
    {
    };

    struct Node
    {
    };

    struct FooNode :
        Node
    {
        std::string foo()
        {
            return "foo";
        }
    };

    struct BarNode :
        Node
    {
        std::string c_bar() const
        {
            return "c_bar";
        }

        std::string bar()
        {
            return "bar";
        }
    };
}

namespace test_cases
{
    struct VisitorCastTest : TestCase
    {
        VisitorCastTest() : TestCase("visitor_cast<>") { }

        void run()
        {
            std::vector<tr1::shared_ptr<NodeVisitorTypes::ConstItem> > v;

            v.push_back(tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(tr1::shared_ptr<FooNode>(new FooNode))));
            v.push_back(tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, BarNode>(tr1::shared_ptr<BarNode>(new BarNode))));
            v.push_back(tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(tr1::shared_ptr<FooNode>(new FooNode))));

            TEST_CHECK(visitor_cast<const FooNode>(*v[0]));
            TEST_CHECK(! visitor_cast<const FooNode>(*v[1]));
            TEST_CHECK(visitor_cast<const FooNode>(*v[2]));

            TEST_CHECK(! visitor_cast<const BarNode>(*v[0]));
            TEST_CHECK(visitor_cast<const BarNode>(*v[1]));
            TEST_CHECK(! visitor_cast<const BarNode>(*v[2]));
        }
    } test_visitor_cast;
}


