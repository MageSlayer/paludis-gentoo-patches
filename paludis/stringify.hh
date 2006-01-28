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

#ifndef PALUDIS_GUARD_PALUDIS_STRINGIFY_HH
#define PALUDIS_GUARD_PALUDIS_STRINGIFY_HH 1

#include <string>
#include <sstream>

/** \file
 * Stringify functions.
 */

namespace paludis
{
    /**
     * Convert item to a string.
     */
    template <typename T_>
    std::string
    stringify(const T_ & item)
    {
        std::ostringstream s;
        s << item;
        return s.str();
    }

    /**
     * Convert item to a string (overload for std::string).
     */
    inline std::string
    stringify(const std::string & item)
    {
        return item;
    }

    /**
     * Convert item to a string (overload for char).
     */
    inline std::string
    stringify(const char & item)
    {
        return std::string(1, item);
    }

    /**
     * Convert item to a string (overload for unsigned char).
     */
    inline std::string
    stringify(const unsigned char & item)
    {
        return std::string(1, item);
    }

    /**
     * Convert item to a string (overload for bool).
     */
    inline std::string
    stringify(const bool & item)
    {
        return item ? "true" : "false";
    }

    /**
     * Convert item to a string (undefined overload for pointers, to
     * catch screwups at compiletime rather than printing out addresses in
     * weird places because of a missing *).
     */
    template <typename T_>
    void
    stringify(const T_ * const item);

    /**
     * Convert item to a string (overload for char *, which isn't a
     * screwup like other pointers).
     */
    inline std::string
    stringify(const char * const item)
    {
        return std::string(item);
    }
}

#endif
