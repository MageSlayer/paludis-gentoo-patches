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

#ifndef PALUDIS_GUARD_PALUDIS_USE_FLAG_NAME_HH
#define PALUDIS_GUARD_PALUDIS_USE_FLAG_NAME_HH 1

#include <paludis/util/validated.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declarations for UseFlagName and related classes.
 *
 * \ingroup Database
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A UseFlagNameError is thrown if an invalid value is assigned to
     * a UseFlagName.
     *
     * \ingroup Database
     * \ingroup Exception
     */
    class UseFlagNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            UseFlagNameError(const std::string & name) throw ();
    };

    /**
     * A UseFlagNameValidator handles validation rules for the value of a
     * UseFlagName.
     *
     * \ingroup Database
     */
    struct UseFlagNameValidator :
        private InstantiationPolicy<UseFlagNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a UseFlagName,
         * throw a UseFlagNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A UseFlagName holds a std::string that is a valid name for a USE flag.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, UseFlagNameValidator> UseFlagName;
}

#endif
