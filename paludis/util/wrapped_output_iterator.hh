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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_OUTPUT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_OUTPUT_ITERATOR_HH 1

#include <paludis/util/wrapped_output_iterator-fwd.hh>
#include <paludis/util/attributes.hh>
#include <iterator>

namespace paludis
{
    /**
     * A WrappedOutputIterator is a generic wrapper around an output iterator,
     * hiding the underlying base iterator.
     *
     * \ingroup g_iterator
     * \since 0.26
     */
    template <typename Tag_, typename Value_>
    class PALUDIS_VISIBLE WrappedOutputIterator
    {
        private:
            WrappedOutputIteratorUnderlyingIteratorHolder * _iter;

        public:
            typedef Tag_ Tag;

            ///\name Basic operations
            ///\{

            ~WrappedOutputIterator();
            WrappedOutputIterator(const WrappedOutputIterator &);

            template <typename T_>
            WrappedOutputIterator(const T_ &);

            WrappedOutputIterator & operator= (const WrappedOutputIterator &);
            WrappedOutputIterator & operator= (const Value_ &);

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef WrappedOutputIterator value_type;
            typedef WrappedOutputIterator & reference;
            typedef WrappedOutputIterator * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::output_iterator_tag iterator_category;

            ///\}

            ///\name Increment
            ///\{

            WrappedOutputIterator & operator++ ();
            WrappedOutputIterator operator++ (int);

            ///\}

            ///\name Dereference
            ///\{

            reference operator* ();

            ///\}

            ///\name Underlying iterator
            ///\{

            template <typename T_> T_ & underlying_iterator();
            template <typename T_> const T_ & underlying_iterator() const;

            ///\}
    };
}

#endif
