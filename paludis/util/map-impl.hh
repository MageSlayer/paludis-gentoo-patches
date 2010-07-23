/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <map>
#include <iterator>
#include <functional>

/** \file
 * Imp for paludis/util/map.hh .
 *
 * \ingroup g_data_structures
 */

namespace paludis
{
    /**
     * The default comparator for a Map is std::less<>.
     *
     * \ingroup g_data_structures
     */
    template <typename T_>
    struct PALUDIS_VISIBLE DefaultMapComparator :
        std::less<T_>
    {
    };

    /**
     * Imp data for a Map.
     *
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename K_, typename V_, typename C_>
    struct Imp<Map<K_, V_, C_> >
    {
        std::map<K_, V_, C_> map;
    };

    template <typename K_, typename V_, typename C_>
    struct WrappedForwardIteratorTraits<MapConstIteratorTag<K_, V_, C_> >
    {
        typedef typename std::map<K_, V_, C_>::const_iterator UnderlyingIterator;
    };

    template <typename K_, typename V_, typename C_>
    struct WrappedOutputIteratorTraits<MapInserterTag<K_, V_, C_> >
    {
        typedef std::insert_iterator<std::map<K_, V_, C_> > UnderlyingIterator;
    };
}

template <typename K_, typename V_, typename C_>
paludis::Map<K_, V_, C_>::Map() :
    paludis::Pimp<paludis::Map<K_, V_, C_> >()
{
}

template <typename K_, typename V_, typename C_>
paludis::Map<K_, V_, C_>::~Map()
{
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::ConstIterator
paludis::Map<K_, V_, C_>::begin() const
{
    return ConstIterator(_imp->map.begin());
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::ConstIterator
paludis::Map<K_, V_, C_>::end() const
{
    return ConstIterator(_imp->map.end());
}

template <typename K_, typename V_, typename C_>
typename paludis::Map<K_, V_, C_>::ConstIterator
paludis::Map<K_, V_, C_>::find(const K_ & k) const
{
    return ConstIterator(_imp->map.find(k));
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
paludis::Map<K_, V_, C_>::erase(const typename paludis::Map<K_, V_, C_>::ConstIterator & i)
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
