/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2011 Ciaran McCreesh
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

#include <paludis/util/graph.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/destringify.hh>

#include <list>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Graph, Directed)
{
    DirectedGraph<std::string, int> g;

    EXPECT_TRUE(! g.has_node("a"));
    EXPECT_TRUE(! g.has_node("b"));
    EXPECT_TRUE(! g.has_node("c"));
    EXPECT_TRUE(! g.has_node("d"));
    EXPECT_TRUE(! g.has_node("e"));
    EXPECT_TRUE(! g.has_node("f"));
    EXPECT_TRUE(! g.has_node("x"));

    g.add_node("a");
    g.add_node("b");
    g.add_node("c");
    g.add_node("d");
    g.add_node("e");
    g.add_node("f");

    EXPECT_TRUE(g.has_node("a"));
    EXPECT_TRUE(g.has_node("b"));
    EXPECT_TRUE(g.has_node("c"));
    EXPECT_TRUE(g.has_node("d"));
    EXPECT_TRUE(g.has_node("e"));
    EXPECT_TRUE(g.has_node("f"));
    EXPECT_TRUE(! g.has_node("x"));

    g.add_node("y");
    EXPECT_TRUE(g.has_node("y"));
    g.delete_node("y");
    EXPECT_TRUE(! g.has_node("y"));

    g.add_edge("a", "b", 1);
    g.add_edge("b", "c", 2);
    g.add_edge("c", "e", 3);
    g.add_edge("b", "d", 4);
    g.add_edge("d", "e", 5);
    g.add_edge("d", "f", 6);

    for (char mc('a') ; mc < 'g' ; ++mc)
        for (char nc('a') ; nc < 'g' ; ++nc)
        {
            std::string m(stringify(mc)), n(stringify(nc));

            if (g.has_edge(m, n))
            {
                EXPECT_TRUE(! g.has_edge(n, m));
                EXPECT_TRUE(g.has_outgoing_edges(m));
                EXPECT_TRUE(0 != g.fetch_edge(m, n));
            }
            else
                EXPECT_THROW(g.fetch_edge(m, n), NoSuchGraphEdgeError);
        }

    std::list<std::string> t;
    g.topological_sort(std::back_inserter(t));

    EXPECT_EQ("e c f d b a", join(t.begin(), t.end(), " "));

    g.add_edge("e", "b", 7);
    EXPECT_THROW(g.topological_sort(std::back_inserter(t)), NoGraphTopologicalOrderExistsError);

    try
    {
        g.topological_sort(std::back_inserter(t));
        FAIL();
    }
    catch (const NoGraphTopologicalOrderExistsError & e)
    {
        EXPECT_EQ("a b c d e", join(e.remaining_nodes()->begin(), e.remaining_nodes()->end(), " "));
    }
}

