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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SEQUENCE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SEQUENCE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/wrapped_output_iterator-fwd.hh>

/** \file
 * Declarations for the Sequence<> class.
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
     * Wrapper around a list, avoiding the need to include standard library
     * headers everywhere.
     *
     * \ingroup g_data_structures
     * \since 0.26
     * \nosubgrouping
     */
    template <typename T_>
    class PALUDIS_VISIBLE Sequence :
        private Pimp<Sequence<T_> >
    {
        private:
            using Pimp<Sequence<T_> >::_imp;

        public:
            ///\name Standard library typedefs
            ///\{

            typedef T_ value_type;
            typedef T_ & reference;
            typedef const T_ & const_reference;

            ///\}

            ///\name Basic operations
            ///\{

            Sequence();
            ~Sequence();

            Sequence(const Sequence &) = delete;
            Sequence & operator= (const Sequence &) = delete;

            ///\}

            ///\name Iteration
            ///\{

            typedef SequenceConstIteratorTag<T_> ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const T_> ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator last() const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef SequenceReverseConstIteratorTag<T_> ReverseConstIteratorTag;
            typedef WrappedForwardIterator<ReverseConstIteratorTag, const T_> ReverseConstIterator;
            ReverseConstIterator rbegin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ReverseConstIterator rend() const PALUDIS_ATTRIBUTE((warn_unused_result));

            typedef SequenceInserterTag<T_> InserterTag;
            typedef WrappedOutputIterator<InserterTag, T_> Inserter;
            Inserter back_inserter();

            ///\}

            ///\name Content information
            ///\{

            bool empty() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Content modification
            ///\{

            void push_back(const T_ &);
            void push_front(const T_ &);

            void pop_front();
            void pop_back();

            template <typename C_>
            void sort(const C_ &);

            ///\}

    };
}

#endif
