/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_FILTER_INSERT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_FILTER_INSERT_ITERATOR_HH 1

#include <functional>
#include <iterator>

namespace paludis
{
    /**
     * A FilterInsertIterator is an insert iterator that only performs an insert
     * if a particular predicate function returns true for the object to be
     * inserted.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Pred_>
    class FilterInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Pred_ _p;

        public:
            typedef typename Iter_::container_type container_type;

            /**
             * Constructor, from an iterator.
             */
            FilterInsertIterator(const Iter_ & i, const Pred_ & p) :
                _i(i),
                _p(p)
            {
            }

            /**
             * Copy constructor.
             */
            FilterInsertIterator(const FilterInsertIterator & other) :
                _i(other._i),
                _p(other._p)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const FilterInsertIterator & operator= (const T_ value)
            {
                if (_p(value))
                    *_i = value;
                return *this;
            }

            /**
             * Dereference.
             */
            FilterInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            FilterInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            FilterInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            FilterInsertIterator & operator++ (int)
            {
                return *this;
            }
    };

    /**
     * Convenience function: make a FilterInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Pred_>
    FilterInsertIterator<Iter_, Pred_> filter_inserter(
            const Iter_ & i, const Pred_ & p)
    {
        return FilterInsertIterator<Iter_, Pred_>(i, p);
    }
}

#endif
