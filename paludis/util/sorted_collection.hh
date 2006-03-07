/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_SORTED_COLLECTION_HH
#define PALUDIS_GUARD_PALUDIS_SORTED_COLLECTION_HH 1

#include <iterator>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <set>

namespace paludis
{
    /**
     * Wrapper around a std::set of a particular item. May be changed at some
     * point to support template find.
     */
    template <typename T_>
    class SortedCollection :
        private InstantiationPolicy<SortedCollection<T_>, instantiation_method::NonCopyableTag>,
        public InternalCounted<SortedCollection<T_> >
    {
        private:
            std::set<T_> _items;

        public:
            /**
             * Constructor.
             */
            SortedCollection()
            {
            }

            /**
             * Destructor.
             */
            virtual ~SortedCollection()
            {
            }

            /**
             * Our iterator type.
             */
            typedef typename std::set<T_>::const_iterator Iterator;

            /**
             * Iterator to the start of our items.
             */
            Iterator begin() const
            {
                return _items.begin();
            }

            /**
             * Iterator to one past our last item.
             */
            Iterator end() const
            {
                return _items.end();
            }

            /**
             * Our reverse iterator type.
             */
            typedef typename std::set<T_>::const_reverse_iterator ReverseIterator;

            /**
             * Reverse iterator to the start of our items.
             */
            ReverseIterator rbegin() const
            {
                return _items.rbegin();
            }

            /**
             * Reverse iterator to one past our last item.
             */
            ReverseIterator rend() const
            {
                return _items.rend();
            }

            /**
             * Iterator to our last item.
             */
            Iterator last() const
            {
                Iterator result(_items.end());
                if (result != _items.begin())
                    --result;
                return result;
            }

            /**
             * Return an Iterator to an item, or end() if there's no match.
             */
            Iterator find(const T_ & v) const
            {
                return _items.find(v);
            }

            /**
             * Insert an item, return whether we succeeded.
             */
            bool insert(T_ v)
            {
                return _items.insert(v).second;
            }

            /**
             * Insert all items from another container.
             */
            bool merge(typename SortedCollection<T_>::ConstPointer o)
            {
                bool result(true);
                Iterator o_begin(o->begin()), o_end(o->end());
                for ( ; o_begin != o_end ; ++o_begin)
                    result &= insert(*o_begin);
                return result;
            }

            /**
             * Are we empty?
             */
            bool empty() const
            {
                return _items.empty();
            }

            /**
             * How big are we?
             */
            unsigned size() const
            {
                return _items.size();
            }

            /**
             * Our insert iterator type.
             */
            typedef typename std::insert_iterator<std::set<T_> > Inserter;

            /**
             * Fetch an inserter.
             */
            Inserter inserter()
            {
                return std::inserter(_items, _items.begin());
            }
    };
}

#endif
