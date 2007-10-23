/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for VirtualConstructor and related classes.
 *
 * \ingroup g_oo
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Behaviour policy classes for what to do if an appropriate constructor
     * cannot be found for a VirtualConstructor::find_maker call.
     *
     * \ingroup g_oo
     */
    namespace virtual_constructor_not_found
    {
        /**
         * Throw an exception of type ExceptionType_, which should have a
         * constructor that takes a single parameter of KeyType_.
         *
         * \ingroup g_oo
         */
        template <typename ExceptionType_>
        struct ThrowException
        {
            /**
             * Internal use: provide handle_not_found.
             *
             * \ingroup g_oo
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
    }

    /**
     * A VirtualConstructor can be used where a mapping between the value of
     * some key type (often a string) to the construction of some kind of
     * class (possibly via a functor) is required.
     *
     * \ingroup g_oo
     */
    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    class PALUDIS_VISIBLE VirtualConstructor :
        public NotFoundBehaviour_::template Parent<KeyType_, ValueType_>,
        private InstantiationPolicy<VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>, instantiation_method::NonCopyableTag>
    {
        private:
            struct EntriesHolder;
            EntriesHolder * _entries_holder;

        protected:
            /**
             * Constructor.
             */
            VirtualConstructor();

        public:
            ~VirtualConstructor();

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
            ValueType_ operator[] (const KeyType_ & k) const;

            /**
             * Register a new maker.
             */
            void register_maker(const KeyType_ & k, const ValueType_ & v);

            /**
             * Copy out our keys.
             */
            template <typename T_>
            void copy_keys(T_ out_iter) const;
    };
}

#endif
