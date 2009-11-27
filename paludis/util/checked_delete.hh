/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_CHECKED_DELETE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_CHECKED_DELETE_HH 1

namespace paludis
{
    template <typename T_>
    void checked_delete(T_ * const t)
    {
        typedef char make_sure_type_is_defined[sizeof(T_) ? 1 : -1];
        static const int make_sure_type_is_defined_again PALUDIS_ATTRIBUTE((unused)) = sizeof(make_sure_type_is_defined);
        delete t;
    }
}

#endif
