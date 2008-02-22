/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_STRIP_HH
#define PALUDIS_GUARD_PALUDIS_STRIP_HH 1

#include <functional>
#include <string>
#include <paludis/util/attributes.hh>

/** \file
 * Strip functions and adapters.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Return a string equal to s, minus the string remove if remove occurs at
     * the start of s.
     *
     * \ingroup g_strings
     */
    std::string strip_leading_string(const std::string & s, const std::string & remove) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));

    /**
     * Return a string equal to s minus any leading characters that are
     * contained in prefix.
     *
     * \ingroup g_strings
     */
    std::string strip_leading(const std::string & s, const std::string & prefix) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));

    /**
     * Return a string equal to s, minus the string remove if remove occurs at
     * the end of s.
     *
     * \ingroup g_strings
     */
    std::string strip_trailing_string(const std::string & s, const std::string & remove) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));

    /**
     * Return a string equal to s minus any trailing characters that are
     * contained in suffix.
     *
     * \ingroup g_strings
     */
    std::string strip_trailing(const std::string & s, const std::string & suffix) PALUDIS_VISIBLE
        PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
