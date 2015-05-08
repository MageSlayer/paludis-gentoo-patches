/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/util/graph-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <memory>

/** \file
 * Declarations for DirectedGraph and related utilities.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Base class for DirectedGraph errors.
     *
     * \ingroup g_data_structures
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GraphError :
        public Exception
    {
        protected:
            ///\name Basic operations
            ///\{

            GraphError(const std::string & msg) noexcept;

            ///\}
    };

    /**
     * Thrown if a DirectedGraph operation relies upon a node being present when it is not.
     *
     * \ingroup g_exceptions
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchGraphNodeError :
        public GraphError
    {
        public:
            ///\name Basic operations
            ///\{

            template <typename Node_>
            NoSuchGraphNodeError(const Node_ & node) noexcept :
                GraphError("Node '" + stringify(node) + "' does not exist")
            {
            }

            template <typename Node_>
            NoSuchGraphNodeError(const std::shared_ptr<Node_> & node) noexcept :
                GraphError("Node '" + stringify(*node) + "' does not exist")
            {
            }

            ///\}
    };

    /**
     * Thrown if a DirectedGraph operation relies upon an edge being present when it is not.
     *
     * \ingroup g_exceptions
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchGraphEdgeError :
        public GraphError
    {
        public:
            ///\name Basic operations
            ///\{

            template <typename Node_>
            NoSuchGraphEdgeError(const Node_ & e1, const Node_ & e2) noexcept :
                GraphError("Edge '" + stringify(e1) + "' -> '" + stringify(e2) + "' does not exist")
            {
            }

            ///\}
    };

    /**
     * Thrown if no ordering is available for a DirectedGraph::topological_sort.
     *
     * \ingroup g_data_structures
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoGraphTopologicalOrderExistsError :
        public GraphError
    {
        public:
            class RemainingNodes;

        private:
            std::shared_ptr<const RemainingNodes> _remaining_nodes;

        public:
            ///\name Basic operations
            ///\{

            NoGraphTopologicalOrderExistsError(const std::shared_ptr<const RemainingNodes> &) noexcept;
            ~NoGraphTopologicalOrderExistsError() noexcept;

            ///\}

            /**
             * The nodes remaining in the graph.
             */
            std::shared_ptr<const RemainingNodes> remaining_nodes() const;
    };

    /**
     * A simple directed graph.
     *
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename Node_, typename Edge_, typename Comparator_>
    class PALUDIS_VISIBLE DirectedGraph
    {
        private:
            Pimp<DirectedGraph<Node_, Edge_, Comparator_> > _imp;

            void operator= (const DirectedGraph &);

        public:
            ///\name Basic operations
            ///\{

            DirectedGraph();
            DirectedGraph(const DirectedGraph &);
            ~DirectedGraph();

            ///\}

            ///\name Node related functions
            ///\{

            /**
             * Add a node, if it does not already exist.
             */
            void add_node(const Node_ &);

            /**
             * Delete a node, if it exists.
             */
            void delete_node(const Node_ &);

            /**
             * Return whether a node exists.
             */
            bool has_node(const Node_ &) const;

            ///\}

            ///\name Iterate over our nodes
            ///\{

            class NodeConstIterator;
            NodeConstIterator begin_nodes() const;
            NodeConstIterator end_nodes() const;

            ///\}

            ///\name Edge related functions
            ///\{

            /**
             * Add an edge, if it does not already exist.
             *
             * \throw NoSuchGraphNodeError if either node is not in the graph.
             */
            void add_edge(const Node_ &, const Node_ &, const Edge_ &);

            /**
             * Delete an edge, if it exists.
             */
            void delete_edge(const Node_ &, const Node_ &);

            /**
             * Delete all edges leaving a node.
             */
            void delete_outgoing_edges(const Node_ &);

            /**
             * Delete all edges entering a node.
             */
            void delete_incoming_edges(const Node_ &);

            /**
             * Return whether an edge exists.
             */
            bool has_edge(const Node_ &, const Node_ &) const;

            /**
             * Fetch an edge.
             *
             * \throw NoSuchGraphEdgeError if the edge does not exist.
             */
            const Edge_ fetch_edge(const Node_ &, const Node_ &) const;

            /**
             * Return whether a node has outgoing edges.
             *
             * \throw NoSuchGraphNodeError if the node does not exist.
             */
            bool has_outgoing_edges(const Node_ &) const;

            ///\}

            ///\name Ordering functions
            ///\{

            /**
             * Place our nodes, topological sorted, into OutputIterator_.
             *
             * \throw NoGraphTopologicalOrderExistsError if no such order exists.
             */
            template <typename OutputIterator_>
            void topological_sort(OutputIterator_ i) const;

            ///\}
    };
}

#endif
