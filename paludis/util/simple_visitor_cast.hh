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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_VISITOR_CAST_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_VISITOR_CAST_HH 1

#include <paludis/util/simple_visitor_cast-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <tr1/type_traits>

namespace paludis
{
    template <typename To_, typename From_>
    struct SimpleVisitorCaster
    {
        To_ * visit(const To_ & t)
        {
            return &t;
        }

        To_ * visit(const From_ &)
        {
            return 0;
        }
    };

    template <typename To_, typename From_>
    To_ * simple_visitor_cast(const From_ & from)
    {
        SimpleVisitorCaster<To_, From_> q;
        return from.template accept_returning<To_ *>(q);
    }
}

#endif
