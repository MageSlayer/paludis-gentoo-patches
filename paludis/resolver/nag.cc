/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/nag.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/strongly_connected_component.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <algorithm>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

typedef std::tr1::unordered_set<Resolvent, Hash<Resolvent> > Nodes;
typedef std::tr1::unordered_map<Resolvent, Nodes, Hash<Resolvent> > Edges;

namespace paludis
{
    namespace n
    {
        typedef Name<struct index_name> index;
        typedef Name<struct lowlink_name> lowlink;
    }

    template <>
    struct Implementation<NAG>
    {
        Nodes nodes;
        Edges edges;
    };

    template <>
    struct WrappedForwardIteratorTraits<NAG::EdgesFromConstIteratorTag>
    {
        typedef Nodes::const_iterator UnderlyingIterator;
    };
}

NAG::NAG() :
    PrivateImplementationPattern<NAG>(new Implementation<NAG>)
{
}

NAG::~NAG()
{
}

void
NAG::add_node(const Resolvent & r)
{
    _imp->nodes.insert(r);
}

void
NAG::add_edge(const Resolvent & a, const Resolvent & b)
{
    _imp->edges.insert(std::make_pair(a, Nodes())).first->second.insert(b);
}

void
NAG::verify_edges() const
{
    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        if (_imp->nodes.end() == _imp->nodes.find(e->first))
            throw InternalError(PALUDIS_HERE, "Missing node for edge " + stringify(e->first));

        for (Nodes::const_iterator f(e->second.begin()), f_end(e->second.end()) ;
                f != f_end ; ++f)
            if (_imp->nodes.end() == _imp->nodes.find(*f))
                throw InternalError(PALUDIS_HERE, "Missing node for edge " + stringify(e->first) + " -> " + stringify(*f));
    }
}

namespace
{
    struct TarjanData
    {
        NamedValue<n::index, int> index;
        NamedValue<n::lowlink, int> lowlink;
    };

    typedef std::tr1::unordered_map<Resolvent, TarjanData, Hash<Resolvent> > TarjanDataMap;
    typedef std::list<Resolvent> TarjanStack;
    typedef std::tr1::unordered_map<Resolvent, StronglyConnectedComponent, Hash<Resolvent> > StronglyConnectedComponentsByRepresentative;
    typedef std::tr1::unordered_map<Resolvent, Resolvent, Hash<Resolvent> > RepresentativeNodes;

    TarjanDataMap::iterator tarjan(const Resolvent & node, const Edges & edges, TarjanDataMap & data, TarjanStack & stack, int & index,
            StronglyConnectedComponentsByRepresentative & result)
    {
        TarjanDataMap::iterator node_data(data.insert(std::make_pair(node, make_named_values<TarjanData>(
                            n::index() = index,
                            n::lowlink() = index
                            ))).first);
        ++index;

        TarjanStack::iterator top_of_stack_before_node(stack.begin());
        stack.push_front(node);

        Edges::const_iterator e(edges.find(node));
        if (e != edges.end())
        {
            for (Nodes::const_iterator n(e->second.begin()), n_end(e->second.end()) ;
                    n != n_end ; ++n)
            {
                TarjanDataMap::iterator n_data(data.find(*n));
                if (data.end() == n_data)
                {
                    n_data = tarjan(*n, edges, data, stack, index, result);
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.lowlink());
                }
                else if (stack.end() != std::find(stack.begin(), stack.end(), *n))
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.index());
            }
        }

        if (node_data->second.index() == node_data->second.lowlink())
        {
            StronglyConnectedComponent scc(make_named_values<StronglyConnectedComponent>(
                        n::nodes() = make_shared_ptr(new Set<Resolvent>),
                        n::requirements() = make_shared_ptr(new Set<Resolvent>)
                        ));

            std::copy(stack.begin(), top_of_stack_before_node, scc.nodes()->inserter());
            stack.erase(stack.begin(), top_of_stack_before_node);

            if (scc.nodes()->empty())
                throw InternalError(PALUDIS_HERE, "empty scc");
            result.insert(std::make_pair(*scc.nodes()->begin(), scc));
        }

        return node_data;
    }

    void dfs(
            const StronglyConnectedComponentsByRepresentative & sccs,
            const Resolvent & node,
            const Edges & edges,
            Nodes & done,
            std::tr1::shared_ptr<SortedStronglyConnectedComponents> & result)
    {
        if (done.end() != done.find(node))
            return;

        Edges::const_iterator e(edges.find(node));
        std::set<Resolvent> consistently_ordered_edges;
        if (e != edges.end())
            std::copy(e->second.begin(), e->second.end(), std::inserter(consistently_ordered_edges, consistently_ordered_edges.begin()));

        for (std::set<Resolvent>::const_iterator n(consistently_ordered_edges.begin()), n_end(consistently_ordered_edges.end()) ;
                n != n_end ; ++n)
            dfs(sccs, *n, edges, done, result);

        done.insert(node);
        result->push_back(sccs.find(node)->second);
    }
}

