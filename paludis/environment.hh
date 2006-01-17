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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_HH 1

#include <paludis/package_database_entry.hh>
#include <paludis/package_database.hh>
#include <paludis/use_flag_name.hh>
#include <paludis/keyword_name.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/mask_reasons.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup Environment
 */

namespace paludis
{
    class PackageDatabase;

    /**
     * Represents a working environment, which contains an available packages
     * database and an installed packages database and provides various methods
     * for querying package visibility and options.
     *
     * \ingroup Environment
     */
    class Environment :
        private InstantiationPolicy<Environment, instantiation_method::NonCopyableTag>
    {
        private:
            PackageDatabase::Pointer _package_db;
            PackageDatabase::Pointer _installed_db;

        protected:
            /**
             * Constructor.
             */
            Environment(PackageDatabase::Pointer, PackageDatabase::Pointer);

        public:
            /**
             * Does the user want the specified USE flag set for a
             * particular package?
             */
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry &) const = 0;

            /**
             * Is the specified KEYWORD accepted?
             */
            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const = 0;

            /**
             * Fetch the masks for a particular package.
             */
            MaskReasons mask_reasons(const PackageDatabaseEntry &) const;

            /**
             * Are there any user masks on a package?
             */
            virtual bool query_user_masks(const PackageDatabaseEntry &) const = 0;

            /**
             * Are there any user unmasks on a package?
             */
            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const = 0;

            /**
             * Fetch our package database.
             */
            PackageDatabase::Pointer package_db() const
            {
                return _package_db;
            }

            /**
             * Fetch our installed database.
             */
            PackageDatabase::Pointer installed_db() const
            {
                return _installed_db;
            }

            /**
             * Destructor.
             */
            virtual ~Environment();
    };
}

#endif
