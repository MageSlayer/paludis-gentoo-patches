/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_EXCEPTION_TO_DEBUG_STRING_HH
#define PALUDIS_GUARD_PALUDIS_EXCEPTION_TO_DEBUG_STRING_HH 1

#include <paludis/stringify.hh>
#include <paludis/exception.hh>
#include <paludis/attributes.hh>
#include <exception>

#ifdef PALUDIS_TEST_CASE
#ifdef PALUDIS_CAN_USE_ATTRIBUTE

std::string exception_to_debug_string(const std::exception & e) PALUDIS_ATTRIBUTE((noinline));

#endif
#else
#error Missing -DPALUDIS_TEST_CASE=1?
#endif

#endif
