/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_COMMON_ARGS_CHECKS_ARG_HH
#define PALUDIS_GUARD_SRC_COMMON_ARGS_CHECKS_ARG_HH 1

#include <paludis/args/args_option.hh>
#include <paludis/action-fwd.hh>

/** \file
 * Declarations for the Checks class.
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
         * The '--checks' standard command line argument.
         *
         * \since 0.26
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE ChecksArg :
            public EnumArg
        {
            public:
                ///\name Basic operations
                ///\{

                ChecksArg(ArgsGroup * const, const std::string &, char);
                ~ChecksArg();

                ///\}

                /**
                 * Our selected value, as an InstallActionChecksOption.
                 */
                InstallActionChecksOption option() const;
        };
    }
}

#endif
