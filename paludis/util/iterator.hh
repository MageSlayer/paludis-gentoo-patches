/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/comparison_policy.hh>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for various iterator helpers.
 *
 * \ingroup Iterator
 */

namespace paludis
{
    /**
     * Return a new iterator pointing to the item after i.
     *
     * \ingroup Iterator
     */
    template <typename T_>
    T_ next(const T_ & i)
    {
        T_ result(i);
        return ++result;
    }

    /**
     * Return a new iterator pointing to the item before i.
     *
     * \ingroup Iterator
     */
    template <typename T_>
    T_ previous(const T_ & i)
    {
        T_ result(i);
        return --result;
    }

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

    /**
     * Convenience constructor for an IndirectIterator.
     */
    template <typename Value_, typename Iter_>
    IndirectIterator<Iter_, Value_> indirect_iterator(const Iter_ & i)
    {
        return IndirectIterator<Iter_, Value_>(i);
    }

    /**
     * A FilterInsertIterator is an insert iterator that only performs an insert
     * if a particular predicate function returns true for the object to be
     * inserted.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Pred_>
    class FilterInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Pred_ _p;

        public:
            /**
             * Fake a container_type for use with other iterator adapters.
             */
            typedef typename Iter_::container_type container_type;

            /**
             * Constructor, from an iterator.
             */
            FilterInsertIterator(const Iter_ & i, const Pred_ & p) :
                _i(i),
                _p(p)
            {
            }

            /**
             * Copy constructor.
             */
            FilterInsertIterator(const FilterInsertIterator & other) :
                _i(other._i),
                _p(other._p)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const FilterInsertIterator & operator= (const T_ value)
            {
                if (_p(value))
                    *_i = value;
                return *this;
            }

            /**
             * Destructor.
             */
            ~FilterInsertIterator();

            /**
             * Dereference.
             */
            FilterInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            FilterInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            FilterInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            FilterInsertIterator & operator++ (int)
            {
                return *this;
            }
    };

    template <typename Iter_, typename Pred_>
    FilterInsertIterator<Iter_, Pred_>::~FilterInsertIterator()
    {
    }

    /**
     * Convenience function: make a FilterInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Pred_>
    FilterInsertIterator<Iter_, Pred_> filter_inserter(
            const Iter_ & i, const Pred_ & p)
    {
        return FilterInsertIterator<Iter_, Pred_>(i, p);
    }

    /**
     * A TransformInsertIterator is an insert iterator that calls some function
     * upon an item before inserting it.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    class TransformInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;
            Trans_ _t;

        public:
            /**
             * Fake a container_type entry to allow a TransformInsertIterator to
             * work with other iterator adapters.
             */
            struct container_type
            {
                /// Our value type.
                typedef typename Trans_::argument_type value_type;
            };

            /**
             * Constructor, from an iterator.
             */
            TransformInsertIterator(const Iter_ & i, const Trans_ & t = Trans_()) :
                _i(i),
                _t(t)
            {
            }

            /**
             * Copy constructor.
             */
            TransformInsertIterator(const TransformInsertIterator & other) :
                _i(other._i),
                _t(other._t)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const TransformInsertIterator & operator= (const T_ value)
            {
                *_i = _t(value);
                return *this;
            }

            /**
             * Dereference.
             */
            TransformInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            TransformInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            TransformInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            TransformInsertIterator & operator++ (int)
            {
                return *this;
            }

    };

    /**
     * Convenience function: make a TransformInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Trans_>
    TransformInsertIterator<Iter_, Trans_> transform_inserter(
            const Iter_ & i, const Trans_ & t)
    {
        return TransformInsertIterator<Iter_, Trans_>(i, t);
    }

    /**
     * Convenience class: select the first item of a pair.
     */
    template <typename A_, typename B_>
    struct SelectFirst :
        std::unary_function<A_, std::pair<A_, B_> >
    {
        /// Carry out the selection.
        A_ operator() (const std::pair<A_, B_> & p) const
        {
            return p.first;
        }
    };

    /**
     * Convenience class: select the second item of a pair.
     */
    template <typename A_, typename B_>
    struct SelectSecond :
        std::unary_function<A_, std::pair<A_, B_> >
    {
        /// Carry out the selection.
        A_ operator() (const std::pair<A_, B_> & p) const
        {
            return p.second;
        }
    };

    /**
     * A CreateInsertIterator is an insert iterator that creates an object of
     * the specified type using the provided value.
     *
     * \ingroup Iterator
     */
    template <typename Iter_, typename Type_>
    class CreateInsertIterator :
        public std::iterator<typename std::iterator_traits<Iter_>::iterator_category, void, void, void, void>
    {
        private:
            Iter_ _i;

        public:
            /**
             * Fake a container_type to allow us to work with other iterator
             * adapters.
             */
            struct container_type
            {
                /// Our faked item type.
                typedef Type_ value_type;
            };

            /**
             * Constructor, from an iterator.
             */
            CreateInsertIterator(const Iter_ & i) :
                _i(i)
            {
            }

            /**
             * Copy constructor.
             */
            CreateInsertIterator(const CreateInsertIterator & other) :
                _i(other._i)
            {
            }

            /**
             * Assignment.
             */
            template <typename T_>
            const CreateInsertIterator & operator= (const T_ value)
            {
                *_i = Type_(value);
                return *this;
            }

            /**
             * Dereference.
             */
            CreateInsertIterator & operator* ()
            {
                return *this;
            }

            /**
             * Dereference arrow.
             */
            CreateInsertIterator * operator-> ()
            {
                return this;
            }

            /**
             * Increment.
             */
            CreateInsertIterator & operator++ ()
            {
                return *this;
            }

            /**
             * Increment.
             */
            CreateInsertIterator & operator++ (int)
            {
                return *this;
            }
    };

    /**
     * Convenience function: make a CreateInsertIterator.
     *
     * \ingroup Iterator
     */
    template <typename Type_, typename Iter_>
    CreateInsertIterator<Iter_, Type_> create_inserter(const Iter_ & i)
    {
        return CreateInsertIterator<Iter_, Type_>(i);
    }
}

#endif
