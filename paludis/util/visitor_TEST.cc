/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <list>
#include <paludis/util/visitor.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/no_type.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>
#include <vector>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for visitor.hh .
 *
 */

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
        std::string c_foo() const
        {
            return "c_foo";
        }

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

    struct NodeCVisitor :
        ConstVisitor<NodeVisitorTypes>
    {
        std::string r;

        virtual void visit_leaf(const FooNode & f)
        {
            r.append(f.c_foo());
        }

        virtual void visit_leaf(const BarNode & b)
        {
            r.append(b.c_bar());
        }
    };

    struct NodeVisitor :
        Visitor<NodeVisitorTypes>
    {
        std::string r;

        virtual void visit_leaf(FooNode & f)
        {
            r.append(f.foo());
        }

        virtual void visit_leaf(BarNode & b)
        {
            r.append(b.bar());
        }
    };
}

namespace test_cases
{
    /**
     * \test Test const visitors.
     *
     */
    struct ConstVisitorTest : TestCase
    {
        ConstVisitorTest() : TestCase("const visitor") { }

        void run()
        {
            std::vector<std::tr1::shared_ptr<NodeVisitorTypes::ConstItem> > v;

            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(std::tr1::shared_ptr<FooNode>(new FooNode))));
            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, BarNode>(std::tr1::shared_ptr<BarNode>(new BarNode))));
            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::ConstItem>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(std::tr1::shared_ptr<FooNode>(new FooNode))));

            NodeCVisitor c;
            TEST_CHECK_EQUAL(c.r, "");
            std::for_each(indirect_iterator(v.begin()), indirect_iterator(v.end()), accept_visitor(c));
            TEST_CHECK_EQUAL(c.r, "c_fooc_barc_foo");

            TEST_CHECK((ConstVisitor<NodeVisitorTypes>::Contains<const TreeLeaf<NodeVisitorTypes, FooNode> >::value));
            TEST_CHECK((! ConstVisitor<NodeVisitorTypes>::Contains<const TreeLeaf<NodeVisitorTypes, NoType<12345u> > >::value));
        }
    } test_const_visitor;

    /**
     * \test Test non-const visitors.
     *
     */
    struct VisitorTest : TestCase
    {
        VisitorTest() : TestCase("visitor") { }

        void run()
        {
            std::vector<std::tr1::shared_ptr<NodeVisitorTypes::Item> > v;

            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::Item>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(std::tr1::shared_ptr<FooNode>(new FooNode))));
            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::Item>(
                        new TreeLeaf<NodeVisitorTypes, BarNode>(std::tr1::shared_ptr<BarNode>(new BarNode))));
            v.push_back(std::tr1::shared_ptr<NodeVisitorTypes::Item>(
                        new TreeLeaf<NodeVisitorTypes, FooNode>(std::tr1::shared_ptr<FooNode>(new FooNode))));

            NodeVisitor c;
            TEST_CHECK_EQUAL(c.r, "");
            std::for_each(indirect_iterator(v.begin()), indirect_iterator(v.end()), accept_visitor(c));
            TEST_CHECK_EQUAL(c.r, "foobarfoo");

            TEST_CHECK((ConstVisitor<NodeVisitorTypes>::Contains<TreeLeaf<NodeVisitorTypes, FooNode> >::value));
            TEST_CHECK((! ConstVisitor<NodeVisitorTypes>::Contains<TreeLeaf<NodeVisitorTypes, NoType<12345u> > >::value));
        }
    } test_visitor;
}

