/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009 Ciaran McCreesh
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
#include <paludis/util/checked_delete.hh>

namespace paludis
{
    template <typename WrappedIter_>
    typename WrappedOutputIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator *
    wrapped_underlying_iterator_real_type(const WrappedIter_ &, void * i)
    {
        return reinterpret_cast<typename WrappedOutputIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator *>(i);
    }

    template <typename WrappedIter_>
    WrappedOutputIteratorUnderlyingIteratorHolder *
    wrapped_underlying_iterator_hide_real_type(const WrappedIter_ &,
            typename WrappedOutputIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator * const i)
    {
        return reinterpret_cast<WrappedOutputIteratorUnderlyingIteratorHolder *>(i);
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>::~WrappedOutputIterator()
    {
        checked_delete(wrapped_underlying_iterator_real_type(*this, _iter));
    }

    template <typename Tag_, typename Value_, typename Iter_>
    struct WrappedOutputIteratorGetBase
    {
        template <typename Traits_>
        static typename Traits_::UnderlyingIterator real_get_iter(const typename Traits_::UnderlyingIterator & i)
        {
            return i;
        }

        template <typename Traits_>
        static typename Traits_::UnderlyingIterator
        real_get_iter(const typename Traits_::EquivalentNonConstIterator & i)
        {
            return typename Traits_::UnderlyingIterator(
                    i.template underlying_iterator<typename WrappedOutputIteratorTraits<
                    typename WrappedOutputIteratorTraits<Tag_>::EquivalentNonConstIterator::Tag
                    >::UnderlyingIterator>()
                    );
        }

        static typename WrappedOutputIteratorTraits<Tag_>::UnderlyingIterator get_iter(const Iter_ & i)
        {
            return real_get_iter<WrappedOutputIteratorTraits<Tag_> >(i);
        }
    };

    template <typename Tag_, typename Value_>
    struct WrappedOutputIteratorGetBase<Tag_, Value_, WrappedOutputIterator<Tag_, Value_> >
    {
        static WrappedOutputIterator<Tag_, Value_> get_iter(const WrappedOutputIterator<Tag_, Value_> & i)
        {
            return i.template underlying_iterator<typename WrappedOutputIteratorTraits<Tag_>::UnderlyingIterator>();
        }
    };

    template <typename Tag_, typename Value_>
    template <typename T_>
    WrappedOutputIterator<Tag_, Value_>::WrappedOutputIterator(const T_ & iter) :
        _iter(wrapped_underlying_iterator_hide_real_type(*this, new typename WrappedOutputIteratorTraits<Tag_>::UnderlyingIterator(
                        WrappedOutputIteratorGetBase<Tag_, Value_, T_>::get_iter(iter))
                    ))

    {
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>::WrappedOutputIterator(const WrappedOutputIterator & other) :
        _iter(wrapped_underlying_iterator_hide_real_type(*this, new typename WrappedOutputIteratorTraits<Tag_>::UnderlyingIterator(
                        *wrapped_underlying_iterator_real_type(other, other._iter))))
    {
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator= (const WrappedOutputIterator & other)
    {
        *wrapped_underlying_iterator_real_type(*this, _iter) = *wrapped_underlying_iterator_real_type(other, other._iter);
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator= (const Value_ & value)
    {
        *wrapped_underlying_iterator_real_type(*this, _iter) = value;
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_> &
    WrappedOutputIterator<Tag_, Value_>::operator++ ()
    {
        ++*wrapped_underlying_iterator_real_type(*this, _iter);
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedOutputIterator<Tag_, Value_>
    WrappedOutputIterator<Tag_, Value_>::operator++ (int)
    {
        WrappedOutputIterator result(*this);
        operator++ ();
        return result;
    }

    template <typename Tag_, typename Value_>
    typename WrappedOutputIterator<Tag_, Value_>::reference
    WrappedOutputIterator<Tag_, Value_>::operator* ()
    {
        return *this;
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    T_ &
    WrappedOutputIterator<Tag_, Value_>::underlying_iterator()
    {
        return *wrapped_underlying_iterator_real_type(*this, _iter);
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    const T_ &
    WrappedOutputIterator<Tag_, Value_>::underlying_iterator() const
    {
        return *wrapped_underlying_iterator_real_type(*this, _iter);
    }
}

#endif
