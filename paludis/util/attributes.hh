/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ATTRIBUTES_HH
#define PALUDIS_GUARD_PALUDIS_ATTRIBUTES_HH 1

/** \file
 * Declare the PALUDIS_ATTRIBUTE macro.
 */

/** \def PALUDIS_ATTRIBUTE
 * If we're using a recent GCC or ICC, expands to __attribute__, otherwise
 * discards its arguments.
 */

/** \def PALUDIS_CAN_USE_ATTRIBUTE
 * Defined if we can rely upon PALUDIS_ATTRIBUTE working (for example, for
 * weak).
 */

#if (defined(__GNUC__) || defined(DOXYGEN))
#  if ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || defined(DOXYGEN))
#    define PALUDIS_ATTRIBUTE(x) __attribute__(x)
#    define PALUDIS_CAN_USE_ATTRIBUTE 1
#  else
#    define PALUDIS_ATTRIBUTE(x)
#  endif
#else
#  define PALUDIS_ATTRIBUTE(x)
#endif

#endif
