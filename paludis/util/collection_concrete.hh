/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_COLLECTION_CONCRETE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_COLLECTION_CONCRETE_HH 1

#include <paludis/util/collection.hh>
#include <list>
#include <set>
#include <map>
#include <algorithm>

/** \file
 * Concrete implementations for the collection classes.
 *
 * Do not include this file in other headers if possible. It pulls in big
 * STL things, slowing down compilation.
 *
 * \ingroup grpcollections
 */

namespace paludis
{
    /**
     * Concrete implementation for a SequentialCollection.
     *
     * \see SequentialCollection
     * \ingroup grpcollections
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE SequentialCollection<T_>::Concrete :
        public SequentialCollection<T_>
    {
        private:
            std::list<T_> _items;

        public:
            /// Access to the underlying data structure
            std::list<T_> & list;

            ///\name Basic operations
            ///\{

            Concrete() :
                SequentialCollection(),
                list(_items)
            {
            }

            virtual ~Concrete()
            {
            }

            ///\}

            virtual Iterator begin() const
            {
                return Iterator(_items.begin());
            }

            virtual Iterator end() const
            {
                return Iterator(_items.end());
            }

            virtual Iterator last() const
            {
                return Iterator(_items.begin() == _items.end() ? _items.end() : --(_items.end()));
            }

            virtual Iterator find(const T_ & v) const
            {
                return Iterator(std::find(_items.begin(), _items.end(), v));
            }

            virtual ReverseIterator rbegin() const
            {
                return ReverseIterator(_items.rbegin());
            }

            virtual ReverseIterator rend() const
            {
                return ReverseIterator(_items.rend());
            }

            virtual bool append(T_ v)
            {
                if (end() != find(v))
                    return false;

                _items.push_back(v);
                return true;
            }

            void push_back(const T_ & v)
            {
                if (end() == find(v))
                    _items.push_back(v);
            }

            virtual bool empty() const
            {
                return _items.empty();
            }

            virtual Inserter inserter()
            {
                return Inserter(std::back_inserter(_items));
            }

            ///\name Extra functions
            ///\{

            template <typename C_>
            void sort(const C_ & t)
            {
                _items.sort(t);
            }

            //\}

            ///\name Pointer types
            ///\{

            typedef CountedPtr<Concrete, count_policy::InternalCountTag> ConstPointer;
            typedef CountedPtr<Concrete, count_policy::InternalCountTag> Pointer;

            ///\}
    };

    /**
     * Concrete implementation for a SortedCollection.
     *
     * \see SortedCollection
     * \ingroup grpcollections
     * \nosubgrouping
     */
    template <typename T_, typename C_>
    class PALUDIS_VISIBLE SortedCollection<T_, C_>::Concrete :
        public SortedCollection<T_, C_>
    {
        private:
            std::set<T_, C_> _items;

        public:
            ///\name Basic operations
            ///\{

            Concrete()
            {
            }

            virtual ~Concrete()
            {
            }

            ///\}

            virtual Iterator begin() const
            {
                return Iterator(_items.begin());
            }

            virtual Iterator end() const
            {
                return Iterator(_items.end());
            }

            virtual ReverseIterator rbegin() const
            {
                return ReverseIterator(_items.rbegin());
            }

            virtual ReverseIterator rend() const
            {
                return ReverseIterator(_items.rend());
            }

            virtual Iterator last() const
            {
                typename std::set<T_, C_>::const_iterator result(_items.end());
                if (result != _items.begin())
                    --result;
                return Iterator(result);
            }

            virtual Iterator find(const T_ & v) const
            {
                return Iterator(_items.find(v));
            }

            virtual int count(const T_ & v) const
            {
                return _items.count(v);
            }

            virtual bool insert(const T_ & v)
            {
                return _items.insert(v).second;
            }

            virtual bool erase(const T_ & v)
            {
                return 0 != _items.erase(v);
            }

            virtual bool merge(typename SortedCollection<T_, C_>::ConstPointer o)
            {
                bool result(true);
                Iterator o_begin(o->begin()), o_end(o->end());
                for ( ; o_begin != o_end ; ++o_begin)
                    result &= insert(*o_begin);
                return result;
            }

            virtual Inserter inserter()
            {
                return Inserter(std::inserter(_items, _items.begin()));
            }

            virtual bool empty() const
            {
                return _items.empty();
            }

            virtual unsigned size() const
            {
                return _items.size();
            }
    };

    /**
     * Concrete implementation for an AssociativeCollection.
     *
     * \see AssociativeCollection
     * \ingroup grpcollections
     * \nosubgrouping
     */
    template <typename K_, typename V_>
    class PALUDIS_VISIBLE AssociativeCollection<K_, V_>::Concrete :
        public AssociativeCollection<K_, V_>
    {
        private:
            std::map<K_, V_> _items;

        public:
            ///\name Basic operations
            ///\{

            Concrete()
            {
            }

            template <typename I_>
            Concrete(const I_ & b, const I_ & b_end) :
                _items(b, b_end)
            {
            }

            virtual ~Concrete()
            {
            }

            ///\}

            virtual Iterator begin() const
            {
                return Iterator(_items.begin());
            }

            virtual Iterator end() const
            {
                return Iterator(_items.end());
            }

            virtual Iterator find(const K_ & v) const
            {
                return Iterator(_items.find(v));
            }

            virtual bool insert(const K_ & k, const V_ & v)
            {
                return _items.insert(std::make_pair(k, v)).second;
            }

            virtual bool erase(const K_ & k)
            {
                return 0 != _items.erase(k);
            }

            virtual bool erase(const Iterator & i)
            {
                _items.erase(i->first);
                return true;
            }

            virtual bool empty() const
            {
                return _items.empty();
            }

            virtual Inserter inserter()
            {
                return Inserter(std::inserter(_items, _items.begin()));
            }
    };
}

#endif
