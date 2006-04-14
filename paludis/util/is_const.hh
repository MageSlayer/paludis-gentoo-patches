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

#ifndef PALUDIS_GUARD_PALUDIS_IS_CONST_HH
#define PALUDIS_GUARD_PALUDIS_IS_CONST_HH 1

/** \file
 * Declarations for the IsConst class.
 */

namespace paludis
{
    /**
     * The value member will be true if a type is const and false otherwise.
     */
    template <typename T_>
    struct IsConst
    {
        /// Are we const?
        static const bool value = false;
    };

    /**
     * IsConst: specialisation for const.
     */
    template <typename T_>
    struct IsConst<const T_>
    {
        /// Are we const?
        static const bool value = true;
    };

    /**
     * IsConst: specialisation for const references.
     */
    template <typename T_>
    struct IsConst<const T_ &>
    {
        /// Are we const?
        static const bool value = true;
    };
}

#endif
