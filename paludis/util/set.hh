/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SET_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SET_HH 1

#include <paludis/util/set-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/wrapped_output_iterator-fwd.hh>
#include <string>

/** \file
 * Declarations for the Set<> class.
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
     * A wrapper around a set, avoiding the need to include lots of STL bloat
     * all over the place.
     *
     * \ingroup g_data_structures
     * \since 0.26
     */
    template <typename T_, typename C_>
    class PALUDIS_VISIBLE Set :
        private PrivateImplementationPattern<Set<T_, C_> >
    {
        private:
            using PrivateImplementationPattern<Set<T_, C_> >::_imp;

        public:
            typedef T_ Tag;

            ///\name Standard library typedefs
            ///\{

            typedef T_ value_type;
            typedef T_ & reference;
            typedef const T_ & const_reference;

            ///\}

            ///\name Basic operations
            ///\{

            Set();
            ~Set();

            Set(const Set &) = delete;
            Set & operator= (const Set &) = delete;

            ///\}

            ///\name Iteration
            ///\{

            typedef SetConstIteratorTag<T_, C_> ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const T_> ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator find(const T_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef SetInsertIteratorTag<T_, C_> InserterTag;
            typedef WrappedOutputIterator<InserterTag, T_> Inserter;
            Inserter inserter();

            ///\}

            ///\name Content information
            ///\{

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned size() const PALUDIS_ATTRIBUTE((warn_unused_result));
            unsigned count(const T_ &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Content modification
            ///\{

            void insert(const T_ &);
            void erase(const T_ &);
            void clear();

            ///\}
    };

    extern template class Set<std::string>;
}

#endif
