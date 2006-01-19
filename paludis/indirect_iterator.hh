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

#ifndef PALUDIS_GUARD_PALUDIS_INDIRECT_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_INDIRECT_ITERATOR_HH 1

#include <iterator>
#include <paludis/instantiation_policy.hh>
#include <paludis/comparison_policy.hh>

namespace paludis
{
    template <typename Iter_, typename Value_>
    class IndirectIterator;

    namespace
    {
        /**
         * Determine the comparison class to use for IndirectIterator.
         */
        template <typename IterCategory_, typename Iter_, typename Value_>
        struct Comparisons
        {
            /**
             * Default to providing == and !=.
             */
            typedef ComparisonPolicy<IndirectIterator<Iter_, Value_>,
                    comparison_mode::EqualityComparisonTag,
                    comparison_method::CompareByMemberTag<Iter_> > Type;
        };

        /**
         * Determine the comparison class to use for IndirectIterator
         * (specialisation for random access iterators).
         */
        template <typename Iter_, typename Value_>
        struct Comparisons<std::random_access_iterator_tag, Iter_, Value_>
        {
            /**
             * Provide the full range of comparison operators.
             */
            typedef ComparisonPolicy<IndirectIterator<Iter_, Value_>,
                    comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberTag<Iter_> > Type;
        };
    }

    /**
     * An IndirectIterator is an iterator adapter that does one additional level
     * of dereferencing.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Value_>
    class IndirectIterator : public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, Value_>,
                             public Comparisons<typename std::iterator_traits<Iter_>::iterator_category,
                                        Iter_, Value_>::Type
    {
        private:
            Iter_ _i;

        public:
            /**
             * Constructor, from a base iterator.
             */
            IndirectIterator(const Iter_ & i) :
                Comparisons<typename std::iterator_traits<Iter_>::iterator_category, Iter_, Value_>::Type(
                        &IndirectIterator<Iter_, Value_>::_i),
                _i(i)
            {
            }

            /**
             * Copy constructor.
             */
            IndirectIterator(const IndirectIterator & other) :
                Comparisons<typename std::iterator_traits<Iter_>::iterator_category, Iter_, Value_>::Type(
                        &IndirectIterator<Iter_, Value_>::_i),
                _i(other._i)
            {
            }

            /**
             * Assignment.
             */
            const IndirectIterator & operator= (const IndirectIterator & other)
            {
                _i = other._i;
                return *this;
            }

            /**
             * Dereference.
             */
            Value_ & operator*()
            {
                return **_i;
            }

            /**
             * Dereference arrow.
             */
            Value_ * operator->()
            {
                return &**_i;
            }

            /**
             * Dereference, const.
             */
            const Value_ & operator*() const
            {
                return **_i;
            }

            /**
             * Dereference arrow, const.
             */
            const Value_ * operator->() const
            {
                return &**_i;
            }

            /**
             * Increment.
             */
            IndirectIterator & operator++ ()
            {
                ++_i;
                return *this;
            }

            /**
             * Increment.
             */
            IndirectIterator operator++ (int)
            {
                IndirectIterator tmp(*this);
                ++_i;
                return tmp;
            }
    };
}

#endif
