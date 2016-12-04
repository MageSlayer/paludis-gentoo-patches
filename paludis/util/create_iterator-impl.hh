/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_CREATE_ITERATOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_CREATE_ITERATOR_IMPL_HH 1

#include <paludis/util/create_iterator.hh>

namespace paludis
{
    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>::CreateInsertIterator() :
        _iter()
    {
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>::~CreateInsertIterator() = default;

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>::CreateInsertIterator(const CreateInsertIterator & i) :
        _iter(i._iter)
    {
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>::CreateInsertIterator(const Iter_ & i) :
        _iter(i)
    {
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_> &
    CreateInsertIterator<Value_, Iter_>::operator= (const CreateInsertIterator & other)
    {
        _iter = other._iter;
        return *this;
    }

    template <typename Value_, typename Iter_>
    template <typename T_>
    CreateInsertIterator<Value_, Iter_> &
    CreateInsertIterator<Value_, Iter_>::operator= (const T_ & value)
    {
        *_iter = Value_(value);
        return *this;
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_> &
    CreateInsertIterator<Value_, Iter_>::operator++ ()
    {
        ++_iter;
        return *this;
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>
    CreateInsertIterator<Value_, Iter_>::operator++ (int)
    {
        CreateInsertIterator result(*this);
        ++_iter;
        return result;
    }

    template <typename Value_, typename Iter_>
    typename CreateInsertIterator<Value_, Iter_>::pointer
    CreateInsertIterator<Value_, Iter_>::operator-> ()
    {
        return this;
    }

    template <typename Value_, typename Iter_>
    typename CreateInsertIterator<Value_, Iter_>::reference
    CreateInsertIterator<Value_, Iter_>::operator* ()
    {
        return *this;
    }

    template <typename Value_, typename Iter_>
    CreateInsertIterator<Value_, Iter_>
    create_inserter(const Iter_ & i)
    {
        return CreateInsertIterator<Value_, Iter_>(i);
    }
}

#endif
