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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SET_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SET_IMPL_HH 1

#include <paludis/util/set.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>

#include <set>
#include <functional>
#include <iterator>

/** \file
 * Imp for paludis/util/set.hh .
 *
 * \ingroup g_data_structures
 */

namespace paludis
{
    /**
     * The default comparator for a Set is std::less<>.
     *
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename T_>
    struct PALUDIS_VISIBLE DefaultSetComparator :
        std::less<T_>
    {
    };

    /**
     * Imp data for a Set.
     *
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename T_, typename C_>
    struct Imp<Set<T_, C_> >
    {
        std::set<T_, C_> set;
    };

    template <typename T_, typename C_>
    struct WrappedForwardIteratorTraits<SetConstIteratorTag<T_, C_> >
    {
        typedef typename std::set<T_, C_>::const_iterator UnderlyingIterator;
    };

    template <typename T_, typename C_>
    struct WrappedOutputIteratorTraits<SetInsertIteratorTag<T_, C_> >
    {
        typedef std::insert_iterator<std::set<T_, C_> > UnderlyingIterator;
    };
}

template <typename T_, typename C_>
paludis::Set<T_, C_>::Set() :
    paludis::Pimp<Set<T_, C_> >()
{
}

template <typename T_, typename C_>
paludis::Set<T_, C_>::~Set()
{
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::ConstIterator
paludis::Set<T_, C_>::begin() const
{
    return ConstIterator(_imp->set.begin());
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::ConstIterator
paludis::Set<T_, C_>::end() const
{
    return ConstIterator(_imp->set.end());
}

template <typename T_, typename C_>
typename paludis::Set<T_, C_>::ConstIterator
paludis::Set<T_, C_>::find(const T_ & t) const
{
    return ConstIterator(_imp->set.find(t));
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

template <typename T_, typename C_>
void
paludis::Set<T_, C_>::erase(const T_ & t)
{
    _imp->set.erase(t);
}

template <typename T_, typename C_>
void
paludis::Set<T_, C_>::clear()
{
    _imp->set.clear();
}

#endif
