/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PRIVATE_IMPLEMENTATION_PATTERN_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PRIVATE_IMPLEMENTATION_PATTERN_IMPL_HH 1

#include <paludis/util/pimp.hh>
#include <utility>

/** \file
 * Imp for paludis/util/pimp.hh .
 *
 * \ingroup g_oo
 */

template <typename C_>
template <typename... Args_>
paludis::Pimp<C_>::Pimp(Args_ && ... args) :
    _ptr(new Imp<C_>{std::forward<Args_>(args)...})
{
}

template <typename C_>
paludis::Pimp<C_>::Pimp(Pimp && other) :
    _ptr(std::move(other._ptr))
{
    other._ptr = 0;
}

template <typename C_>
paludis::Pimp<C_>::~Pimp()
{
    delete _ptr;
}

template <typename C_>
paludis::Imp<C_> *
paludis::Pimp<C_>::get()
{
    return _ptr;
}

template <typename C_>
const paludis::Imp<C_> *
paludis::Pimp<C_>::get() const
{
    return _ptr;
}

template <typename C_>
void
paludis::Pimp<C_>::reset(Imp<C_> * p)
{
    delete _ptr;
    _ptr = p;
}

#endif
