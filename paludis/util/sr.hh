/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SR_HH 1

#include <paludis/util/attributes.hh>

/** \file
 * Declarations for various sr-related utility classes.
 *
 * \ingroup g_oo
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Metaprogramming: an empty class.
     *
     * \ingroup g_oo
     */
    struct Empty
    {
        /// Convenience instance.
        static const PALUDIS_VISIBLE Empty instance;
    };

    template <bool value_, typename IfTrue_, typename IfFalse_>
    struct Select;

    /**
     * Metaprogramming: select a type based upon a condition.
     *
     * \ingroup g_oo
     */
    template <typename IfTrue_, typename IfFalse_>
    struct Select<true, IfTrue_, IfFalse_>
    {
        /// Our value.
        typedef IfTrue_ Type;
    };

    /**
     * Metaprogramming: select a type based upon a condition.
     *
     * \ingroup g_oo
     */
    template <typename IfTrue_, typename IfFalse_>
    struct Select<false, IfTrue_, IfFalse_>
    {
        /// Our value.
        typedef IfFalse_ Type;
    };

    template <bool value_>
    struct SelectValue;

    /**
     * Metaprogramming: select a value based upon a condition.
     *
     * \ingroup g_oo
     */
    template <>
    struct SelectValue<true>
    {
        template <typename IfTrue_, typename IfFalse_>
        static const IfTrue_ & get(const IfTrue_ & v, const IfFalse_ &)
        {
            return v;
        }
    };

    /**
     * Metaprogramming: select a value based upon a condition.
     *
     * \ingroup g_oo
     */
    template <>
    struct SelectValue<false>
    {
        template <typename IfTrue_, typename IfFalse_>
        static const IfFalse_ & get(const IfTrue_ &, const IfFalse_ & v)
        {
            return v;
        }
    };
}

#endif
