/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2010 Ciaran McCreesh
 * Copyright (c) 2009 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PTY_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PTY_HH 1

#include <paludis/util/channel.hh>
#include <paludis/util/exception.hh>

/** \file
 * Declaration for the Pty class.
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
     * Thrown if a pty cannot be allocated and opened.
     *
     * \ingroup g_exceptions
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PtyError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            PtyError(const std::string & message) noexcept;

            ///\}
    };

    /**
     * Wrapper around pty file descriptors.
     *
     * \ingroup g_system
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Pty :
        public Channel
    {
        public:
            ///\name Basic operations
            ///\{

            Pty();
            explicit Pty(const bool close_exec);
            explicit Pty(const bool close_exec, const unsigned short columns, const unsigned short lines);

            virtual ~Pty();

            ///\}

    };

}

#endif
