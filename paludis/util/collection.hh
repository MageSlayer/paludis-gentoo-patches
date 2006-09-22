/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_SEQUENTIAL_COLLECTION_HH
#define PALUDIS_GUARD_PALUDIS_SEQUENTIAL_COLLECTION_HH 1

#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <libwrapiter/libwrapiter.hh>
#include <iterator>

/** \file
 * Various wrappers around collections of items, for convenience and
 * avoiding passing around huge containers.
 *
 * \ingroup grpcollections
 */

namespace paludis
{
    /**
     * Wrapper around a std::list of a particular item. Multiple items
     * with the same value are disallowed.
     *
     * This item cannot be constructed. Use SequentialCollection::Concrete,
     * which requires including paludis/util/collection_concrete.hh .
     *
     * \ingroup grpcollections
     */
    template <typename T_>
    class SequentialCollection :
        private InstantiationPolicy<SequentialCollection<T_>, instantiation_method::NonCopyableTag>,
        public InternalCounted<SequentialCollection<T_> >,
        public std::iterator<typename std::iterator_traits<
            typename libwrapiter::ForwardIterator<SequentialCollection<T_>, const T_> >::iterator_category, T_>
    {
        protected:
            ///\name Basic operations
            ///\{

            SequentialCollection()
            {
            }

            ///\}

        public:
            class Concrete;

            /**
             * Issue with g++ 3.4.6: const_reference isn't defined via our std::iterator
             * inherit, but it is needed by many standard algorithms.
             */
            typedef const T_ & const_reference;

            ///\name Basic operations
            ///\{

            virtual ~SequentialCollection()
            {
            }

            ///\}

            ///\name Iterate over our items
            ///\{

            typedef libwrapiter::ForwardIterator<SequentialCollection<T_>, const T_> Iterator;

            virtual Iterator begin() const = 0;

            virtual Iterator end() const = 0;

            virtual Iterator last() const = 0;

            ///\}

            ///\name Finding items
            ///\{

            /**
             * Return an Iterator to an item, or end() if there's no match.
             */
            virtual Iterator find(const T_ & v) const = 0;

            ///\}

            ///\name Adding and modifying items
            ///\{

            /**
             * Append an item, return whether we succeeded.
             */
            virtual bool append(T_ v) = 0;

            /**
             * Append an item.
             */
            virtual void push_back(const T_ & v) = 0;

            ///\}

            ///\name Queries
            ///\{

            /**
             * Are we empty?
             */
            virtual bool empty() const = 0;

            ///\}
    };

    /**
     * Wrapper around a std::set of a particular item.
     *
     * This item cannot be constructed. Use SortedCollection::Concrete,
     * which requires including paludis/util/collection_concrete.hh .
     *
     * \ingroup grpcollections
     */
    template <typename T_, typename C_ = std::less<T_> >
    class SortedCollection :
        private InstantiationPolicy<SortedCollection<T_, C_>, instantiation_method::NonCopyableTag>,
        public InternalCounted<SortedCollection<T_, C_> >,
        public std::iterator<typename std::iterator_traits<
            typename libwrapiter::ForwardIterator<SortedCollection<T_, C_>, const T_> >::iterator_category, T_>
    {
        protected:
            ///\name Basic operations
            ///\{

            SortedCollection()
            {
            }

            ///\}

        public:
            class Concrete;

            ///\name Basic operations
            ///\{

            virtual ~SortedCollection()
            {
            }

            ///\}

            ///\name Iterate over our items
            ///\{

            typedef libwrapiter::ForwardIterator<SortedCollection<T_, C_>, const T_> Iterator;

            virtual Iterator begin() const = 0;

            virtual Iterator end() const = 0;

            struct ReverseTag;
            typedef libwrapiter::ForwardIterator<ReverseTag, const T_> ReverseIterator;

            virtual ReverseIterator rbegin() const = 0;

            virtual ReverseIterator rend() const = 0;

            virtual Iterator last() const = 0;

            ///\}

            ///\name Finding items
            ///\{

            virtual Iterator find(const T_ & v) const = 0;

            virtual int count(const T_ & v) const = 0;

            ///\}

            ///\name Adding, removing and modifying items
            ///\{

            /**
             * Insert an item, return whether we succeeded.
             */
            virtual bool insert(const T_ & v) = 0;

            /**
             * Erase an item, return whether we succeeded.
             */
            virtual bool erase(const T_ & v) = 0;

            /**
             * Insert all items from another container.
             */
            virtual bool merge(typename SortedCollection<T_, C_>::ConstPointer o) = 0;

            /**
             * Our insert iterator type.
             */
            typedef libwrapiter::OutputIterator<SortedCollection<T_, C_>, T_> Inserter;

            /**
             * Fetch an inserter.
             */
            virtual Inserter inserter() = 0;

            ///\}

            ///\name Queries
            ///\{

            /**
             * Are we empty?
             */
            virtual bool empty() const = 0;

            /**
             * How big are we?
             */
            virtual unsigned size() const = 0;

            ///\}
    };

    /**
     * Wrapper around a std::map of a particular item.
     *
     * This item cannot be constructed. Use AssociativeCollection::Concrete,
     * which requires including paludis/util/collection_concrete.hh .
     *
     * \ingroup grpcollections
     */
    template <typename K_, typename V_>
    class AssociativeCollection :
        private InstantiationPolicy<AssociativeCollection<K_, V_>, instantiation_method::NonCopyableTag>,
        public InternalCounted<AssociativeCollection<K_, V_> >,
        public std::iterator<typename std::iterator_traits<
            typename libwrapiter::ForwardIterator<AssociativeCollection<K_, V_>,
            const std::pair<const K_, V_> > >::iterator_category, const std::pair<const K_, V_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            AssociativeCollection()
            {
            }

            ///\}

        public:
            class Concrete;

            ///\name Basic operations
            ///\{

            virtual ~AssociativeCollection()
            {
            }

            ///\}

            ///\name Iterate over our items
            ///\{

            typedef libwrapiter::ForwardIterator<AssociativeCollection<K_, V_>,
                    const std::pair<const K_, V_> > Iterator;

            virtual Iterator begin() const = 0;

            virtual Iterator end() const = 0;

            ///\}

            ///\name Finding items
            ///\{

            virtual Iterator find(const K_ & v) const = 0;

            ///\}

            ///\name Adding, removing and modifying items
            ///\{

            /**
             * Insert an item, return whether we succeeded.
             */
            virtual bool insert(const K_ & k, const V_ & v) = 0;

            /**
             * Erase an item, return whether we succeeded.
             */
            virtual bool erase(const K_ & k) = 0;

            /**
             * Erase an item, return whether we succeeded.
             */
            virtual bool erase(const Iterator & i) = 0;

            ///\}

            ///\name Queries
            ///\{

            /**
             * Are we empty?
             */
            virtual bool empty() const = 0;

            ///\}
    };
}

#endif
