/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAKE_NAMED_VALUES_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAKE_NAMED_VALUES_HH 1

/** \file
 * A load of make_named_values functions.
 *
 * Bizarre oddity in C++98: you can only use an initialiser list when using
 * equals to initialise a newly constructed object. C++0x fixes this, but GCC
 * 4.4 is buggy, so for now we can't use braces directly...
 */

#include <utility>

namespace paludis
{
    template <typename R_, typename... T_>
    R_ make_named_values(T_ && ... a)
    {
        R_ result = { std::forward<T_>(a)... };
        return result;
    }
}

#endif
