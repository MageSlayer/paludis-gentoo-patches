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

#ifndef PALUDIS_GUARD_PALUDIS_CREATE_INSERT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_CREATE_INSERT_ITERATOR_HH 1

#include <functional>
#include <iterator>

namespace paludis
{
    /**
     * A CreateInsertIterator is an insert iterator that creates an object of
     * the specified type using the provided value.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Type_>
    class CreateInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;

        public:
            /**
             * Fake a container_type to allow us to work with other iterator
             * adapters.
             */
            struct container_type
            {
                /// Our faked item type.
                typedef Type_ value_type;
            };

            /**
             * Constructor, from an iterator.
             */
            CreateInsertIterator(const Iter_ & i) :
                _i(i)
            {
            }

            /**
             * Copy constructor.
             */
            CreateInsertIterator(const CreateInsertIterator & other) :
                _i(other._i)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const CreateInsertIterator & operator= (const T_ value)
            {
                *_i = Type_(value);
                return *this;
            }

            /**
             * Dereference.
             */
            CreateInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            CreateInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            CreateInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            CreateInsertIterator & operator++ (int)
            {
                return *this;
            }
    };

    /**
     * Convenience function: make a CreateInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Type_, typename Iter_>
    CreateInsertIterator<Iter_, Type_> create_inserter(const Iter_ & i)
    {
        return CreateInsertIterator<Iter_, Type_>(i);
    }
}

#endif
