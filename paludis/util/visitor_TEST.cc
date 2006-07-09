/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <algorithm>
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for visitor.hh .
 *
 * \ingroup grptestcases
 */

#ifndef DOXYGEN

namespace
{
    class Deleter
    {
        public:
            /**
             * Constructor.
             */
            Deleter()
            {
            }

            /**
             * Delete an item.
             */
            template <typename T_>
            void operator() (T_ t)
            {
                delete t;
            }
    };

    class Node;
    class FooNode;
    class BarNode;

    typedef VisitorTypes<FooNode *, BarNode *> NodeVisitorTypes;

    struct Node :
        virtual VisitableInterface<NodeVisitorTypes>
    {
    };

    struct FooNode :
        Node,
        Visitable<FooNode, NodeVisitorTypes>
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
        Node,
        Visitable<BarNode, NodeVisitorTypes>
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
        NodeVisitorTypes::ConstVisitor
    {
        std::string r;

        virtual void visit(const FooNode * const f)
        {
            r.append(f->c_foo());
        }

        virtual void visit(const BarNode * const b)
        {
            r.append(b->c_bar());
        }
    };

    struct NodeVisitor :
        NodeVisitorTypes::Visitor
    {
        std::string r;

        virtual void visit(FooNode * const f)
        {
            r.append(f->foo());
        }

        virtual void visit(BarNode * const b)
        {
            r.append(b->bar());
        }
    };
}

#endif

namespace test_cases
{
    /**
     * \test Test const visitors.
     *
     * \ingroup grptestcases
     */
    struct ConstVisitorTest : TestCase
    {
        ConstVisitorTest() : TestCase("const visitor") { }

        void run()
        {
            std::vector<Node *> v;

            v.push_back(new FooNode);
            v.push_back(new BarNode);
            v.push_back(new FooNode);

            NodeCVisitor c;
            TEST_CHECK_EQUAL(c.r, "");
            std::for_each(v.begin(), v.end(), accept_visitor(&c));
            TEST_CHECK_EQUAL(c.r, "c_fooc_barc_foo");

            std::for_each(v.begin(), v.end(), Deleter());
        }
    } test_const_visitor;

    /**
     * \test Test non-const visitors.
     *
     * \ingroup grptestcases
     */
    struct VisitorTest : TestCase
    {
        VisitorTest() : TestCase("visitor") { }

        void run()
        {
            std::vector<Node *> v;

            v.push_back(new FooNode);
            v.push_back(new BarNode);
            v.push_back(new FooNode);

            NodeVisitor c;
            TEST_CHECK_EQUAL(c.r, "");
            std::for_each(v.begin(), v.end(), accept_visitor(&c));
            TEST_CHECK_EQUAL(c.r, "foobarfoo");

            std::for_each(v.begin(), v.end(), Deleter());
        }
    } test_visitor;
}

