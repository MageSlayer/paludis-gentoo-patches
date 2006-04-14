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

#ifndef PALUDIS_GUARD_PALUDIS_VALIDATED_HH
#define PALUDIS_GUARD_PALUDIS_VALIDATED_HH 1

#include <ostream>
#include <paludis/util/comparison_policy.hh>

namespace paludis
{
    /**
     * A Validated wraps a particular class instance, ensuring that it always
     * meets certain validation criteria.
     */
    template <typename ValidatedDataType_, typename Validator_>
    class Validated : public ComparisonPolicy<
                          Validated<ValidatedDataType_, Validator_>,
                          comparison_mode::FullComparisonTag,
                          comparison_method::CompareByMemberTag<ValidatedDataType_> >
    {
        private:
            ValidatedDataType_ _value;

        public:
            /**
             * Copy constructor (no validation needed).
             */
            Validated(const Validated<ValidatedDataType_, Validator_> & other) :
                ComparisonPolicy<
                    Validated<ValidatedDataType_, Validator_>,
                    comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberTag<ValidatedDataType_> >
                        (other),
                _value(other._value)
            {
            }

            /**
             * Constructor (validation needed).
             */
            explicit Validated(const ValidatedDataType_ & value) :
                ComparisonPolicy<
                    Validated<ValidatedDataType_, Validator_>,
                    comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberTag<ValidatedDataType_> >
                        (&Validated<ValidatedDataType_, Validator_>::_value),
                _value(value)
            {
                Validator_::validate(_value);
            }

            /**
             * Assignment (no validation needed).
             */
            const Validated<ValidatedDataType_, Validator_> & operator=
                (const Validated<ValidatedDataType_,Validator_> & other)
            {
                _value = other._value;
                return *this;
            }

            /**
             * Fetch to our ValidatedDataType_. This should not be a cast
             * operator to avoid problems with ambiguous comparison operators.
             */
            const ValidatedDataType_ & data() const
            {
                return _value;
            }
    };

    /**
     * Writing a Validated instance to a stream is done by its data.
     */
    template <typename D_, typename V_>
    std::ostream &
    operator<< (std::ostream & s, const Validated<D_, V_> & v)
    {
        s << v.data();
        return s;
    }
}

#endif
