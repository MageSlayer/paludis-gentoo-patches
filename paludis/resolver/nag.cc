/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tribool.hh>
#include <paludis/serialise-impl.hh>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <list>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

#include <paludis/resolver/nag-se.cc>

typedef std::unordered_set<NAGIndex, Hash<NAGIndex> > Nodes;
typedef std::unordered_map<NAGIndex, NAGEdgeProperties, Hash<NAGIndex> > NodesWithProperties;
typedef std::unordered_map<NAGIndex, NodesWithProperties, Hash<NAGIndex> > Edges;
typedef std::unordered_map<NAGIndex, Nodes, Hash<NAGIndex> > PlainEdges;

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
        typedef Name<struct name_index> index;
        typedef Name<struct name_lowlink> lowlink;
    }

    template <>
    struct Imp<NAG>
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
    _imp()
{
}

NAG::~NAG() = default;

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
    Context context("When verifying NAG edges:");

    for (Edges::const_iterator e(_imp->edges.begin()), e_end(_imp->edges.end()) ;
            e != e_end ; ++e)
    {
        if (_imp->nodes.end() == _imp->nodes.find(e->first))
            throw InternalError(PALUDIS_HERE, "Missing node for edge '" + stringify(e->first)
                    + "' to { '" + join(first_iterator(e->second.begin()), first_iterator(e->second.end()), "', '")
                    + " }' in nodes { " + join(_imp->nodes.begin(), _imp->nodes.end(), ", ") + " }");

        for (NodesWithProperties::const_iterator f(e->second.begin()), f_end(e->second.end()) ;
                f != f_end ; ++f)
            if (_imp->nodes.end() == _imp->nodes.find(f->first))
                throw InternalError(PALUDIS_HERE, "Missing node for edge '" + stringify(e->first) + "' -> '" + stringify(f->first) + "' in nodes { "
                        + join(_imp->nodes.begin(), _imp->nodes.end(), ", ") + " }");
    }
}

namespace
{
    struct TarjanData
    {
        NamedValue<n::index, int> index;
        NamedValue<n::lowlink, int> lowlink;
    };

    typedef std::unordered_map<NAGIndex, TarjanData, Hash<NAGIndex> > TarjanDataMap;
    typedef std::list<NAGIndex> TarjanStack;
    typedef std::unordered_map<NAGIndex, StronglyConnectedComponent, Hash<NAGIndex> > StronglyConnectedComponentsByRepresentative;
    typedef std::unordered_map<NAGIndex, NAGIndex, Hash<NAGIndex> > RepresentativeNodes;

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
            for (const auto & n : e->second)
            {
                TarjanDataMap::iterator n_data(data.find(n.first));
                if (data.end() == n_data)
                {
                    n_data = tarjan(n.first, edges, data, stack, index, result);
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.lowlink());
                }
                else if (stack.end() != std::find(stack.begin(), stack.end(), n.first))
                    node_data->second.lowlink() = std::min(node_data->second.lowlink(), n_data->second.index());
            }
        }

        if (node_data->second.index() == node_data->second.lowlink())
        {
            StronglyConnectedComponent scc(make_named_values<StronglyConnectedComponent>(
                        n::nodes() = std::make_shared<Set<NAGIndex>>(),
                        n::requirements() = std::make_shared<Set<NAGIndex>>()
                        ));

            std::copy(stack.begin(), top_of_stack_before_node, scc.nodes()->inserter());
            stack.erase(stack.begin(), top_of_stack_before_node);

            if (scc.nodes()->empty())
                throw InternalError(PALUDIS_HERE, "empty scc");
            result.insert(std::make_pair(*scc.nodes()->begin(), scc));
        }

        return node_data;
    }

    int order_score_one(const NAGIndex & n, const std::function<Tribool (const NAGIndex &)> & order_early_fn)
    {
        /* lower scores are 'better' and mean 'order earlier' */
        Tribool order_early(order_early_fn(n));
        int bias(0);
        if (order_early.is_indeterminate())
            bias = 1;
        else if (order_early.is_false())
            bias = 2;

        switch (n.role())
        {
            case nir_fetched:
                return 10 + bias;

            case nir_done:
                return 20 + bias;

            case last_nir:
                break;
        }

        throw InternalError(PALUDIS_HERE, "bad nir");
    }

    std::pair<int, NAGIndex> order_score(const NAGIndex & r, const StronglyConnectedComponent & scc,
            const std::function<Tribool (const NAGIndex &)> & order_early_fn)
    {
        int best_score(-1);

        for (Set<NAGIndex>::ConstIterator e(scc.nodes()->begin()), e_end(scc.nodes()->end()) ;
                e != e_end ; ++e)
        {
            int score(order_score_one(*e, order_early_fn));
            if (best_score == -1 || score < best_score)
                best_score = score;
        }

        return std::make_pair(best_score, r);
    }
}

