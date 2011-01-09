/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAP_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAP_HH 1

#include <paludis/util/map-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_output_iterator-fwd.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <string>
#include <utility>

/** \file
 * Declarations for the Map<> class.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * A wrapper around a map, avoiding the need to include lots of STL bloat
     * everywhere.
     *
     * \ingroup g_data_structures
     * \since 0.26
     * \nosubgrouping
     */
    template <typename K_, typename V_, typename C_>
    class PALUDIS_VISIBLE Map
    {
        private:
            Pimp<Map<K_, V_, C_> > _imp;

        public:
            ///\name Basic operations
            ///\{

            Map();
            ~Map();

            Map(const Map &) = delete;
            Map & operator= (const Map &) = delete;

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef std::pair<const K_, V_> value_type;
            typedef std::pair<const K_, V_> & reference;
            typedef const reference & const_reference;

            ///\}

            ///\name Iteration
            ///\{

            typedef MapConstIteratorTag<K_, V_, C_> ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::pair<const K_, V_> > ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator find(const K_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef MapInserterTag<K_, V_, C_> InserterTag;
            typedef WrappedOutputIterator<InserterTag, std::pair<const K_, V_> > Inserter;
            Inserter inserter() PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Content information
            ///\{

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned size() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Content modification
            ///\{

            void insert(const K_ &, const V_ &);
            void erase(const ConstIterator &);
            void erase(const K_ &);

            ///\}
    };

    extern template class Map<std::string, std::string>;
    extern template class WrappedForwardIterator<Map<std::string, std::string>::ConstIteratorTag, const std::pair<const std::string, std::string> >;
}

#endif
