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

#include "graph.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <paludis/util/join.hh>
#include <paludis/util/destringify.hh>

using namespace test;
using namespace paludis;

namespace
{
    class StringGraph :
        public Graph<std::string>
    {
        protected:
            virtual unsigned node_to_index(const std::string & s) const
            {
                return destringify<unsigned>(s);
            }

        public:
            StringGraph(unsigned size) :
                Graph<std::string>(size)
            {
            }
    };
}

namespace test_cases
{
    struct SimpleGraphTest : TestCase
    {
        SimpleGraphTest() : TestCase("simple graph") { }

        void run()
        {
            SimpleGraph g(20);
            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                {
                    TEST_CHECK(! g.is_connected(x, y));
                    if (x > y)
                    {
                        g.connect(x, y);
                        TEST_CHECK(g.is_connected(x, y));
                    }
                }

            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                    TEST_CHECK_EQUAL(g.is_connected(x, y), bool(x > y));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_outgoing(x), bool(x > 0));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_incoming(x), bool(x < 19));

            g.reverse();

            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                    TEST_CHECK_EQUAL(g.is_connected(x, y), bool(x < y));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_incoming(x), bool(x > 0));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_outgoing(x), bool(x < 19));
        }
    } test_simple_graph;

    struct SimpleTopologicalOrderingTest : TestCase
    {
        SimpleTopologicalOrderingTest() : TestCase("simple topological ordering") { }

        void run()
        {
            SimpleGraph g(8);
            g.connect(0, 1);
            g.connect(1, 2);
            g.connect(2, 3);
            g.connect(1, 4);
            g.connect(4, 3);
            g.connect(4, 5);
            g.connect(7, 6);

            SimpleTopologicalOrdering t(g);
            TEST_CHECK_EQUAL(join(t.begin(), t.end(), " "), "3 2 5 4 1 0 6 7");
        }
    } test_topological_ordering;

    struct SimpleTopologicalOrderingCycleTest : TestCase
    {
        SimpleTopologicalOrderingCycleTest() : TestCase("simple topological ordering cycles") { }

        void run()
        {
            SimpleGraph g(3);
            g.connect(0, 1);
            g.connect(1, 2);
            g.connect(2, 0);

            TEST_CHECK_THROWS((SimpleTopologicalOrdering(g)), NoSimpleTopologicalOrderingExists);

            SimpleGraph h(3);
            h.connect(0, 1);
            h.connect(2, 2);

            TEST_CHECK_THROWS((SimpleTopologicalOrdering(h)), NoSimpleTopologicalOrderingExists);
        }
    } test_topological_ordering_cycles;

    struct GraphTest : TestCase
    {
        GraphTest() : TestCase("graph") { }

        void run()
        {
            StringGraph g(20);
            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                {
                    TEST_CHECK(! g.is_connected(stringify(x), stringify(y)));
                    if (x > y)
                    {
                        g.connect(stringify(x), stringify(y));
                        TEST_CHECK(g.is_connected(stringify(x), stringify(y)));
                    }
                }

            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                    TEST_CHECK_EQUAL(g.is_connected(stringify(x), stringify(y)), bool(x > y));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_outgoing(stringify(x)), bool(x > 0));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_incoming(stringify(x)), bool(x < 19));

            g.reverse();

            for (unsigned x(0) ; x < 20 ; ++x)
                for (unsigned y(0) ; y < 20 ; ++y)
                    TEST_CHECK_EQUAL(g.is_connected(stringify(x), stringify(y)), bool(x < y));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_incoming(stringify(x)), bool(x > 0));

            for (unsigned x(0) ; x < 20 ; ++x)
                TEST_CHECK_EQUAL(g.has_outgoing(stringify(x)), bool(x < 19));
        }
    } test_graph;
}

