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
#include <paludis/util/save.hh>
#include <stdint.h>
#include <vector>

using namespace paludis;

namespace
{
    unsigned size_needed_for(const unsigned s)
    {
        return 1 + ((s - 1) / 8);
    }
}

namespace paludis
{
    template<>
    struct Implementation<SimpleGraph>
    {
        std::vector<uint8_t> pool;
        const unsigned size;

        Implementation(unsigned sz) :
            pool(size_needed_for(sz * sz), 0),
            size(sz)
        {
        }
    };
}

SimpleGraph::SimpleGraph(const unsigned sz) :
    PrivateImplementationPattern<SimpleGraph>(new Implementation<SimpleGraph>(sz))
{
}

SimpleGraph::~SimpleGraph()
{
}

unsigned
SimpleGraph::size() const
{
    return _imp->size;
}

void
SimpleGraph::connect(const unsigned s, const unsigned d)
{
    const unsigned offset(s * _imp->size + d);
    _imp->pool[offset / 8] |= (1 << (offset % 8));
}

void
SimpleGraph::unconnect(const unsigned s, const unsigned d)
{
    const unsigned offset(s * _imp->size + d);
    _imp->pool[offset / 8] &= ~(1 << (offset % 8));
}

bool
SimpleGraph::is_connected(const unsigned s, const unsigned d) const
{
    const unsigned offset(s * _imp->size + d);
    return _imp->pool[offset / 8] & (1 << (offset % 8));
}

bool
SimpleGraph::has_outgoing(const unsigned s) const
{
    for (unsigned d(0) ; d < _imp->size ; ++d)
        if (is_connected(s, d))
            return true;

    return false;
}

bool
SimpleGraph::has_incoming(const unsigned s) const
{
    for (unsigned d(0) ; d < _imp->size ; ++d)
        if (is_connected(d, s))
            return true;

    return false;
}

void
SimpleGraph::reverse()
{
    for (unsigned s(0), s_end(_imp->pool.size()) ; s < s_end ; ++s)
        _imp->pool[s] = ~_imp->pool[s];

    for (unsigned s(0), s_end(_imp->size) ; s < s_end ; ++s)
        if (is_connected(s, s))
            unconnect(s, s);
        else
            connect(s, s);
}

void
SimpleGraph::clear_incoming(const unsigned s)
{
    for (unsigned d(0) ; d < _imp->size ; ++d)
        unconnect(d, s);
}

void
SimpleGraph::clear_outgoing(const unsigned s)
{
    for (unsigned d(0) ; d < _imp->size ; ++d)
        unconnect(s, d);
}

NoSimpleTopologicalOrderingExists::NoSimpleTopologicalOrderingExists(const unsigned & c) throw () :
    Exception("No topological ordering exists because of cycle on '" + stringify(c) + "'"),
    _cycle(c)
{
}

unsigned
NoSimpleTopologicalOrderingExists::cycle() const
{
    return _cycle;
}

namespace paludis
{
    template<>
    struct Implementation<SimpleTopologicalOrdering>
    {
        std::vector<unsigned> order;
        std::vector<uint8_t> pending;
        std::vector<uint8_t> done;

        Implementation(const unsigned s) :
            pending(s, 0),
            done(s, 0)
        {
            order.reserve(s);
        }
    };
}

SimpleTopologicalOrdering::SimpleTopologicalOrdering(const SimpleGraph & g) :
    PrivateImplementationPattern<SimpleTopologicalOrdering>(new Implementation<SimpleTopologicalOrdering>(g.size()))
{
    for (unsigned x(0), x_end(g.size()) ; x != x_end ; ++x)
        _dfs(g, x);
}

void
SimpleTopologicalOrdering::_dfs(const SimpleGraph & g, const unsigned node)
{
    if (_imp->pending[node])
        throw NoSimpleTopologicalOrderingExists(node);

    if (_imp->done[node])
        return;

    Save<uint8_t> save_pending(&_imp->pending[node], 1);

    for (unsigned x(0), x_end(g.size()) ; x != x_end ; ++x)
        if (g.is_connected(node, x))
            _dfs(g, x);

    _imp->order.push_back(node);
    _imp->done[node] = 1;
}

SimpleTopologicalOrdering::~SimpleTopologicalOrdering()
{
}

SimpleTopologicalOrdering::Iterator
SimpleTopologicalOrdering::begin() const
{
    return Iterator(_imp->order.begin());
}

SimpleTopologicalOrdering::Iterator
SimpleTopologicalOrdering::end() const
{
    return Iterator(_imp->order.end());
}

