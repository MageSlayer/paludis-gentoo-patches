/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_IMPL_HH 1

#include <paludis/util/graph.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/set-impl.hh>
#include <map>
#include <set>
#include <list>

/** \file
 * Implementation for paludis/util/graph.hh .
 *
 * \ingroup g_data_structures
 */

namespace paludis
{
    /**
     * Holds the remaining nodes list for a NoGraphTopologicalOrderExistsError.
     *
     * \see NoGraphTopologicalOrderExistsError
     * \ingroup g_exceptions
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoGraphTopologicalOrderExistsError::RemainingNodes
    {
        private:
            std::list<std::string> _n;

        public:
            /**
             * Add a remaining node.
             */
            void add(const std::string & s)
            {
                _n.push_back(s);
            }

            ///\name Iterate over our nodes
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;

            ConstIterator begin() const
            {
                return ConstIterator(_n.begin());
            }

            ConstIterator end() const
            {
                return ConstIterator(_n.end());
            }

            ///\}
    };

    template <>
    struct WrappedForwardIteratorTraits<NoGraphTopologicalOrderExistsError::RemainingNodes::ConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };

    /**
     * Implementation data for a DirectedGraph.
     *
     * \see DirectedGraph
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename Node_, typename Edge_, typename Comparator_>
    struct Implementation<DirectedGraph<Node_, Edge_, Comparator_> >
    {
        /// Our data.
        std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_> store;

        ///\name Basic operations
        ///\{

        Implementation()
        {
        }

        Implementation(const std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_> s) :
            store(s)
        {
        }

        ///\}
    };

    template <typename Node_, typename Edge_, typename Comparator_>
    DirectedGraph<Node_, Edge_, Comparator_>::DirectedGraph() :
        PrivateImplementationPattern<DirectedGraph<Node_, Edge_, Comparator_> >(new Implementation<DirectedGraph<Node_, Edge_, Comparator_> >)
    {
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    DirectedGraph<Node_, Edge_, Comparator_>::DirectedGraph(const DirectedGraph & g) :
        PrivateImplementationPattern<DirectedGraph<Node_, Edge_, Comparator_> >(new Implementation<DirectedGraph<Node_, Edge_, Comparator_> >(g._imp->store))
    {
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    DirectedGraph<Node_, Edge_, Comparator_>::~DirectedGraph()
    {
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::add_node(const Node_ & n)
    {
        _imp->store.insert(std::make_pair(n, std::map<Node_, Edge_, Comparator_>()));
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::delete_node(const Node_ & n)
    {
        delete_incoming_edges(n);
        _imp->store.erase(n);
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    bool
    DirectedGraph<Node_, Edge_, Comparator_>::has_node(const Node_ & n) const
    {
        return _imp->store.end() != _imp->store.find(n);
    }

    /**
     * Iterate over the nodes in a DirectedGraph.
     *
     * \see DirectedGraph
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename Node_, typename Edge_, typename Comparator_>
    class DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator
    {
        friend class DirectedGraph<Node_, Edge_, Comparator_>;

        private:
            typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::const_iterator _i;

            NodeConstIterator(const typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::const_iterator & i) :
                _i(i)
            {
            }

        public:
            ///\name Basic operations
            ///\{

            NodeConstIterator(const NodeConstIterator & other) :
                _i(other._i)
            {
            }

            ~NodeConstIterator()
            {
            }

            ///\}

            ///\name Comparison operators
            ///\{

            bool operator== (const NodeConstIterator & other) const
            {
                return _i == other._i;
            }

            bool operator!= (const NodeConstIterator & other) const
            {
                return ! operator== (other);
            }

            ///\}

            ///\name Advance operators
            ///\{

            NodeConstIterator & operator++ ()
            {
                ++_i;
                return *this;
            }

            NodeConstIterator operator++ (int)
            {
                NodeConstIterator tmp(*this);
                ++_i;
                return tmp;
            }

            ///\}

            ///\name Dereference operators
            ///\{

            const Node_ & operator*() const
            {
                return _i->first;
            }

            const Node_ * operator->() const
            {
                return &_i->first;
            }

            ///\}
    };

    template <typename Node_, typename Edge_, typename Comparator_>
    typename DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator
    DirectedGraph<Node_, Edge_, Comparator_>::begin_nodes() const
    {
        return NodeConstIterator(_imp->store.begin());
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    typename DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator
    DirectedGraph<Node_, Edge_, Comparator_>::end_nodes() const
    {
        return NodeConstIterator(_imp->store.end());
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::add_edge(const Node_ & n1, const Node_ & n2, const Edge_ & e)
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::iterator i(_imp->store.find(n1));
        if (i == _imp->store.end())
            throw NoSuchGraphNodeError(n1);

        if (! has_node(n2))
            throw NoSuchGraphNodeError(n2);

        i->second.insert(std::make_pair(n2, e));
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::delete_edge(const Node_ & n1, const Node_ & n2)
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
            i->second.erase(n2);
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::delete_outgoing_edges(const Node_ & n)
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::iterator i(_imp->store.find(n.first));
        if (i != _imp->store.end())
            i->second.clear();
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::delete_incoming_edges(const Node_ & n)
    {
        for (typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::iterator i(_imp->store.begin()),
                i_end(_imp->store.end()) ; i != i_end ; ++i)
            i->second.erase(n);
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    bool
    DirectedGraph<Node_, Edge_, Comparator_>::has_edge(const Node_ & n1, const Node_ & n2) const
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::const_iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
            return i->second.end() != i->second.find(n2);
        return false;
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    const Edge_
    DirectedGraph<Node_, Edge_, Comparator_>::fetch_edge(const Node_ & n1, const Node_ & n2) const
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::const_iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
        {
            typename std::map<Node_, Edge_, Comparator_>::const_iterator j(i->second.find(n2));
            if (j != i->second.end())
                return j->second;
        }

        throw NoSuchGraphEdgeError(n1, n2);
    }

    template <typename Node_, typename Edge_, typename Comparator_>
    bool
    DirectedGraph<Node_, Edge_, Comparator_>::has_outgoing_edges(const Node_ & n) const
    {
        typename std::map<Node_, std::map<Node_, Edge_, Comparator_>, Comparator_ >::const_iterator i(_imp->store.find(n));
        if (i == _imp->store.end())
            throw NoSuchGraphNodeError(n);

        return ! i->second.empty();
    }

    template <typename Node_, typename Edge_, typename Comparator_, typename OutputConstIterator_>
    struct DirectedGraphTopologicalSorter
    {
        DirectedGraph<Node_, Edge_, Comparator_> g;
        std::set<Node_, Comparator_> done;

        DirectedGraphTopologicalSorter(const DirectedGraph<Node_, Edge_, Comparator_> & gg) :
            g(gg)
        {
        }

        void do_one(OutputConstIterator_ & i, const Node_ & n)
        {
            if (done.end() != done.find(n))
                return;

            if (g.has_outgoing_edges(n))
                return;

            *i++ = n;
            done.insert(n);

            for (typename DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator m(g.begin_nodes()), m_end(g.end_nodes()) ; m != m_end ; )
                if (g.has_edge(*m, n))
                {
                    g.delete_edge(*m, n);
                    do_one(i, *m++);
                }
                else
                    ++m;
        }

        template <typename T_>
        static const T_ & depointer(const T_ & t)
        {
            return t;
        }

        template <typename T_>
        static const T_ & depointer(const std::tr1::shared_ptr<T_> & t)
        {
            return *t;
        }

        void sort(OutputConstIterator_ & i)
        {
            unsigned c(0);
            for (typename DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator n(g.begin_nodes()), n_end(g.end_nodes()) ; n != n_end ; )
            {
                ++c;
                do_one(i, *n++);
            }

            if (done.size() < c)
            {
                std::tr1::shared_ptr<NoGraphTopologicalOrderExistsError::RemainingNodes> r(
                        new NoGraphTopologicalOrderExistsError::RemainingNodes);
                for (typename DirectedGraph<Node_, Edge_, Comparator_>::NodeConstIterator n(g.begin_nodes()), n_end(g.end_nodes()) ; n != n_end ; ++n)
                    if (done.end() == done.find(*n))
                        r->add(stringify(depointer(*n)));

                throw NoGraphTopologicalOrderExistsError(r);
            }
        }
    };

    template <typename Node_, typename Edge_, typename Comparator_>
    template <typename OutputConstIterator_>
    void
    DirectedGraph<Node_, Edge_, Comparator_>::topological_sort(OutputConstIterator_ x) const
    {
        DirectedGraphTopologicalSorter<Node_, Edge_, Comparator_, OutputConstIterator_> s(*this);
        s.sort(x);
    }
}

#endif
