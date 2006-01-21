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

#ifndef PALUDIS_GUARD_PALUDIS_CATEGORY_NAME_PART_HH
#define PALUDIS_GUARD_PALUDIS_CATEGORY_NAME_PART_HH 1

#include <paludis/validated.hh>
#include <paludis/name_error.hh>
#include <paludis/private_implementation_pattern.hh>
#include <string>

/** \file
 * Declaration for CategoryNamePart and related classes.
 *
 * \ingroup Database
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * A CategoryNamePartError is thrown if an invalid value is assigned to
     * a CategoryNamePart.
     *
     * \ingroup Exception
     * \ingroup Database
     */
    class CategoryNamePartError : public NameError
    {
        public:
            /**
             * Constructor.
             */
            CategoryNamePartError(const std::string & name) throw ();
    };

    /**
     * A CategoryNamePartValidator handles validation rules for the value
     * of a CategoryNamePart.
     *
     * \ingroup Database
     */
    struct CategoryNamePartValidator :
        private InstantiationPolicy<CategoryNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for a CategoryNamePart,
         * throw a CategoryNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * A CategoryNamePart holds a std::string that is a valid name for the
     * category part of a QualifiedPackageName.
     *
     * \ingroup Database
     */
    typedef Validated<std::string, CategoryNamePartValidator> CategoryNamePart;
}

#endif
