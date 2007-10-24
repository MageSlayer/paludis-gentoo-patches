/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include <paludis/util/damerau_levenshtein.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_memory.hh>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<DamerauLevenshtein>
    {
        std::string name;
        unsigned n;
        tr1::shared_ptr<std::vector<unsigned> > prevprev;
        tr1::shared_ptr<std::vector<unsigned> > prev;
        tr1::shared_ptr<std::vector<unsigned> > current;

        Implementation(const std::string & myname) :
            name(myname), n(name.length() + 1), prevprev(new std::vector<unsigned>(n)),
            prev(new std::vector<unsigned>(n)), current(new std::vector<unsigned>(n))
        {
        }
    };
}

DamerauLevenshtein::DamerauLevenshtein(const std::string & name) :
    PrivateImplementationPattern<DamerauLevenshtein>(new Implementation<DamerauLevenshtein>(name))
{
}

DamerauLevenshtein::~DamerauLevenshtein()
{
}

unsigned
DamerauLevenshtein::distance_with(const std::string & candidate)
{
    for (unsigned i(0) ; i < _imp->n ; ++i)
        (*_imp->prev)[i]  = i;
    _imp->prevprev->assign(0, _imp->n);
    _imp->current->assign(0, _imp->n);

    size_t m(candidate.length() + 1);
    for (unsigned i(1) ; i < m ; ++i)
    {
        (*_imp->current)[0] = i;
        for (unsigned j(1) ; j < _imp->n ; ++j)
        {
            unsigned cost(candidate[i - 1] == _imp->name[j - 1] ? 0 : 1);
            (*_imp->current)[j] = std::min(
                    std::min((*_imp->prev)[j] + 1, (*_imp->current)[j - 1] + 1),
                    (*_imp->prev)[j - 1] + cost);
            if (i > 1 && j > 1
                    && candidate[i - 1] == _imp->name[j - 2]
                    && candidate[i - 2] == _imp->name[j - 1])
                (*_imp->current)[j] = std::min((*_imp->current)[j], (*_imp->prevprev)[j - 2] + cost);
        }
        tr1::shared_ptr<std::vector<unsigned> > aux(_imp->current);
        tr1::shared_ptr<std::vector<unsigned> > aux2(_imp->prev);
        _imp->current = _imp->prevprev;
        _imp->prev = aux;
        _imp->prevprev = aux2;
    }

    return (*_imp->prev)[_imp->n - 1];
}

