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

#ifndef PALUDIS_GUARD_PALUDIS_TRANSLATE_INSERT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_TRANSLATE_INSERT_ITERATOR_HH 1

#include <functional>
#include <iterator>

namespace paludis
{
    /**
     * A TranslateInsertIterator is an insert iterator that calls some function
     * upon an item before inserting it.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    class TranslateInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Trans_ _t;

        public:
            /**
             * Fake a container_type entry to allow a TranslateInsertIterator to
             * work with other iterator adapters.
             */
            struct container_type
            {
                typedef typename Trans_::argument_type value_type;
            };

            /**
             * Constructor, from an iterator.
             */
            TranslateInsertIterator(const Iter_ & i, const Trans_ & t) :
                _i(i),
                _t(t)
            {
            }

            /**
             * Copy constructor.
             */
            TranslateInsertIterator(const TranslateInsertIterator & other) :
                _i(other._i),
                _t(other._t)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const TranslateInsertIterator & operator= (const T_ value)
            {
                *_i = _t(value);
                return *this;
            }

            /**
             * Dereference.
             */
            TranslateInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            TranslateInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            TranslateInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            TranslateInsertIterator & operator++ (int)
            {
                return *this;
            }

    };

    /**
     * Convenience function: make a TranslateInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    TranslateInsertIterator<Iter_, Trans_> translate_inserter(
            const Iter_ & i, const Trans_ & t)
    {
        return TranslateInsertIterator<Iter_, Trans_>(i, t);
    }
}

#endif
