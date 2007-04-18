/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_BROKEN_GLOBAL_VARIABLES_CHECK_HH
#define PALUDIS_GUARD_PALUDIS_QA_BROKEN_GLOBAL_VARIABLES_CHECK_HH 1

#include <paludis/qa/file_check.hh>

namespace paludis
{
    namespace qa
    {
        /**
         * QA check: abuse of pkg_setup variables in globals.
         *
         * \ingroup grpqacheck
         */
        class BrokenGlobalVariablesCheck :
            public FileCheck
        {
            public:
                BrokenGlobalVariablesCheck();

                /**
                 * Perform the check.
                 */
                CheckResult operator() (const FSEntry &) const;

                /**
                 * Fetch the check's identifier.
                 */
                static const std::string & identifier();

                /**
                 * Describe the check.
                 */
                virtual std::string describe() const
                {
                    return "Checks for abuse of pkg_setup variables in globals";
                }
        };
    }

}

#endif
