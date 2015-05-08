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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EXCEPTIONS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EXCEPTIONS_HH 1

#include <paludis/util/exception.hh>

/** \file
 * Declaration for the ERepository exception classes.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    /**
     * Thrown if invalid parameters are provided for
     * ERepository::make_e_repository.
     *
     * \ingroup grpexceptions
     * \ingroup grperepository
     */
    class PALUDIS_VISIBLE ERepositoryConfigurationError :
        public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            ERepositoryConfigurationError(const std::string & msg) noexcept;
    };

    /**
     * Thrown when a distfile is missing in
     * ERepository::make_manifest.
     *
     * \ingroup grpexceptions
     * \ingroup grperepository
     * \since 0.46
     */
    class PALUDIS_VISIBLE MissingDistfileError :
        public Exception
    {
        public:
            MissingDistfileError(const std::string &) noexcept;
    };

}

#endif
