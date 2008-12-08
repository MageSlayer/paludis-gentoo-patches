/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_VALIDATED_HH
#define PALUDIS_GUARD_PALUDIS_VALIDATED_HH 1

#include <iosfwd>
#include <functional>
#include <paludis/util/validated-fwd.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/select.hh>

/** \file
 * Validated declarations.
 *
 * \ingroup g_data_structures
 *
 * \section Examples
 *
 * - \ref example_name.cc "example_name.cc" shows basic usage of various defined
 *   Validated classes.
 */

namespace paludis
{
    /**
     * Default comparator for Validated, used to avoid having to include
     * huge standard library headers in a -fwd.
     *
     * \ingroup g_data_structures
     */
    template <typename T_>
    struct PALUDIS_VISIBLE DefaultValidatedComparator :
        std::less<T_>
    {
    };

    /**
     * A Validated wraps a particular class instance, ensuring that it always
     * meets certain validation criteria.
     *
     * \ingroup g_data_structures
     */
    template <typename ValidatedDataType_, typename Validator_, bool full_comparison_, typename Comparator_>
    class Validated :
        public Select<full_comparison_,
            relational_operators::HasRelationalOperators,
            equality_operators::HasEqualityOperators>::Type
    {
        private:
            ValidatedDataType_ _value;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Copy constructor (no validation needed).
             */
            Validated(const Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_> & other);

            /**
             * Constructor (validation needed).
             */
            explicit Validated(const ValidatedDataType_ & value);

            /**
             * Assignment (no validation needed).
             */
            const Validated<ValidatedDataType_, Validator_, full_comparison_> & operator=
                (const Validated<ValidatedDataType_, Validator_, full_comparison_> & other)
            {
                _value = other._value;
                return *this;
            }

            ///\}

            /**
             * Fetch to our ValidatedDataType_. This should not be a cast
             * operator to avoid problems with ambiguous comparison operators.
             */
            const ValidatedDataType_ & data() const
            {
                return _value;
            }
    };

    template <typename ValidatedDataType_, typename Validator_, bool full_comparison_, typename Comparator_>
    Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_>::Validated(
            const Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_> & other) :
        _value(other._value)
    {
    }

    template <typename ValidatedDataType_, typename Validator_, bool full_comparison_, typename Comparator_>
    Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_>::Validated(
            const ValidatedDataType_ & value) :
        _value(value)
    {
        Validator_::validate(_value);
    }

    template <typename ValidatedDataType_, typename Validator_, bool full_comparison_, typename Comparator_>
    bool operator== (
            const Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_> & a,
            const Validated<ValidatedDataType_, Validator_, full_comparison_, Comparator_> & b)
    {
        return a.data() == b.data();
    }

    template <typename ValidatedDataType_, typename Validator_, typename Comparator_>
    bool operator< (
            const Validated<ValidatedDataType_, Validator_, true, Comparator_> & a,
            const Validated<ValidatedDataType_, Validator_, true, Comparator_> & b)
    {
        return Comparator_()(a.data(), b.data());
    }

    /**
     * Writing a Validated instance to a stream is done by its data.
     *
     * \ingroup g_data_structures
     */
    template <typename D_, typename V_, bool c_, typename C_>
    std::ostream &
    operator<< (std::ostream & s, const Validated<D_, V_, c_, C_> & v)
    {
        s << v.data();
        return s;
    }
}

#endif
