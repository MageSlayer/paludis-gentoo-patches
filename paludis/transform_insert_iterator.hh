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

#ifndef PALUDIS_GUARD_PALUDIS_TRANSFORM_INSERT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_TRANSFORM_INSERT_ITERATOR_HH 1

#include <functional>
#include <iterator>

namespace paludis
{
    /**
     * A TransformInsertIterator is an insert iterator that calls some function
     * upon an item before inserting it.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    class TransformInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Trans_ _t;

        public:
            /**
             * Fake a container_type entry to allow a TransformInsertIterator to
             * work with other iterator adapters.
             */
            struct container_type
            {
                typedef typename Trans_::argument_type value_type;
            };

            /**
             * Constructor, from an iterator.
             */
            TransformInsertIterator(const Iter_ & i, const Trans_ & t) :
                _i(i),
                _t(t)
            {
            }

            /**
             * Copy constructor.
             */
            TransformInsertIterator(const TransformInsertIterator & other) :
                _i(other._i),
                _t(other._t)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const TransformInsertIterator & operator= (const T_ value)
            {
                *_i = _t(value);
                return *this;
            }

            /**
             * Dereference.
             */
            TransformInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            TransformInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            TransformInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            TransformInsertIterator & operator++ (int)
            {
                return *this;
            }

    };

    /**
     * Convenience function: make a TransformInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    TransformInsertIterator<Iter_, Trans_> transform_inserter(
            const Iter_ & i, const Trans_ & t)
    {
        return TransformInsertIterator<Iter_, Trans_>(i, t);
    }
}

#endif
