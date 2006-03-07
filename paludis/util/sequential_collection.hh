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

#ifndef PALUDIS_GUARD_PALUDIS_SEQUENTIAL_COLLECTION_HH
#define PALUDIS_GUARD_PALUDIS_SEQUENTIAL_COLLECTION_HH 1

#include <algorithm>
#include <list>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
    /**
     * Wrapper around a std::list of a particular item. Multiple items
     * with the same value are disallowed.
     */
    template <typename T_>
    class SequentialCollection :
        private InstantiationPolicy<SequentialCollection<T_>, instantiation_method::NonCopyableTag>,
        public InternalCounted<SequentialCollection<T_> >
    {
        private:
            std::list<T_> _items;

        public:
            /**
             * Constructor.
             */
            SequentialCollection()
            {
            }

            /**
             * Destructor.
             */
            virtual ~SequentialCollection()
            {
            }

            /**
             * Our iterator type.
             */
            typedef typename std::list<T_>::const_iterator Iterator;

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
             * Iterator to our last item.
             */
            Iterator last() const
            {
                return _items.begin() == _items.end() ? _items.end() : --(_items.end());
            }

            /**
             * Return an Iterator to an item, or end() if there's no match.
             */
            Iterator find(const T_ & v) const
            {
                return std::find(_items.begin(), _items.end(), v);
            }

            /**
             * Append an item, return whether we succeeded.
             */
            bool append(T_ v)
            {
                if (end() != find(v))
                    return false;

                _items.push_back(v);
                return true;
            }

            /**
             * Are we empty?
             */
            bool empty() const
            {
                return _items.empty();
            }
    };
}

#endif
