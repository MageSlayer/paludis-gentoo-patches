/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_CLONE_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_CLONE_IMPL_HH 1

#include <paludis/util/clone.hh>

/** \file
 * Implementation for paludis/clone.hh .
 *
 * \ingroup g_oo
 */

namespace paludis
{
    template <typename T_>
    Cloneable<T_>::~Cloneable()
    {
    }

    template<typename Base_, typename Child_>
    std::shared_ptr<Base_>
    CloneUsingThis<Base_, Child_>::clone() const
    {
        return std::shared_ptr<Base_>(new Child_(*static_cast<const Child_ *>(this)));
    }

    template<typename Base_, typename Child_>
    CloneUsingThis<Base_, Child_>::~CloneUsingThis()
    {
    }
}

#endif

