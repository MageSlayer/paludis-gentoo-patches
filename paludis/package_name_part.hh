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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_NAME_PART_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_NAME_PART_HH 1

#include <paludis/util/validated.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sorted_collection.hh>
#include <string>

/** \file
 * Declarations for PackageNamePart and related classes.
 *
 * \ingroup Database
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A PackageNamePartError is thrown if an invalid value is assigned to
     * a PackageNamePart.
     *
     * \ingroup Database
     * \ingroup Exception
     */
    class PackageNamePartError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            PackageNamePartError(const std::string & name) throw ();
    };

    /**
     * A PackageNamePartValidator handles validation rules for the value
     * of a PackageNamePart.
     *
     * \ingroup Database
     */
    struct PackageNamePartValidator :
        private InstantiationPolicy<PackageNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a PackageNamePart,
         * throw a PackageNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * A PackageNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, PackageNamePartValidator> PackageNamePart;

    /**
     * Holds a set of PackageNamePart instances.
     *
     * \ingroup Database
     */
    typedef SortedCollection<PackageNamePart> PackageNamePartCollection;
}

#endif
