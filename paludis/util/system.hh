/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SYSTEM_HH 1

#include <paludis/util/exception.hh>
#include <string>
#include <sys/types.h>

/** \file
 * Various system utilities.
 *
 * \ingroup g_system
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Thrown if getenv_or_error fails.
     *
     * \ingroup g_exceptions
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GetenvError : public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            GetenvError(const std::string & key) noexcept;

            ///\}
    };

    /**
     * Fetch the value of environment variable key, or def if the variable is
     * not defined.
     *
     * \ingroup g_system
     */
    std::string getenv_with_default(const std::string & key, const std::string & def) PALUDIS_VISIBLE;

    /**
     * Fetch the value of environment variable key, or throw a GetenvError if
     * the variable is not defined.
     *
     * \ingroup g_system
     */
    std::string getenv_or_error(const std::string & key) PALUDIS_VISIBLE;

    /**
     * Fetch the kernel version, for $KV.
     *
     * \ingroup g_system
     */
    std::string kernel_version() PALUDIS_VISIBLE;

    /**
     * Fetch the username for a uid, or the uid as a string if not available.
     *
     * \ingroup g_system
     */
    std::string get_user_name(const uid_t) PALUDIS_VISIBLE;

    /**
     * Fetch the group name for a gid, or the gid as a string if not available.
     *
     * \ingroup g_system
     */
    std::string get_group_name(const gid_t) PALUDIS_VISIBLE;
}

#endif