const std::shared_ptr<const SortedStronglyConnectedComponents>
NAG::sorted_strongly_connected_components(
        const std::function<Tribool (const NAGIndex &)> & order_early_fn
        ) const
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
    PlainEdges all_scc_edges;
    PlainEdges scc_edges;
    PlainEdges scc_edges_backwards;
    for (const auto & edge : _imp->edges)
    {
        RepresentativeNodes::const_iterator from(representative_nodes.find(edge.first));
        for (const auto & n : edge.second)
        {
            RepresentativeNodes::const_iterator to(representative_nodes.find(n.first));
            if (! (to->second == from->second))
            {
                all_scc_edges.insert(std::make_pair(from->second, Nodes())).first->second.insert(to->second);
                scc_edges.insert(std::make_pair(from->second, Nodes())).first->second.insert(to->second);
                scc_edges_backwards.insert(std::make_pair(to->second, Nodes())).first->second.insert(from->second);
            }
        }
    }

    /* topological sort with consistent ordering (mostly to make test cases
     * easier). we know there're no cycles. */
    std::shared_ptr<SortedStronglyConnectedComponents> result(std::make_shared<SortedStronglyConnectedComponents>());

    typedef std::set<std::pair<int, NAGIndex> > OrderableNow;
    OrderableNow orderable_now;
    Nodes done;
    Nodes pending_fetches;

    for (StronglyConnectedComponentsByRepresentative::const_iterator c(sccs.begin()), c_end(sccs.end()) ;
            c != c_end ; ++c)
        if (scc_edges.end() == scc_edges.find(c->first))
            orderable_now.insert(order_score(c->first, c->second, order_early_fn));

    while (! orderable_now.empty())
    {
        OrderableNow::iterator ordering_now(orderable_now.begin());
        StronglyConnectedComponentsByRepresentative::const_iterator ordering_now_scc(sccs.find(ordering_now->second));

        if (ordering_now_scc->second.nodes()->size() == 1 && ordering_now_scc->second.nodes()->begin()->role() == nir_fetched)
            pending_fetches.insert(ordering_now->second);
        else
        {
            auto this_scc_edges(all_scc_edges.find(ordering_now->second));
            if (this_scc_edges != all_scc_edges.end())
            {
                for (const auto & e : this_scc_edges->second)
                {
                    auto p(pending_fetches.find(e));
                    if (p != pending_fetches.end())
                    {
                        result->push_back(sccs.find(e)->second);
                        pending_fetches.erase(p);
                    }
                }
            }

            result->push_back(ordering_now_scc->second);
        }
        done.insert(ordering_now->second);

        PlainEdges::iterator ordering_now_edges(scc_edges_backwards.find(ordering_now->second));
        if (ordering_now_edges != scc_edges_backwards.end())
            for (Nodes::iterator e(ordering_now_edges->second.begin()), e_end(ordering_now_edges->second.end()) ;
                    e != e_end ; )
            {
                PlainEdges::iterator reverse_edges(scc_edges.find(*e));
                if (reverse_edges == scc_edges.end())
                    throw InternalError(PALUDIS_HERE, "huh?");
                reverse_edges->second.erase(ordering_now->second);
                if (reverse_edges->second.empty())
                    orderable_now.insert(order_score(*e, sccs.find(*e)->second, order_early_fn));
                ordering_now_edges->second.erase(e++);
            }

        orderable_now.erase(ordering_now);
    }

    if (! pending_fetches.empty())
        throw InternalError(PALUDIS_HERE, "still have pending fetches");

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

NAG::NodesConstIterator
NAG::find_node(const NAGIndex & x) const
{
    return NodesConstIterator(_imp->nodes.find(x));
}

void
NAG::serialise(Serialiser & s) const
{
    SerialiserObjectWriter w(s.object("NAG"));
    w.member(SerialiserFlags<serialise::container>(), "nodes", _imp->nodes);

    int c(0);
    for (const auto & edge : _imp->edges)
    {
        for (NodesWithProperties::const_iterator n(edge.second.begin()), n_end(edge.second.end()) ;
                n != n_end ; ++n)
        {
            ++c;
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".f", edge.first);
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".t", n->first);
            w.member(SerialiserFlags<>(), "edge." + stringify(c) + ".p", n->second);
        }
    }

    w.member(SerialiserFlags<>(), "edge.count", stringify(c));
}

const std::shared_ptr<NAG>
NAG::deserialise(Deserialisation & d)
{
    Context context("When deserialising NAG:");

    Deserialisator v(d, "NAG");
    std::shared_ptr<NAG> result(std::make_shared<NAG>());

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
    always() |= other.always();
    build() |= other.build();
    build_all_met() &= other.build_all_met();
    run() |= other.run();
    run_all_met() &= other.run_all_met();
    return *this;
}

void
NAGEdgeProperties::serialise(Serialiser & s) const
{
    s.object("NAGEdgeProperties")
        .member(SerialiserFlags<>(), "always", always())
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
            n::always() = v.member<bool>("always"),
            n::build() = v.member<bool>("build"),
            n::build_all_met() = v.member<bool>("build_all_met"),
            n::run() = v.member<bool>("run"),
            n::run_all_met() = v.member<bool>("run_all_met")
            );
}

namespace paludis
{
    template class WrappedForwardIterator<NAG::EdgesFromConstIteratorTag, const std::pair<const NAGIndex, NAGEdgeProperties> >;
    template class WrappedForwardIterator<NAG::NodesConstIteratorTag, const NAGIndex>;

    template class Set<NAGIndex>;
    template class WrappedForwardIterator<Set<NAGIndex>::ConstIteratorTag, const NAGIndex>;
    template class WrappedOutputIterator<Set<NAGIndex>::InserterTag, NAGIndex>;
}
