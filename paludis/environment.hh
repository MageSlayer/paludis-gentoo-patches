/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/mask_reasons.hh>
#include <paludis/name.hh>
#include <paludis/package_database.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup Environment
 */

namespace paludis
{
    /**
     * Represents a working environment, which contains an available packages
     * database and provides various methods for querying package visibility
     * and options.
     *
     * \ingroup Environment
     */
    class Environment :
        private InstantiationPolicy<Environment, instantiation_method::NonCopyableTag>
    {
        private:
            PackageDatabase::Pointer _package_database;

            mutable bool _has_provide_map;

            mutable std::map<QualifiedPackageName, QualifiedPackageName> _provide_map;

        protected:
            /**
             * Constructor.
             */
            Environment(PackageDatabase::Pointer);

        public:
            /**
             * Does the user want the specified USE flag set for a
             * particular package?
             */
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const = 0;

            /**
             * Is the specified KEYWORD accepted?
             */
            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const) const = 0;

            /**
             * Is the specified LICENSE accepted?
             */
            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const = 0;

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
            PackageDatabase::Pointer package_database() const
            {
                return _package_database;
            }

            /**
             * Our bashrc files.
             */
            virtual std::string bashrc_files() const = 0;

            /**
             * How to run paludis.
             */
            virtual std::string paludis_command() const = 0;

            /**
             * Destructor.
             */
            virtual ~Environment();

            typedef std::map<QualifiedPackageName, QualifiedPackageName>::const_iterator ProvideMapIterator;

            ProvideMapIterator begin_provide_map() const;

            ProvideMapIterator end_provide_map() const;

            DepAtom::Pointer package_set(const std::string &) const;

            void add_appropriate_to_world(DepAtom::ConstPointer) const;
            void remove_appropriate_from_world(DepAtom::ConstPointer) const;
    };
}

#endif