const std::tr1::shared_ptr<const SortedStronglyConnectedComponents>
NAG::sorted_strongly_connected_components() const
{
    StronglyConnectedComponentsByRepresentative sccs;
    TarjanDataMap data;
    TarjanStack stack;

    /* find our strongly connected components */
    int index(0);
    for (Nodes::const_iterator n(_imp->nodes.begin()), n_end(_imp->nodes.end()) ;
            n != n_end ; ++n)
        if (data.end() == data.find(*n))
            tarjan(*n, _imp->edges, data, stack, index, sccs);

    /* find the 'representative' scc node for every node */
    RepresentativeNodes representative_nodes;
    for (StronglyConnectedComponentsByRepresentative::const_iterator s(sccs.begin()), s_end(sccs.end()) ;
            s != s_end ; ++s)
        for (Set<Resolvent>::ConstIterator r(s->second.nodes()->begin()), r_end(s->second.nodes()->end()) ;
                r != r_end ; ++r)
            if (! representative_nodes.insert(std::make_pair(*r, s->first)).second)
                throw InternalError(PALUDIS_HERE, "node in multiple sccs");

    /* sanity check, to avoid us much weirdness if there's a bug */
    if (representative_nodes.size() != _imp->nodes.size())
        throw InternalError(PALUDIS_HERE, "mismatch");

    /* build edges between SCCs */
    Edges scc_edges;
    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        RepresentativeNodes::const_iterator from(representative_nodes.find(e->first));
        for (Nodes::const_iterator n(e->second.begin()), n_end(e->second.end()) ;
                n != n_end ; ++n)
        {
            RepresentativeNodes::const_iterator to(representative_nodes.find(*n));
            if (! (to->second == from->second))
                scc_edges.insert(std::make_pair(from->second, Nodes())).first->second.insert(to->second);
        }
    }

    /* topological sort with consistent ordering (mostly to make test cases
     * easier). we know there're no cycles. */
    std::tr1::shared_ptr<SortedStronglyConnectedComponents> result(new SortedStronglyConnectedComponents);
    Nodes done;
    std::set<Resolvent> consistently_ordered_representative_nodes;
    std::copy(first_iterator(sccs.begin()), first_iterator(sccs.end()),
            std::inserter(consistently_ordered_representative_nodes, consistently_ordered_representative_nodes.begin()));
    for (std::set<Resolvent>::const_iterator n(consistently_ordered_representative_nodes.begin()),
            n_end(consistently_ordered_representative_nodes.end()) ;
            n != n_end ; ++n)
        dfs(sccs, *n, scc_edges, done, result);

    if (done.size() != sccs.size())
        throw InternalError(PALUDIS_HERE, "mismatch");

    return result;
}

NAG::EdgesFromConstIterator
NAG::begin_edges_from(const Resolvent & r) const
{
    Edges::const_iterator e(_imp->edges.find(r));
    if (e == _imp->edges.end())
        return EdgesFromConstIterator(_imp->nodes.end());
    else
        return EdgesFromConstIterator(e->second.begin());
}

NAG::EdgesFromConstIterator
NAG::end_edges_from(const Resolvent & r) const
{
    Edges::const_iterator e(_imp->edges.find(r));
    if (e == _imp->edges.end())
        return EdgesFromConstIterator(_imp->nodes.end());
    else
        return EdgesFromConstIterator(e->second.end());
}

template class WrappedForwardIterator<NAG::EdgesFromConstIteratorTag, const Resolvent>;

