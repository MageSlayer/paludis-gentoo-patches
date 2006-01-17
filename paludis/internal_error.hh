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

#ifndef PALUDIS_GUARD_PALUDIS_INTERNAL_ERROR_HH
#define PALUDIS_GUARD_PALUDIS_INTERNAL_ERROR_HH 1

#include <paludis/exception.hh>

/** \file
 * Declaration for the InternalError exception class.
 */

namespace paludis
{
    /**
     * An InternalError is an Exception that is thrown if something that is
     * never supposed to happen happens.
     */
    class InternalError : public Exception
    {
        public:
            /**
             * Constructor.
             *
             * \param function Should be set to the __PRETTY_FUNCTION__ magic
             * constant.
             *
             * \param message A short message.
             */
            InternalError(const char * const function, const std::string & message) throw ();

            /**
             * Constructor, with no message (deprecated).
             *
             * \param function Should be set to the __PRETTY_FUNCTION__ magic
             * constant.
             *
             * \deprecated Use paludis::InternalError::InternalError(const char * const,
             * const std::string &) instead.
             */
            InternalError(const char * const function) throw () PALUDIS_ATTRIBUTE((deprecated));
    };
}

#endif
