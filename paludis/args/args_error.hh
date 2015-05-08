/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_ARGS_ARGS_ERROR_HH
#define PALUDIS_GUARD_ARGS_ARGS_ERROR_HH 1

#include <paludis/util/exception.hh>
#include <string>

/** \file
 * Declarations for argument exception classes.
 *
 * \ingroup g_args
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    namespace args
    {
        /**
         * Thrown if an invalid command line argument is provided.
         *
         * \ingroup g_exceptions
         * \ingroup g_args
         */
        class PALUDIS_VISIBLE ArgsError :
            public paludis::Exception
        {
            protected:
                /**
                 * Constructor.
                 */
                ArgsError(const std::string & message) noexcept;
        };

        /**
         * Thrown if an invalid parameter is passed to a valid command line argument.
         *
         * \ingroup g_args
         * \ingroup g_exceptions
         */
        class PALUDIS_VISIBLE BadValue :
            public ArgsError
        {
            public:
                /**
                 * Constructor
                 */
                BadValue(const std::string& option, const std::string& value) noexcept;
        };

        /**
         * Thrown if an argument is specified that needs a parameter,
         * but no parameter is given.
         *
         * \ingroup g_args
         * \ingroup g_exceptions
         */
        class PALUDIS_VISIBLE MissingValue :
            public ArgsError
        {
            public:
                /**
                 * Constructor.
                 */
                MissingValue(const std::string & arg) noexcept;
        };
    }
}

#endif
