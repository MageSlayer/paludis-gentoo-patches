/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/options.hh>
#include <paludis/util/collection.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/mask_reasons.hh>
#include <paludis/name.hh>
#include <paludis/hook.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec-fwd.hh>

/** \file
 * Declarations for the Environment class.
 *
 * \ingroup grpenvironment
 */

namespace paludis
{
    class PackageDatabase;
    class PackageDatabaseEntry;

#include <paludis/environment-se.hh>

    /**
     * Options for Environment::mask_reasons().
     *
     * \see Environment
     * \see MaskReasonsOption
     * \ingroup grpenvironment
     */
    typedef Options<MaskReasonsOption> MaskReasonsOptions;

    /**
     * A collection of mirror prefixes.
     *
     * \see Environment
     * \ingroup grpenvironment
     */
    typedef SequentialCollection<std::string> MirrorsCollection;

    /**
     * Represents a working environment, which contains an available packages
     * database and provides various methods for querying package visibility
     * and options.
     *
     * Contains a PackageDatabase, which in turn contains a number of Repository
     * instances.
     *
     * Environment itself is purely an interface class. Actual Environment
     * implementations descend from EnvironmentImplementation, which provides
     * much of the common implementation details. EnvironmentMaker is often
     * used to create the appropriate Environment subclass for an application.
     *
     * \ingroup grpenvironment
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
            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a collection of known use flag names for a particular package that start
             * with a particular use expand prefix.
             *
             * It is up to subclasses to decide whether to return all known use flags with
             * the specified prefix or merely all enabled use flags. It is not safe to assume
             * that all flags in the returned value will be enabled for the specified package.
             */
            virtual tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(
                    const UseFlagName &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Mask-related queries
            ///\{

            /**
             * Return the reasons for a package being masked.
             *
             * \see paludis::query::NotMasked
             */
            virtual MaskReasons mask_reasons(const PackageDatabaseEntry &,
                    const MaskReasonsOptions & = MaskReasonsOptions()) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we accept a particular license for a particular package?
             *
             * Default behaviour: true.
             */
            virtual bool accept_license(const std::string &, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we accept any of the specified keywords for a particular package?
             *
             * If the collection includes "*", should return true.
             *
             * Default behaviour: true if the collection includes "*".
             */
            virtual bool accept_keywords(tr1::shared_ptr<const KeywordNameCollection>, const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Database-related functions
            ///\{

            virtual tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name System information
            ///\{

            /**
             * Return a collection of bashrc files to be used by the various components
             * that are implemented in bash.
             */
            virtual tr1::shared_ptr<const FSEntryCollection> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for syncer scripts.
             */
            virtual tr1::shared_ptr<const FSEntryCollection> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for fetcher scripts.
             */
            virtual tr1::shared_ptr<const FSEntryCollection> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for hooks.
             */
            virtual tr1::shared_ptr<const FSEntryCollection> hook_dirs() const
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

            ///\}

            ///\name Mirror information
            ///\{

            /**
             * Return the mirror URI prefixes for a named mirror.
             */
            virtual tr1::shared_ptr<const MirrorsCollection> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Package sets
            ///\{

            /**
             * Return all known named sets.
             */
            virtual tr1::shared_ptr<const SetNameCollection> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a named set.
             *
             * If the named set is not known, returns a zero pointer.
             */
            virtual tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Destination information
            ///\{

            /**
             * Default destination candidates for installing packages.
             */
            virtual tr1::shared_ptr<const DestinationsCollection> default_destinations() const
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
    };
}

#endif
