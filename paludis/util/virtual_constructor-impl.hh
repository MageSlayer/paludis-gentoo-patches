/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_IMPL_HH 1

#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/tr1_functional.hh>
#include <algorithm>
#include <vector>

/** \file
 * Implementation for paludis/util/virtual_constructor.hh .
 *
 * \ingroup g_oo
 */

namespace paludis
{
    namespace virtual_constructor_not_found
    {
        template <typename ExceptionType_>
        template <typename KeyType_, typename ValueType_>
        ValueType_
        ThrowException<ExceptionType_>::Parent<KeyType_, ValueType_>::handle_not_found(
                const KeyType_ & k) const
        {
            throw ExceptionType_(k);
        }
    }

    /**
     * For internal use by VirtualConstructor.
     *
     * \ingroup g_oo
     */
    namespace virtual_constructor_internals
    {
        /**
         * Comparator class for our entries.
         *
         * \ingroup g_oo
         */
        template <typename First_, typename Second_>
        struct ComparePairByFirst
        {
            /**
             * Compare, with the entry on the LHS.
             */
            bool operator() (const std::pair<First_, Second_> & a, const First_ & b) const
            {
                return a.first < b;
            }

            /**
             * Compare, with the entry on the RHS.
             */
            bool operator() (const First_ & a, const std::pair<First_, Second_> & b) const
            {
                return a < b.first;
            }
        };
    }

    /**
     * Holds the entries for a VirtualConstructor.
     *
     * \see VirtualConstructor
     * \ingroup g_oo
     * \nosubgrouping
     */
    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    struct VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::EntriesHolder
    {
        /// The entries.
        std::vector<std::pair<KeyType_, ValueType_> > entries;
    };

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::VirtualConstructor() :
        _entries_holder(new EntriesHolder)
    {
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::~VirtualConstructor()
    {
        delete _entries_holder;
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    ValueType_
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::operator[] (const KeyType_ & k) const
    {
        return find_maker(k);
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    ValueType_
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::find_maker(
            const KeyType_ & k) const
    {
        std::pair<
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator,
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator > m(
                    std::equal_range(_entries_holder->entries.begin(), _entries_holder->entries.end(), k,
                        virtual_constructor_internals::ComparePairByFirst<KeyType_, ValueType_>()));
        if (m.first == m.second)
            return this->handle_not_found(k);
        else
            return m.first->second;
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    void
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::register_maker(
            const KeyType_ & k, const ValueType_ & v)
    {
        std::pair<
            typename std::vector<std::pair<KeyType_, ValueType_> >::iterator,
            typename std::vector<std::pair<KeyType_, ValueType_> >::iterator > m(
                    std::equal_range(_entries_holder->entries.begin(), _entries_holder->entries.end(), k,
                        virtual_constructor_internals::ComparePairByFirst<KeyType_, ValueType_>()));
        if (m.first == m.second)
            _entries_holder->entries.insert(m.first, std::make_pair(k, v));
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    template <typename T_>
    void
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::copy_keys(T_ out_iter) const
    {
        std::copy(_entries_holder->entries.begin(), _entries_holder->entries.end(), transform_inserter(out_iter,
                    paludis::tr1::mem_fn(&std::pair<KeyType_, ValueType_>::first)));
    }
}

#endif
