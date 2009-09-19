/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/enum_iterator-fwd.hh>
#include <paludis/util/operators.hh>
#include <iterator>

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ENUM_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ENUM_ITERATOR_HH 1

namespace paludis
{
    /**
     * An iterator for iterating over enums.
     *
     * \since 0.41
     * \ingroup g_iterator
     */
    template <typename E_>
    class EnumIterator :
        public relational_operators::HasRelationalOperators
    {
        private:
            E_ _value;

        public:
            ///\name Basic operations
            ///\{

            EnumIterator() :
                _value(static_cast<E_>(0))
            {
            }

            EnumIterator(const EnumIterator & other) :
                _value(other._value)
            {
            }

            explicit EnumIterator(const E_ e) :
                _value(e)
            {
            }


            EnumIterator & operator= (const EnumIterator & other)
            {
                _value = other._value;
                return *this;
            }

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef E_ & value_type;

            typedef E_ & reference;
            typedef const E_ & const_reference;

            typedef E_ * pointer;
            typedef const E_ * const_pointer;

            typedef std::ptrdiff_t difference_type;
            typedef std::forward_iterator_tag iterator_category;

            ///\}

            ///\name Increment
            ///\{

            EnumIterator & operator++ ()
            {
                _value = static_cast<E_>(_value + 1);
                return *this;
            }

            EnumIterator operator++ (int)
            {
                EnumIterator result(*this);
                ++result;
                return result;
            }

            ///\}

            ///\name Dereference
            ///\{

            pointer operator-> ()
            {
                return &_value;
            }

            reference operator* ()
            {
                return _value;
            }

            const_pointer operator-> () const
            {
                return &_value;
            }

            const_reference operator* () const
            {
                return _value;
            }

            ///\}

            ///\name Comparisons

            bool operator== (const EnumIterator & other) const
            {
                return **this == *other;
            }

            bool operator< (const EnumIterator & other) const
            {
                return **this < *other;
            }

            ///\}
    };

    template <typename E_>
    EnumIterator<E_> enum_iterator(const E_ e)
    {
        return EnumIterator<E_>(e);
    }
}

#endif
