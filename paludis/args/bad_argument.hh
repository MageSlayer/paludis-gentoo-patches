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

#ifndef PALUDIS_GUARD_ARGS_BAD_ARGUMENT_HH
#define PALUDIS_GUARD_ARGS_BAD_ARGUMENT_HH 1

#include <paludis/args/args_error.hh>

/** \file
 * Declarations for BadArgument.
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
         * Thrown if an unrecognised command line argument is specified.
         *
         * \ingroup g_args
         * \ingroup g_exceptions
         */
        class PALUDIS_VISIBLE BadArgument :
            public ArgsError
        {
            public:
                /**
                 * Constructor.
                 */
                BadArgument(const std::string & option) noexcept;
        };
    }
}

#endif
