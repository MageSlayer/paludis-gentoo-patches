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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
    class SimpleGraph :
        private PrivateImplementationPattern<SimpleGraph>,
        private InstantiationPolicy<SimpleGraph, instantiation_method::NonCopyableTag>
    {
        public:
            SimpleGraph(const unsigned nodes);
            ~SimpleGraph();

            unsigned size() const;

            void connect(const unsigned, const unsigned);
            void unconnect(const unsigned, const unsigned);
            void reverse();

            bool is_connected(const unsigned, const unsigned) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
            bool has_outgoing(const unsigned) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
            bool has_incoming(const unsigned) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void clear_incoming(const unsigned);
            void clear_outgoing(const unsigned);
    };

    class NoSimpleTopologicalOrderingExists :
        public Exception
    {
        private:
            const unsigned _cycle;

        public:
            NoSimpleTopologicalOrderingExists(const unsigned &) throw ();

            unsigned cycle() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class SimpleTopologicalOrdering :
        private PrivateImplementationPattern<SimpleTopologicalOrdering>
    {
        private:
            void _dfs(const SimpleGraph &, const unsigned);

        public:
            SimpleTopologicalOrdering(const SimpleGraph &);
            ~SimpleTopologicalOrdering();

            typedef libwrapiter::ForwardIterator<SimpleTopologicalOrdering, const unsigned> Iterator;
            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    template <typename T_>
    class Graph :
        protected SimpleGraph
    {
        protected:
            virtual unsigned node_to_index(const T_ &) const = 0;

            Graph(const unsigned nodes) :
                SimpleGraph(nodes)
            {
            }

        public:
            virtual ~Graph()
            {
            }

            using SimpleGraph::size;

            void connect(const T_ & a, const T_ & b)
            {
                SimpleGraph::connect(node_to_index(a), node_to_index(b));
            }

            void unconnect(const T_ a, const T_ & b)
            {
                SimpleGraph::unconnect(node_to_index(a), node_to_index(b));
            }

            using SimpleGraph::reverse;

            bool is_connected(const T_ &, const T_ &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_outgoing(const T_ &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_incoming(const T_ &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            void clear_incoming(const T_ & a)
            {
                SimpleGraph::clear_incoming(a);
            }

            void clear_outgoing(const T_ & a)
            {
                SimpleGraph::clear_outgoing(a);
            }
    };

    template <typename T_>
    bool Graph<T_>::is_connected(const T_ & a, const T_ & b) const
    {
        return SimpleGraph::is_connected(node_to_index(a), node_to_index(b));
    }

    template <typename T_>
    bool Graph<T_>::has_outgoing(const T_ & a) const
    {
        return SimpleGraph::has_outgoing(node_to_index(a));
    }

    template <typename T_>
    bool Graph<T_>::has_incoming(const T_ & a) const
    {
        return SimpleGraph::has_incoming(node_to_index(a));
    }
}

#endif
