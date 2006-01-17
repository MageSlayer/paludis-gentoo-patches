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

#ifndef PALUDIS_GUARD_PALUDIS_NO_SUCH_PACKAGE_ERROR_HH
#define PALUDIS_GUARD_PALUDIS_NO_SUCH_PACKAGE_ERROR_HH 1

#include <paludis/package_database_lookup_error.hh>

/** \file
 * Declaration for the NoSuchPackageError exception class.
 */

namespace paludis
{
    /**
     * Thrown if there is no Package in a PackageDatabase with the given
     * name.
     */
    class NoSuchPackageError : public PackageDatabaseLookupError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchPackageError(const std::string & name) throw ();
    };
}

#endif
