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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_CREATE_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_CREATE_ITERATOR_HH 1

#include <iterator>
#include <paludis/util/create_iterator-fwd.hh>

namespace paludis
{
    /**
     * A CreateInsertIterator creates a value of type Value_ from the assigned value
     * and inserts it into Iter_.
     *
     * \ingroup g_iterator
     */
    template <typename Value_, typename Iter_>
    class CreateInsertIterator
    {
        private:
            Iter_ _iter;

        public:
            ///\name Basic operations
            ///\{

            CreateInsertIterator();
            ~CreateInsertIterator();
            CreateInsertIterator(const CreateInsertIterator &);
            CreateInsertIterator(const Iter_ &);

            CreateInsertIterator & operator= (const CreateInsertIterator &);

            template <typename T_>
            CreateInsertIterator & operator= (const T_ &);

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef CreateInsertIterator value_type;
            typedef CreateInsertIterator & reference;
            typedef CreateInsertIterator * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::output_iterator_tag iterator_category;

            ///\}

            ///\name Increment
            ///\{

            CreateInsertIterator & operator++ ();
            CreateInsertIterator operator++ (int);

            ///\}

            ///\name Dereference
            ///\{

            pointer operator-> ();
            reference operator* ();

            ///\}
    };
}

#endif
