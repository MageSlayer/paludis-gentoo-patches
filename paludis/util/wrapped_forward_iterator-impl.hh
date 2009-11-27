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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_FORWARD_ITERATOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_FORWARD_ITERATOR_IMPL_HH 1

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/checked_delete.hh>

namespace paludis
{
    template <typename WrappedIter_>
    typename WrappedForwardIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator *
    wrapped_underlying_iterator_real_type(const WrappedIter_ &, void * i)
    {
        return reinterpret_cast<typename WrappedForwardIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator *>(i);
    }

    template <typename WrappedIter_>
    WrappedForwardIteratorUnderlyingIteratorHolder *
    wrapped_underlying_iterator_hide_real_type(const WrappedIter_ &,
            typename WrappedForwardIteratorTraits<typename WrappedIter_::Tag>::UnderlyingIterator * const i)
    {
        return reinterpret_cast<WrappedForwardIteratorUnderlyingIteratorHolder *>(i);
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_>::WrappedForwardIterator() :
        _iter(wrapped_underlying_iterator_hide_real_type(*this, new typename WrappedForwardIteratorTraits<Tag_>::UnderlyingIterator))
    {
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_>::~WrappedForwardIterator()
    {
        checked_delete(wrapped_underlying_iterator_real_type(*this, _iter));
    }

    template <typename Tag_, typename Value_, typename Iter_>
    struct WrappedForwardIteratorGetBase
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
                    i.template underlying_iterator<typename WrappedForwardIteratorTraits<
                    typename WrappedForwardIteratorTraits<Tag_>::EquivalentNonConstIterator::Tag
                    >::UnderlyingIterator>()
                    );
        }

        static typename WrappedForwardIteratorTraits<Tag_>::UnderlyingIterator get_iter(const Iter_ & i)
        {
            return real_get_iter<WrappedForwardIteratorTraits<Tag_> >(i);
        }
    };

    template <typename Tag_, typename Value_>
    struct WrappedForwardIteratorGetBase<Tag_, Value_, WrappedForwardIterator<Tag_, Value_> >
    {
        static WrappedForwardIterator<Tag_, Value_> get_iter(const WrappedForwardIterator<Tag_, Value_> & i)
        {
            return i.template underlying_iterator<typename WrappedForwardIteratorTraits<Tag_>::UnderlyingIterator>();
        }
    };

    template <typename LHS_, typename RHS_>
    void set_wrapped_forward_iterator_iterator(LHS_ & lhs, RHS_ rhs)
    {
        lhs = rhs;
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    WrappedForwardIterator<Tag_, Value_>::WrappedForwardIterator(const T_ & iter) :
        _iter(wrapped_underlying_iterator_hide_real_type(*this, new typename WrappedForwardIteratorTraits<Tag_>::UnderlyingIterator))
    {
        set_wrapped_forward_iterator_iterator(
                *wrapped_underlying_iterator_real_type(*this, _iter),
                WrappedForwardIteratorGetBase<Tag_, Value_, T_>::get_iter(iter));
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_>::WrappedForwardIterator(const WrappedForwardIterator & other) :
        _iter(wrapped_underlying_iterator_hide_real_type(*this, new typename WrappedForwardIteratorTraits<Tag_>::UnderlyingIterator(
                        *wrapped_underlying_iterator_real_type(other, other._iter))))
    {
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_> &
    WrappedForwardIterator<Tag_, Value_>::operator= (const WrappedForwardIterator & other)
    {
        *wrapped_underlying_iterator_real_type(*this, _iter) = *wrapped_underlying_iterator_real_type(other, other._iter);
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_> &
    WrappedForwardIterator<Tag_, Value_>::operator++ ()
    {
        ++*wrapped_underlying_iterator_real_type(*this, _iter);
        return *this;
    }

    template <typename Tag_, typename Value_>
    WrappedForwardIterator<Tag_, Value_>
    WrappedForwardIterator<Tag_, Value_>::operator++ (int)
    {
        WrappedForwardIterator result(*this);
        operator++ ();
        return result;
    }

    template <typename Tag_, typename Value_>
    typename WrappedForwardIterator<Tag_, Value_>::pointer
    WrappedForwardIterator<Tag_, Value_>::operator-> () const
    {
        return wrapped_underlying_iterator_real_type(*this, _iter)->operator-> ();
    }

    template <typename Tag_, typename Value_>
    typename WrappedForwardIterator<Tag_, Value_>::reference
    WrappedForwardIterator<Tag_, Value_>::operator* () const
    {
        return wrapped_underlying_iterator_real_type(*this, _iter)->operator* ();
    }

    template <typename Tag_, typename Value_>
    bool
    WrappedForwardIterator<Tag_, Value_>::operator== (const WrappedForwardIterator & other) const
    {
        return *wrapped_underlying_iterator_real_type(*this, _iter) == *wrapped_underlying_iterator_real_type(other, other._iter);
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    T_ &
    WrappedForwardIterator<Tag_, Value_>::underlying_iterator()
    {
        return *wrapped_underlying_iterator_real_type(*this, _iter);
    }

    template <typename Tag_, typename Value_>
    template <typename T_>
    const T_ &
    WrappedForwardIterator<Tag_, Value_>::underlying_iterator() const
    {
        return *wrapped_underlying_iterator_real_type(*this, _iter);
    }
}

#endif
