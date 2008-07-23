/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_NAMED_VALUE_HH 1

#include <paludis/util/named_value-fwd.hh>

namespace paludis
{
    template <typename K_, typename V_>
    class NamedValue
    {
        private:
            V_ _value;

        public:
            explicit NamedValue(const V_ & v) :
                _value(v)
            {
            }

            NamedValue(const NamedValue & v) :
                _value(v._value)
            {
            }

            V_ & operator() ()
            {
                return _value;
            }

            const V_ & operator() () const
            {
                return _value;
            }
    };

    template <typename K_, typename V_>
    NamedValue<K_, V_>
    value_for(const V_ & v)
    {
        return NamedValue<K_, V_>(v);
    }

    template <typename K_>
    NamedValue<K_, std::string>
    value_for(const char * const v)
    {
        return NamedValue<K_, std::string>(v);
    }
}

#endif
