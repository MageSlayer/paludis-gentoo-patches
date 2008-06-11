/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/environment-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_tree.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/package_database-fwd.hh>
#include <paludis/selection-fwd.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup g_environment
 *
 * \section Examples
 *
 * - \ref example_environment.cc "example_environment.cc"
 */

namespace paludis
{
    /**
     * Represents a working environment, which contains an available packages
     * database and provides various methods for querying package visibility
     * and options.
     *
     * Contains a PackageDatabase, which in turn contains a number of Repository
     * instances.
     *
     * Environment itself is purely an interface class. Actual Environment
     * implementations usually descend from EnvironmentImplementation, which
     * provides much of the common implementation details. EnvironmentMaker is
     * often used to create the appropriate Environment subclass for an
     * application.
     *
     * \ingroup g_environment
     * \see PackageDatabase
     * \see EnvironmentMaker
     * \see EnvironmentImplementation
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Environment :
        private InstantiationPolicy<Environment, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~Environment() = 0;

            ///\}

            ///\name Use-related queries
            ///\{

            /**
             * Is a particular use flag enabled for a particular package?
             */
            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a collection of known use flag names for a particular package that start
             * with a particular use expand prefix.
             *
             * It is up to subclasses to decide whether to return all known use flags with
             * the specified prefix or merely all enabled use flags. It is not safe to assume
             * that all flags in the returned value will be enabled for the specified package.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameSet> known_use_expand_names(
                    const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Mask-related queries
            ///\{

            /**
             * Do we accept a particular license for a particular package?
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we accept any of the specified keywords for a particular package?
             *
             * If the collection includes "*", should return true.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual bool accept_keywords(std::tr1::shared_ptr<const KeywordNameSet>, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we have a 'breaks' mask for a particular package?
             *
             * Returns a zero pointer if no.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual const std::tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we have a 'user' mask for a particular package?
             *
             * Returns a zero pointer if no.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we have a user unmask for a particular package?
             *
             * This is only applied to repository and profile style masks, not
             * keywords, licences etc. If true, user_mask shouldn't be used.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Database-related functions
            ///\{

            virtual std::tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Select some packages.
             */
            virtual std::tr1::shared_ptr<PackageIDSequence> operator[] (const Selection &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name System information
            ///\{

            /**
             * Return a collection of bashrc files to be used by the various components
             * that are implemented in bash.
             */
            virtual std::tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for syncer scripts.
             */
            virtual std::tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for fetcher scripts.
             */
            virtual std::tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for hooks.
             */
            virtual std::tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return the command used to launch paludis (the client).
             */
            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Change the command used to launch paludis (the client).
             */
            virtual void set_paludis_command(const std::string &) = 0;

            /**
             * Our root location for installs.
             */
            virtual const FSEntry root() const = 0;

            /**
             * User id to use when reduced privs are permissible.
             */
            virtual uid_t reduced_uid() const = 0;

            /**
             * Group id to use when reduced privs are permissible.
             */
            virtual gid_t reduced_gid() const = 0;

            /**
             * Is the specified package Paludis?
             *
             * Used by InstallTask to decide whether to exec() after installing
             * a package.
             */
            virtual bool is_paludis_package(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Mirror information
            ///\{

            /**
             * Return the mirror URI prefixes for a named mirror.
             */
            virtual std::tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Package sets
            ///\{

            /**
             * Return all known named sets.
             */
            virtual std::tr1::shared_ptr<const SetNameSet> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a named set.
             *
             * If the named set is not known, returns a zero pointer.
             */
            virtual std::tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Destination information
            ///\{

            /**
             * Default destination candidates for installing packages.
             */
            virtual std::tr1::shared_ptr<const DestinationsSet> default_destinations() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Hook methods
            ///\{

            /**
             * Perform a hook.
             */
            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Distribution information
            ///\{

            virtual std::string distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name World functionality
            ///\{

            /**
             * Add this package to world.
             */
            virtual void add_to_world(const QualifiedPackageName &) const = 0;

            /**
             * Add this set to world.
             */
            virtual void add_to_world(const SetName &) const = 0;

            /**
             * Remove this package from world, if it is present.
             */
            virtual void remove_from_world(const QualifiedPackageName &) const = 0;

            /**
             * Remove this set from world, if it is present.
             */
            virtual void remove_from_world(const SetName &) const = 0;

            ///\}
    };
}

#endif
