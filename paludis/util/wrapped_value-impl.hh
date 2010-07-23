/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_VALUE_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_VALUE_IMPL_HH 1

#include <paludis/util/wrapped_value.hh>
#include <ostream>

namespace paludis
{
    template <typename Tag_, typename Extra_>
    struct WrappedValueValidate
    {
        typedef WrappedValueTraits<Tag_> Type;
    };

    template <typename Tag_>
    struct WrappedValueValidate<Tag_, void>
    {
        typedef WrappedValueValidate<Tag_, void> Type;

        static bool validate(const typename WrappedValueTraits<Tag_>::UnderlyingType & t, const NoType<0u> *)
        {
            return WrappedValueTraits<Tag_>::validate(t);
        }
    };

    template <typename Tag_>
    WrappedValue<Tag_>::WrappedValue(
            const typename WrappedValueTraits<Tag_>::UnderlyingType & v,
            const typename WrappedValueDevoid<typename WrappedValueTraits<Tag_>::ValidationParamsType>::Type & p)
    {
        if (WrappedValueValidate<Tag_, typename WrappedValueTraits<Tag_>::ValidationParamsType>::Type::validate(v, p))
            _value = std::make_shared<typename WrappedValueTraits<Tag_>::UnderlyingType>(v);
        else
            throw typename WrappedValueTraits<Tag_>::ExceptionType(v);
    }

    template <typename Tag_>
    WrappedValue<Tag_> &
    WrappedValue<Tag_>::WrappedValue::operator= (const WrappedValue & v)
    {
        _value = v._value;
        return *this;
    }

    template <typename Tag_>
    bool
    WrappedValue<Tag_>::WrappedValue::operator< (const WrappedValue & other) const
    {
        return value() < other.value();
    }

    template <typename Tag_>
    bool
    WrappedValue<Tag_>::WrappedValue::operator== (const WrappedValue & other) const
    {
        return value() == other.value();
    }

    template <typename Tag_>
    WrappedValue<Tag_>::WrappedValue(const WrappedValue & other) :
        _value(other._value)
    {
    }

    template <typename Tag_>
    WrappedValue<Tag_>::~WrappedValue()
    {
    }

    template <typename Tag_>
    const typename WrappedValueTraits<Tag_>::UnderlyingType &
    WrappedValue<Tag_>::value() const
    {
        return *_value;
    }

    template <typename Tag_>
    std::ostream & operator<< (std::ostream & s, const WrappedValue<Tag_> & v)
    {
        s << v.value();
        return s;
    }
}

#endif
