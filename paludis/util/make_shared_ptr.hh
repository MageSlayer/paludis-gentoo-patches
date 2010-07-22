/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAKE_SHARED_PTR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAKE_SHARED_PTR_HH 1

#include <paludis/util/make_shared_ptr-fwd.hh>
#include <memory>

/** \file
 * Declarations for the make_shared_ptr function.
 *
 * \ingroup g_utils
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    template <typename T_>
    std::shared_ptr<T_>
    make_shared_ptr(T_ * const t)
    {
        return std::shared_ptr<T_>(t);
    }

    /**
     * An object that can convert itself into an empty std::shared_ptr<>
     * of any type.
     *
     * \see make_null_shared_ptr()
     * \since 0.32
     * \ingroup g_utils
     */
    struct PALUDIS_VISIBLE NullSharedPtr
    {
        template <typename T_>
        inline operator std::shared_ptr<T_> () const
        {
            return std::shared_ptr<T_>();
        }
    };

    inline NullSharedPtr make_null_shared_ptr()
    {
        return NullSharedPtr();
    }
}

#endif
