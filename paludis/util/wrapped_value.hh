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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_VALUE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_WRAPPED_VALUE_HH 1

#include <paludis/util/wrapped_value-fwd.hh>
#include <paludis/util/no_type.hh>
#include <paludis/util/operators.hh>
#include <memory>

namespace paludis
{
    template <typename T_>
    struct WrappedValueDevoid
    {
        typedef T_ Type;
    };

    template <>
    struct WrappedValueDevoid<void>
    {
        typedef NoType<0u> * Type;
    };

    template <typename Tag_>
    class PALUDIS_VISIBLE WrappedValue :
        public relational_operators::HasRelationalOperators
    {
        private:
            std::shared_ptr<const typename WrappedValueTraits<Tag_>::UnderlyingType> _value;

        public:
            explicit WrappedValue(
                    const typename WrappedValueTraits<Tag_>::UnderlyingType &,
                    const typename WrappedValueDevoid<typename WrappedValueTraits<Tag_>::ValidationParamsType>::Type & = static_cast<NoType<0u> *>(0)
                    );

            WrappedValue & operator= (const WrappedValue &);
            WrappedValue(const WrappedValue &);
            ~WrappedValue();

            const typename WrappedValueTraits<Tag_>::UnderlyingType & value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            bool operator< (const WrappedValue &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool operator== (const WrappedValue &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
