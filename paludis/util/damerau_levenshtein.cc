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
#include <paludis/util/pimp-impl.hh>
#include <memory>
#include <vector>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<DamerauLevenshtein>
    {
        std::string name;
        unsigned n;

        Imp(const std::string & myname) :
            name(myname), n(name.length() + 1)
        {
        }
    };
}

DamerauLevenshtein::DamerauLevenshtein(const std::string & name) :
    _imp(name)
{
}

DamerauLevenshtein::~DamerauLevenshtein()
{
}

unsigned
DamerauLevenshtein::distance_with(const std::string & candidate) const
{
    std::vector<unsigned> prevprev(_imp->n, 0);
    std::vector<unsigned> prev(_imp->n);
    std::vector<unsigned> current(_imp->n, 0);

    for (unsigned i(0) ; i < _imp->n ; ++i)
        prev[i] = i;

    size_t m(candidate.length() + 1);
    for (unsigned i(1) ; i < m ; ++i)
    {
        current[0] = i;
        for (unsigned j(1) ; j < _imp->n ; ++j)
        {
            unsigned cost(candidate[i - 1] == _imp->name[j - 1] ? 0 : 1);
            current[j] = std::min(
                    std::min(prev[j] + 1, current[j - 1] + 1),
                    prev[j - 1] + cost);
            if (i > 1 && j > 1
                    && candidate[i - 1] == _imp->name[j - 2]
                    && candidate[i - 2] == _imp->name[j - 1])
                current[j] = std::min(current[j], prevprev[j - 2] + cost);
        }
        prevprev.swap(current);
        prevprev.swap(prev);
    }

    return prev[_imp->n - 1];
}

template class Pimp<DamerauLevenshtein>;

