/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_HH
#define PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/iterator.hh>

#include <algorithm>
#include <vector>

/** \file
 * Declarations for VirtualConstructor and related classes.
 *
 * \ingroup grpvc
 */

namespace paludis
{
    /**
     * Behaviour policy classes for what to do if an appropriate constructor
     * cannot be found for a VirtualConstructor::find_maker call.
     *
     * \ingroup grpvc
     */
    namespace virtual_constructor_not_found
    {
        /**
         * Throw an exception of type ExceptionType_, which should have a
         * constructor that takes a single parameter of KeyType_.
         *
         * \ingroup grpvc
         */
        template <typename ExceptionType_>
        struct ThrowException
        {
            /**
             * Internal use: provide handle_not_found.
             *
             * \ingroup grpvc
             */
            template <typename KeyType_, typename ValueType_>
            struct Parent
            {
                /**
                 * Internal use: called when we cannot find a key.
                 */
                ValueType_ handle_not_found(const KeyType_ & k) const PALUDIS_ATTRIBUTE((noreturn));
            };
        };

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
     * \ingroup grpvc
     */
    namespace virtual_constructor_internals
    {
        /**
         * Comparator class for our entries.
         *
         * \ingroup grpvc
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
     * A VirtualConstructor can be used where a mapping between the value of
     * some key type (often a string) to the construction of some kind of
     * class (possibly via a functor) is required.
     *
     * \ingroup grpvc
     */
    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    class PALUDIS_VISIBLE VirtualConstructor :
        public NotFoundBehaviour_::template Parent<KeyType_, ValueType_>,
        public InstantiationPolicy<VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>,
            instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<
            VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>,
            instantiation_method::SingletonAsNeededTag>;

        private:
            VirtualConstructor()
            {
            }

        protected:
            /**
             * Our entries, sorted.
             */
            std::vector<std::pair<KeyType_, ValueType_> > entries;

        public:
            /**
             * The type of our key.
             */
            typedef KeyType_ KeyType;

            /**
             * The type of our value.
             */
            typedef ValueType_ ValueType;

            /**
             * The behaviour policy for when a key is not found.
             */
            typedef NotFoundBehaviour_ NotFoundBehaviour;

            /**
             * Find a value for the specified key, or perform the appropriate
             * NotFoundBehaviour.
             */
            ValueType_ find_maker(const KeyType_ & k) const;

            /**
             * Convenience alias for find_maker.
             */
            ValueType_ operator[] (const KeyType_ & k) const
            {
                return find_maker(k);
            }

            /**
             * Register a new maker (should usually be called by the
             * RegisterMaker child class.
             */
            void register_maker(const KeyType_ & k, const ValueType_ & v);

            /**
             * Copy out our keys.
             */
            template <typename T_>
            void copy_keys(T_ out_iter) const;

            /**
             * An instance of this class registers a new maker with the
             * specified key.
             *
             * \ingroup grpvc
             */
            struct RegisterMaker
            {
                /**
                 * Constructor.
                 */
                RegisterMaker(const KeyType_ & k, const ValueType_ & v)
                {
                    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::get_instance()->
                        register_maker(k, v);
                }
            };
    };

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    ValueType_
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::find_maker(
            const KeyType_ & k) const
    {
        std::pair<
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator,
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator > m(
                    std::equal_range(entries.begin(), entries.end(), k,
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
                    std::equal_range(entries.begin(), entries.end(), k,
                        virtual_constructor_internals::ComparePairByFirst<KeyType_, ValueType_>()));
        if (m.first == m.second)
            entries.insert(m.first, std::make_pair(k, v));
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    template <typename T_>
    void
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::copy_keys(T_ out_iter) const
    {
        std::copy(entries.begin(), entries.end(), TransformInsertIterator<
                T_, SelectFirst<KeyType_, ValueType_> >(out_iter));
    }
}

#endif
