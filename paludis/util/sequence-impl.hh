/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SEQUENCE_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SEQUENCE_IMPL_HH 1

#include <paludis/util/sequence.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <list>
#include <iterator>

/** \file
 * Imp for paludis/util/sequence.hh .
 *
 * \ingroup g_data_structures
 */

namespace paludis
{
    /**
     * Imp data for a Sequence.
     *
     * \ingroup g_data_structures
     * \nosubgrouping
     */
    template <typename T_>
    struct Imp<Sequence<T_> >
    {
        std::list<T_> list;
    };

    template <typename T_>
    struct WrappedForwardIteratorTraits<SequenceConstIteratorTag<T_> >
    {
        typedef typename std::list<T_>::const_iterator UnderlyingIterator;
    };

    template <typename T_>
    struct WrappedForwardIteratorTraits<SequenceReverseConstIteratorTag<T_> >
    {
        typedef typename std::list<T_>::const_reverse_iterator UnderlyingIterator;
    };

    template <typename T_>
    struct WrappedOutputIteratorTraits<SequenceInserterTag<T_> >
    {
        typedef std::back_insert_iterator<std::list<T_> > UnderlyingIterator;
    };
}

template <typename T_>
paludis::Sequence<T_>::Sequence() :
    _imp()
{
}

template <typename T_>
paludis::Sequence<T_>::~Sequence()
{
}

template <typename T_>
typename paludis::Sequence<T_>::ConstIterator
paludis::Sequence<T_>::begin() const
{
    return ConstIterator(_imp->list.begin());
}

template <typename T_>
typename paludis::Sequence<T_>::ConstIterator
paludis::Sequence<T_>::end() const
{
    return ConstIterator(_imp->list.end());
}

template <typename T_>
typename paludis::Sequence<T_>::ConstIterator
paludis::Sequence<T_>::last() const
{
    return ConstIterator(_imp->list.begin() == _imp->list.end() ? _imp->list.end() : --(_imp->list.end()));
}

template <typename T_>
typename paludis::Sequence<T_>::ReverseConstIterator
paludis::Sequence<T_>::rbegin() const
{
    return ReverseConstIterator(_imp->list.rbegin());
}

template <typename T_>
typename paludis::Sequence<T_>::ReverseConstIterator
paludis::Sequence<T_>::rend() const
{
    return ReverseConstIterator(_imp->list.rend());
}

template <typename T_>
typename paludis::Sequence<T_>::Inserter
paludis::Sequence<T_>::back_inserter()
{
    return Inserter(std::back_inserter(_imp->list));
}

template <typename T_>
void
paludis::Sequence<T_>::push_back(const T_ & t)
{
    _imp->list.push_back(t);
}

template <typename T_>
void
paludis::Sequence<T_>::push_front(const T_ & t)
{
    _imp->list.push_front(t);
}

template <typename T_>
void
paludis::Sequence<T_>::pop_back()
{
    _imp->list.pop_back();
}

template <typename T_>
void
paludis::Sequence<T_>::pop_front()
{
    _imp->list.pop_front();
}

template <typename T_>
bool
paludis::Sequence<T_>::empty() const
{
    return _imp->list.empty();
}

template <typename T_>
template <typename C_>
void
paludis::Sequence<T_>::sort(const C_ & c)
{
    _imp->list.template sort<const C_ &>(c);
}

#endif
