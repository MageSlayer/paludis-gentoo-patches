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
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/serialise-impl.hh>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <algorithm>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

#include <paludis/resolver/nag-se.cc>

typedef std::tr1::unordered_set<NAGIndex, Hash<NAGIndex> > Nodes;
typedef std::tr1::unordered_map<NAGIndex, NAGEdgeProperties, Hash<NAGIndex> > NodesWithProperties;
typedef std::tr1::unordered_map<NAGIndex, NodesWithProperties, Hash<NAGIndex> > Edges;
typedef std::tr1::unordered_map<NAGIndex, Nodes, Hash<NAGIndex> > PlainEdges;

std::size_t
NAGIndex::hash() const
{
    return resolvent().hash();
}

bool
paludis::resolver::operator< (const NAGIndex & a, const NAGIndex & b)
{
    if (a.resolvent() < b.resolvent())
        return true;
    if (b.resolvent() < a.resolvent())
        return false;
    return a.role() < b.role();
}

bool
paludis::resolver::operator== (const NAGIndex & a, const NAGIndex & b)
{
    return a.resolvent() == b.resolvent() && a.role() == b.role();
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const NAGIndex & r)
{
    s << r.role() << " " << r.resolvent();
    return s;
}

void
NAGIndex::serialise(Serialiser & s) const
{
    s.object("NAGIndex")
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        .member(SerialiserFlags<>(), "role", stringify(role()))
        ;
}

const NAGIndex
NAGIndex::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "NAGIndex");

    return make_named_values<NAGIndex>(
            n::resolvent() = v.member<Resolvent>("resolvent"),
            n::role() = destringify<NAGIndexRole>(v.member<std::string>("role"))
            );
}

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
        const NodesWithProperties empty_nodes_with_properties;
    };

    template <>
    struct WrappedForwardIteratorTraits<NAG::EdgesFromConstIteratorTag>
    {
        typedef NodesWithProperties::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<NAG::NodesConstIteratorTag>
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
NAG::add_node(const NAGIndex & r)
{
    _imp->nodes.insert(r);
}

void
NAG::add_edge(const NAGIndex & a, const NAGIndex & b, const NAGEdgeProperties & p)
{
    _imp->edges.insert(std::make_pair(a, NodesWithProperties())).first->second.insert(std::make_pair(b, p)).first->second |= p;
}

void
NAG::verify_edges() const
{
    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        if (_imp->nodes.end() == _imp->nodes.find(e->first))
            throw InternalError(PALUDIS_HERE, "Missing node for edge " + stringify(e->first));

        for (NodesWithProperties::const_iterator f(e->second.begin()), f_end(e->second.end()) ;
                f != f_end ; ++f)
            if (_imp->nodes.end() == _imp->nodes.find(f->first))
                throw InternalError(PALUDIS_HERE, "Missing node for edge " + stringify(e->first) + " -> " + stringify(f->first));
    }
}

namespace
{
    struct TarjanData
    {
        NamedValue<n::index, int> index;
        NamedValue<n::lowlink, int> lowlink;
    };

    typedef std::tr1::unordered_map<NAGIndex, TarjanData, Hash<NAGIndex> > TarjanDataMap;
    typedef std::list<NAGIndex> TarjanStack;
    typedef std::tr1::unordered_map<NAGIndex, StronglyConnectedComponent, Hash<NAGIndex> > StronglyConnectedComponentsByRepresentative;
    typedef std::tr1::unordered_map<NAGIndex, NAGIndex, Hash<NAGIndex> > RepresentativeNodes;

