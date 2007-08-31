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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAP_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAP_IMPL_HH 1

#include <paludis/util/map.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <map>
#include <iterator>
#include <functional>

namespace paludis
{
    template <typename T_>
    struct PALUDIS_VISIBLE DefaultMapComparator :
        std::less<T_>
    {
    };

    template <>
    template <typename K_, typename V_, typename C_>
    struct Implementation<Map<K_, V_, C_> >
    {
        std::map<K_, V_, C_> map;
    };
}

template <typename K_, typename V_, typename C_>
paludis::Map<K_, V_, C_>::Map() :
    paludis::PrivateImplementationPattern<paludis::Map<K_, V_, C_> >(new paludis::Implementation<paludis::Map<K_, V_, C_> >)
{
}

template <typename K_, typename V_, typename C_>
paludis::Map<K_, V_, C_>::~Map()
{
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::Iterator
paludis::Map<K_, V_, C_>::begin() const
{
    return Iterator(_imp->map.begin());
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::Iterator
paludis::Map<K_, V_, C_>::end() const
{
    return Iterator(_imp->map.end());
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::Iterator
paludis::Map<K_, V_, C_>::find(const K_ & k) const
{
    return Iterator(_imp->map.find(k));
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::Inserter
paludis::Map<K_, V_, C_>::inserter()
{
    return Inserter(std::inserter(_imp->map, _imp->map.begin()));
}

template <typename K_, typename V_, typename C_>
bool
paludis::Map<K_, V_, C_>::empty() const
{
    return _imp->map.empty();
}

template <typename K_, typename V_, typename C_>
unsigned
paludis::Map<K_, V_, C_>::size() const
{
    return _imp->map.size();
}

template <typename K_, typename V_, typename C_>
void
paludis::Map<K_, V_, C_>::insert(const K_ & k, const V_ & v)
{
    _imp->map.insert(std::make_pair(k, v));
}

template <typename K_, typename V_, typename C_>
void
paludis::Map<K_, V_, C_>::erase(const typename paludis::Map<K_, V_, C_>::Iterator & i)
{
    _imp->map.erase(i->first);
}

template <typename K_, typename V_, typename C_>
void
paludis::Map<K_, V_, C_>::erase(const K_ & i)
{
    _imp->map.erase(i);
}

#endif
