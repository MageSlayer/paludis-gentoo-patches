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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SET_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SET_IMPL_HH 1

#include <paludis/util/set.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

#include <set>
#include <functional>
#include <iterator>

namespace paludis
{
    template <typename T_>
    struct PALUDIS_VISIBLE DefaultSetComparator :
        std::less<T_>
    {
    };

    template <>
    template <typename T_, typename C_>
    struct Implementation<Set<T_, C_> >
    {
        std::set<T_, C_> set;
    };
}

template <typename T_, typename C_>
paludis::Set<T_, C_>::Set() :
    paludis::PrivateImplementationPattern<Set<T_, C_> >(new Implementation<Set<T_, C_> >)
{
}

template <typename T_, typename C_>
paludis::Set<T_, C_>::~Set()
{
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::Iterator
paludis::Set<T_, C_>::begin() const
{
    return Iterator(_imp->set.begin());
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::Iterator
paludis::Set<T_, C_>::end() const
{
    return Iterator(_imp->set.end());
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::Iterator
paludis::Set<T_, C_>::find(const T_ & t) const
{
    return Iterator(_imp->set.find(t));
}

template <typename T_, typename C_>
bool
paludis::Set<T_, C_>::empty() const
{
    return _imp->set.empty();
}

template <typename T_, typename C_>
unsigned
paludis::Set<T_, C_>::size() const
{
    return _imp->set.size();
}

template <typename T_, typename C_>
unsigned
paludis::Set<T_, C_>::count(const T_ & t) const
{
    return _imp->set.count(t);
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::Inserter
paludis::Set<T_, C_>::inserter()
{
    return Inserter(std::inserter(_imp->set, _imp->set.begin()));
}

template <typename T_, typename C_>
void
paludis::Set<T_, C_>::insert(const T_ & t)
{
    _imp->set.insert(t);
}

#endif