    TarjanDataMap::iterator tarjan(const NAGIndex & node, const Edges & edges, TarjanDataMap & data, TarjanStack & stack, int & index,
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
            for (NodesWithProperties::const_iterator n(e->second.begin()), n_end(e->second.end()) ;
                    n != n_end ; ++n)
            {
                TarjanDataMap::iterator n_data(data.find(n->first));
                if (data.end() == n_data)
                {
                    n_data = tarjan(n->first, edges, data, stack, index, result);
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.lowlink());
                }
                else if (stack.end() != std::find(stack.begin(), stack.end(), n->first))
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.index());
            }
        }

        if (node_data->second.index() == node_data->second.lowlink())
        {
            StronglyConnectedComponent scc(make_named_values<StronglyConnectedComponent>(
                        n::nodes() = make_shared_ptr(new Set<NAGIndex>),
                        n::requirements() = make_shared_ptr(new Set<NAGIndex>)
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
            const NAGIndex & node,
            const PlainEdges & edges,
            Nodes & done,
            std::tr1::shared_ptr<SortedStronglyConnectedComponents> & result)
    {
        if (done.end() != done.find(node))
            return;

        PlainEdges::const_iterator e(edges.find(node));
        std::set<NAGIndex> consistently_ordered_edges;
        if (e != edges.end())
            std::copy(e->second.begin(), e->second.end(), std::inserter(consistently_ordered_edges, consistently_ordered_edges.begin()));

        for (std::set<NAGIndex>::const_iterator n(consistently_ordered_edges.begin()), n_end(consistently_ordered_edges.end()) ;
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
        for (Set<NAGIndex>::ConstIterator r(s->second.nodes()->begin()), r_end(s->second.nodes()->end()) ;
                r != r_end ; ++r)
            if (! representative_nodes.insert(std::make_pair(*r, s->first)).second)
                throw InternalError(PALUDIS_HERE, "node in multiple sccs");

    /* sanity check, to avoid us much weirdness if there's a bug */
    if (representative_nodes.size() != _imp->nodes.size())
        throw InternalError(PALUDIS_HERE, "mismatch");

    /* build edges between SCCs */
    PlainEdges scc_edges;
    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        RepresentativeNodes::const_iterator from(representative_nodes.find(e->first));
        for (NodesWithProperties::const_iterator n(e->second.begin()), n_end(e->second.end()) ;
                n != n_end ; ++n)
        {
            RepresentativeNodes::const_iterator to(representative_nodes.find(n->first));
            if (! (to->second == from->second))
                scc_edges.insert(std::make_pair(from->second, Nodes())).first->second.insert(to->second);
        }
    }

    /* topological sort with consistent ordering (mostly to make test cases
     * easier). we know there're no cycles. */
    std::tr1::shared_ptr<SortedStronglyConnectedComponents> result(new SortedStronglyConnectedComponents);
    Nodes done;
    std::set<NAGIndex> consistently_ordered_representative_nodes;
    std::copy(first_iterator(sccs.begin()), first_iterator(sccs.end()),
            std::inserter(consistently_ordered_representative_nodes, consistently_ordered_representative_nodes.begin()));
    for (std::set<NAGIndex>::const_iterator n(consistently_ordered_representative_nodes.begin()),
            n_end(consistently_ordered_representative_nodes.end()) ;
            n != n_end ; ++n)
        dfs(sccs, *n, scc_edges, done, result);

    if (done.size() != sccs.size())
        throw InternalError(PALUDIS_HERE, "mismatch");

    return result;
}

NAG::EdgesFromConstIterator
NAG::begin_edges_from(const NAGIndex & r) const
{
    Edges::const_iterator e(_imp->edges.find(r));
    if (e == _imp->edges.end())
        return EdgesFromConstIterator(_imp->empty_nodes_with_properties.end());
    else
        return EdgesFromConstIterator(e->second.begin());
}

NAG::EdgesFromConstIterator
NAG::end_edges_from(const NAGIndex & r) const
{
    Edges::const_iterator e(_imp->edges.find(r));
    if (e == _imp->edges.end())
        return EdgesFromConstIterator(_imp->empty_nodes_with_properties.end());
    else
        return EdgesFromConstIterator(e->second.end());
}

NAG::NodesConstIterator
NAG::begin_nodes() const
{
    return NodesConstIterator(_imp->nodes.begin());
}

NAG::NodesConstIterator
NAG::end_nodes() const
{
    return NodesConstIterator(_imp->nodes.end());
}

void
NAG::serialise(Serialiser & s) const
{
    SerialiserObjectWriter w(s.object("NAG"));
    w.member(SerialiserFlags<serialise::container>(), "nodes", _imp->nodes);

    int c(0);
    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        for (NodesWithProperties::const_iterator n(e->second.begin()), n_end(e->second.end()) ;
                n != n_end ; ++n)
        {
            ++c;
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".f", e->first);
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".t", n->first);
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".p", n->second);
        }
    }

    w.member(SerialiserFlags<>(), "edge.count", stringify(c));
}

const std::tr1::shared_ptr<NAG>
NAG::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "NAG");
    std::tr1::shared_ptr<NAG> result(new NAG);

    {
        Deserialisator vv(*v.find_remove_member("nodes"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            result->add_node(vv.member<NAGIndex>(stringify(n)));
    }

    for (int n(1), n_end(v.member<int>("edge.count") + 1) ; n != n_end ; ++n)
        result->add_edge(
                v.member<NAGIndex>("edge." + stringify(n) + ".f"),
                v.member<NAGIndex>("edge." + stringify(n) + ".t"),
                v.member<NAGEdgeProperties>("edge." + stringify(n) + ".p")
                );

    result->verify_edges();
    return result;
}

NAGEdgeProperties &
NAGEdgeProperties::operator|= (const NAGEdgeProperties & other)
{
    build() |= other.build();
    run() |= other.run();
    return *this;
}

void
NAGEdgeProperties::serialise(Serialiser & s) const
{
    s.object("NAGEdgeProperties")
        .member(SerialiserFlags<>(), "build", build())
        .member(SerialiserFlags<>(), "build_all_met", build_all_met())
        .member(SerialiserFlags<>(), "run", run())
        .member(SerialiserFlags<>(), "run_all_met", run_all_met())
        ;
}

const NAGEdgeProperties
NAGEdgeProperties::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "NAGEdgeProperties");

    return make_named_values<NAGEdgeProperties>(
            n::build() = v.member<bool>("build"),
            n::build_all_met() = v.member<bool>("build_all_met"),
            n::run() = v.member<bool>("run"),
            n::run_all_met() = v.member<bool>("run_all_met")
            );
}

template class WrappedForwardIterator<NAG::EdgesFromConstIteratorTag, const std::pair<const NAGIndex, NAGEdgeProperties> >;
template class WrappedForwardIterator<NAG::NodesConstIteratorTag, const NAGIndex>;

template class Set<NAGIndex>;
template class WrappedForwardIterator<Set<NAGIndex>::ConstIteratorTag, const NAGIndex>;
template class WrappedOutputIterator<Set<NAGIndex>::InserterTag, NAGIndex>;

