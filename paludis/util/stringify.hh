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

#ifndef PALUDIS_GUARD_PALUDIS_STRINGIFY_HH
#define PALUDIS_GUARD_PALUDIS_STRINGIFY_HH 1

#include <sstream>
#include <string>
#include <paludis/util/attributes.hh>

/** \file
 * Stringify functions.
 *
 * \ingroup grpstringify
 */

namespace paludis
{
    template <typename T_, typename U_, typename V_>
    class CountedPtr;

    /**
     * For use by stringify.
     *
     * \ingroup grpstringify
     */
    namespace stringify_internals
    {
        template <typename T_>
        struct CheckType
        {
            enum { value = 0 } Value;
        };

        template <typename T_>
        struct CheckType<T_ *>
        {
        };

        template <typename T_, typename U_, typename V_>
        struct CheckType<CountedPtr<T_, U_, V_> >
        {
        };

        template <>
        struct CheckType<char *>
        {
            enum { value = 0 } Value;
        };
    }

    /**
     * Convert item to a string.
     *
     * \ingroup grpstringify
     */
    template <typename T_>
    std::string
    stringify(const T_ & item)
    {
        /* check that we're not trying to stringify a pointer or somesuch */
        int check_for_stringifying_silly_things
            PALUDIS_ATTRIBUTE((unused)) = stringify_internals::CheckType<T_>::value;

        std::ostringstream s;
        s << item;
        return s.str();
    }

    /**
     * Convert item to a string (overload for std::string).
     *
     * \ingroup grpstringify
     */
    inline std::string
    stringify(const std::string & item)
    {
        return item;
    }

    /**
     * Convert item to a string (overload for char).
     *
     * \ingroup grpstringify
     */
    inline std::string
    stringify(const char & item)
    {
        return std::string(1, item);
    }

    /**
     * Convert item to a string (overload for unsigned char).
     *
     * \ingroup grpstringify
     */
    inline std::string
    stringify(const unsigned char & item)
    {
        return std::string(1, item);
    }

    /**
     * Convert item to a string (overload for bool).
     *
     * \ingroup grpstringify
     */
    inline std::string
    stringify(const bool & item)
    {
        return item ? "true" : "false";
    }

    /**
     * Convert item to a string (overload for char *, which isn't a
     * screwup like other pointers).
     *
     * \ingroup grpstringify
     */
    inline std::string
    stringify(const char * const item)
    {
        return std::string(item);
    }
}

#endif
