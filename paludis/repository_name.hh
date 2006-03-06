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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_NAME_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_NAME_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sequential_collection.hh>
#include <paludis/util/validated.hh>

#include <string>

/** \file
 * Declaration for RepositoryName and related classes.
 *
 * \ingroup Database
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A RepositoryNameError is thrown if an invalid value is assigned to
     * a RepositoryName.
     *
     * \ingroup Exception
     * \ingroup Database
     */
    class RepositoryNameError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            RepositoryNameError(const std::string & name) throw ();
    };

    /**
     * A RepositoryNameValidator handles validation rules for the value
     * of a RepositoryName.
     *
     * \ingroup Database
     */
    struct RepositoryNameValidator :
        private InstantiationPolicy<RepositoryNameValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a RepositoryName,
         * throw a RepositoryNameError.
         */
        static void validate(const std::string &);
    };

    /**
     * A RepositoryNamePart holds a std::string that is a valid name for a
     * Repository.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, RepositoryNameValidator> RepositoryName;

    /**
     * Holds a collection of RepositoryName instances.
     */
    typedef SequentialCollection<RepositoryName> RepositoryNameCollection;
}

#endif
