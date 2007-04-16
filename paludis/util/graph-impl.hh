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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_GRAPH_IMPL_HH 1

#include <paludis/util/graph.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <map>
#include <set>
#include <list>

namespace paludis
{
    class NoGraphTopologicalOrderExistsError::RemainingNodes
    {
        private:
            std::list<std::string> _n;

        public:
            void add(const std::string & s)
            {
                _n.push_back(s);
            }

            typedef libwrapiter::ForwardIterator<RemainingNodes, const std::string> Iterator;

            Iterator begin() const
            {
                return Iterator(_n.begin());
            }

            Iterator end() const
            {
                return Iterator(_n.end());
            }
    };

    template<>
    template <typename Node_, typename Edge_>
    struct Implementation<DirectedGraph<Node_, Edge_> >
    {
        std::map<Node_, std::map<Node_, Edge_> > store;

        Implementation()
        {
        }

        Implementation(const std::map<Node_, std::map<Node_, Edge_> > s) :
            store(s)
        {
        }
    };

    template <typename Node_, typename Edge_>
    DirectedGraph<Node_, Edge_>::DirectedGraph() :
        PrivateImplementationPattern<DirectedGraph<Node_, Edge_> >(new Implementation<DirectedGraph<Node_, Edge_> >)
    {
    }

    template <typename Node_, typename Edge_>
    DirectedGraph<Node_, Edge_>::DirectedGraph(const DirectedGraph & g) :
        PrivateImplementationPattern<DirectedGraph<Node_, Edge_> >(new Implementation<DirectedGraph<Node_, Edge_> >(g._imp->store))
    {
    }

    template <typename Node_, typename Edge_>
    DirectedGraph<Node_, Edge_>::~DirectedGraph()
    {
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::add_node(const Node_ & n)
    {
        _imp->store.insert(std::make_pair(n, std::map<Node_, Edge_>()));
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::delete_node(const Node_ & n)
    {
        delete_incoming_edges(n);
        _imp->store.erase(n);
    }

    template <typename Node_, typename Edge_>
    bool
    DirectedGraph<Node_, Edge_>::has_node(const Node_ & n) const
    {
        return _imp->store.end() != _imp->store.find(n);
    }

    template <typename Node_, typename Edge_>
    class DirectedGraph<Node_, Edge_>::NodeIterator
    {
        friend class DirectedGraph<Node_, Edge_>;

        private:
            typename std::map<Node_, std::map<Node_, Edge_> >::const_iterator _i;

            NodeIterator(const typename std::map<Node_, std::map<Node_, Edge_> >::const_iterator & i) :
                _i(i)
            {
            }

        public:
            NodeIterator(const NodeIterator & other) :
                _i(other._i)
            {
            }

            ~NodeIterator()
            {
            }

            bool operator== (const NodeIterator & other) const
            {
                return _i == other._i;
            }

            bool operator!= (const NodeIterator & other) const
            {
                return ! operator== (other);
            }

            NodeIterator & operator++ ()
            {
                ++_i;
                return *this;
            }

            NodeIterator operator++ (int)
            {
                NodeIterator tmp(*this);
                ++_i;
                return tmp;
            }

            const Node_ & operator*() const
            {
                return _i->first;
            }

            const Node_ * operator->() const
            {
                return &_i->first;
            }
    };

    template <typename Node_, typename Edge_>
    typename DirectedGraph<Node_, Edge_>::NodeIterator
    DirectedGraph<Node_, Edge_>::begin_nodes() const
    {
        return NodeIterator(_imp->store.begin());
    }

    template <typename Node_, typename Edge_>
    typename DirectedGraph<Node_, Edge_>::NodeIterator
    DirectedGraph<Node_, Edge_>::end_nodes() const
    {
        return NodeIterator(_imp->store.end());
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::add_edge(const Node_ & n1, const Node_ & n2, const Edge_ & e)
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::iterator i(_imp->store.find(n1));
        if (i == _imp->store.end())
            throw NoSuchGraphNodeError(n1);

        if (! has_node(n2))
            throw NoSuchGraphNodeError(n2);

        i->second.insert(std::make_pair(n2, e));
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::delete_edge(const Node_ & n1, const Node_ & n2)
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
            i->second.erase(n2);
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::delete_outgoing_edges(const Node_ & n)
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::iterator i(_imp->store.find(n.first));
        if (i != _imp->store.end())
            i->second.clear();
    }

    template <typename Node_, typename Edge_>
    void
    DirectedGraph<Node_, Edge_>::delete_incoming_edges(const Node_ & n)
    {
        for (typename std::map<Node_, std::map<Node_, Edge_> >::iterator i(_imp->store.begin()),
                i_end(_imp->store.end()) ; i != i_end ; ++i)
            i->second.erase(n);
    }

    template <typename Node_, typename Edge_>
    bool
    DirectedGraph<Node_, Edge_>::has_edge(const Node_ & n1, const Node_ & n2) const
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::const_iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
            return i->second.end() != i->second.find(n2);
        return false;
    }

    template <typename Node_, typename Edge_>
    const Edge_
    DirectedGraph<Node_, Edge_>::fetch_edge(const Node_ & n1, const Node_ & n2) const
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::const_iterator i(_imp->store.find(n1));
        if (i != _imp->store.end())
        {
            typename std::map<Node_, Edge_>::const_iterator j(i->second.find(n2));
            if (j != i->second.end())
                return j->second;
        }

        throw NoSuchGraphEdgeError(n1, n2);
    }

    template <typename Node_, typename Edge_>
    bool
    DirectedGraph<Node_, Edge_>::has_outgoing_edges(const Node_ & n) const
    {
        typename std::map<Node_, std::map<Node_, Edge_> >::const_iterator i(_imp->store.find(n));
        if (i == _imp->store.end())
            throw NoSuchGraphNodeError(n);

        return ! i->second.empty();
    }

    template <typename Node_, typename Edge_>
    template <typename OutputIterator_>
    void
    DirectedGraph<Node_, Edge_>::topological_sort(OutputIterator_ x) const
    {
        struct Sorter
        {
            DirectedGraph g;
            std::set<Node_> done;

            Sorter(const DirectedGraph & gg) :
                g(gg)
            {
            }

            void do_one(OutputIterator_ & i, const Node_ & n)
            {
                if (done.end() != done.find(n))
                    return;

                if (g.has_outgoing_edges(n))
                    return;

                *i++ = n;
                done.insert(n);

                for (typename DirectedGraph::NodeIterator m(g.begin_nodes()), m_end(g.end_nodes()) ; m != m_end ; )
                    if (g.has_edge(*m, n))
                    {
                        g.delete_edge(*m, n);
                        do_one(i, *m++);
                    }
                    else
                        ++m;
            }

            void sort(OutputIterator_ & i)
            {
                unsigned c(0);
                for (typename DirectedGraph::NodeIterator n(g.begin_nodes()), n_end(g.end_nodes()) ; n != n_end ; )
                {
                    ++c;
                    do_one(i, *n++);
                }

                if (done.size() < c)
                {
                    std::tr1::shared_ptr<NoGraphTopologicalOrderExistsError::RemainingNodes> r(
                            new NoGraphTopologicalOrderExistsError::RemainingNodes);
                    for (typename DirectedGraph::NodeIterator n(g.begin_nodes()), n_end(g.end_nodes()) ; n != n_end ; ++n)
                        if (done.end() == done.find(*n))
                            r->add(stringify(*n));

                    throw NoGraphTopologicalOrderExistsError(r);
                }
            }
        };

        Sorter s(*this);
        s.sort(x);
    }
}

#endif
