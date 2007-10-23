/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAKE_SHARED_PTR_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAKE_SHARED_PTR_FWD_HH 1

#include <paludis/util/attributes.hh>

/** \file
 * Forward declarations for paludis/make_shared_ptr.hh .
 *
 * \ingroup g_utils
 */

namespace paludis
{
    namespace tr1
    {
        template <typename T_>
        class shared_ptr;
    }

    /**
     * Convenience function for creating a tr1::shared_ptr<> from a newly
     * constructed object.
     *
     * Use this only with <code>new T_(whatever)</code> as the parameter. Do not
     * use it to try to create a tr1::shared_ptr<> from something that is not
     * newly allocated.
     *
     * \ingroup g_utils
     */
    template <typename T_>
    tr1::shared_ptr<T_>
    make_shared_ptr(T_ * const t) PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
