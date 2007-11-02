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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_OUTPUT_ITERATOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_OUTPUT_ITERATOR_IMPL_HH 1

#include <paludis/util/wrapped_output_iterator.hh>

namespace paludis
{
    template <typename Tag_, typename Value_>
    struct WrappedOutputIterator<Tag_, Value_>::Base
    {
        virtual Base * clone() const = 0;
        virtual void increment() = 0;
        virtual void assign(const Value_ &) = 0;

        virtual ~Base()
        {
        }
    };

    template <typename Tag_, typename Value_>
    template <typename Iter_>
    struct WrappedOutputIterator<Tag_, Value_>::BaseImpl :
        WrappedOutputIterator<Tag_, Value_>::Base
    {
        Iter_ i;

        BaseImpl(const Iter_ & ii) :
            i(ii)
        {
        }

        Base * clone() const
        {
            return new BaseImpl(i);
        }

        void increment()
        {
            ++i;
        }

        void assign(const Value_ & v)
        {
            *i = v;
        }
    };

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>::WrappedOutputIterator() :
        _base(0)
    {
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>::~WrappedOutputIterator()
    {
        delete _base;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>::WrappedOutputIterator(const WrappedOutputIterator & other) :
        _base(other._base ? other._base->clone() : other._base)
    {
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    WrappedOutputIterator<Tag_, Value_>::WrappedOutputIterator(const T_ & base) :
        _base(new BaseImpl<T_>(base))
    {
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator= (const WrappedOutputIterator<Tag_, Value_> & other)
    {
        if (this != &other)
            _base = other._base ? other._base->clone() : other._base;
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator= (const Value_ & value)
    {
        _base->assign(value);
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator++ ()
    {
        _base->increment();
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>
    WrappedOutputIterator<Tag_, Value_>::operator++ (int)
    {
        WrappedOutputIterator result(*this);
        _base->increment();
        return result;
    }

    template <typename Tag_, typename Value_>
    typename WrappedOutputIterator<Tag_, Value_>::pointer
    WrappedOutputIterator<Tag_, Value_>::operator-> ()
    {
        return this;
    }

    template <typename Tag_, typename Value_>
    typename WrappedOutputIterator<Tag_, Value_>::reference
    WrappedOutputIterator<Tag_, Value_>::operator* ()
    {
        return *this;
    }
}

#endif
