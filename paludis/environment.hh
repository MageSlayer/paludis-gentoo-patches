/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/tribool-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/package_database-fwd.hh>
#include <paludis/selection-fwd.hh>
#include <paludis/selection_cache-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <paludis/notifier_callback-fwd.hh>

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
     * provides much of the common implementation details. EnvironmentFactory is
     * often used to create the appropriate Environment subclass for an
     * application.
     *
     * \ingroup g_environment
     * \see PackageDatabase
     * \see EnvironmentFactory
     * \see EnvironmentImplementation
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Environment :
        private InstantiationPolicy<Environment, instantiation_method::NonCopyableTag>,
        public MetadataKeyHolder
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~Environment() = 0;

            ///\}

            ///\name Choice-related queries
            ///\{

            /**
             * Do we want a choice enabled for a particular package?
             *
             * Only for use by Repository, to get defaults from the environment.
             * Clients should query the metadata key directly.
             *
             * The third parameter is the name of the value, which might not
             * have been created yet.
             */
            virtual const Tribool want_choice_enabled(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * What string value, if any, is set for the parameter for a particular
             * choice for a particular package?
             *
             * There is no difference between "not set" and "set to an empty
             * string".
             *
             * Only for use by Repository, to get defaults from the environment.
             * Clients should query the metadata key directly.
             *
             * The third parameter is the name of the value, which might not
             * have been created yet.
             *
             * \since 0.40
             */
            virtual const std::string value_for_choice_parameter(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a collection of known value names for a particular
             * choice.
             *
             * Only for use by Repository, to get defaults from the environment.
             * Clients should query the metadata key directly.
             *
             * This is to deal with cases like USE_EXPAND values, where the
             * repository doesn't know all possible values.
             */
            virtual std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &
                    ) const
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
            virtual bool accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> &, const PackageID &) const
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
             * If the second parameter is true, return a Mask suitable for
             * being added to an OverriddenMask.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             */
            virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID &,
                    const bool will_be_used_for_overridden) const
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

            /**
             * Add a selection cache.
             *
             * Probably only to be used by ScopedSelectionCache.
             *
             * \since 0.42
             */
            virtual void add_selection_cache(
                    const std::tr1::shared_ptr<const SelectionCache> &) = 0;

            /**
             * Remove a selection cache registered using add_selection_cache.
             *
             * Probably only to be used by ScopedSelectionCache.
             *
             * \since 0.42
             */
            virtual void remove_selection_cache(
                    const std::tr1::shared_ptr<const SelectionCache> &) = 0;

            /**
             * Create a repository from a particular file.
             *
             * Does not add the repository to the PackageDatabase.
             *
             * This allows RepositoryRepository to add a repo config file, then
             * sync that repo. If you aren't RepositoryRepository you shouldn't
             * be calling this.
             *
             * \since 0.48
             */
            virtual const std::tr1::shared_ptr<Repository> repository_from_new_config_file(
                    const FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

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
             * Add a package set.
             *
             * Generally called by repositories, when Repository::populate_sets is called.
             *
             * \param base_name The basic name of the set, such as 'security'.
             *
             * \param combined_name The name to use for this set when combine is true, such
             *     as 'security.myrepo'. If combine is false, should be the same as base_name.
             *
             * \param func A function that returns the set.
             *
             * \param combine If true, rename the set from foo to foo.reponame, and make
             *     the foo set contain foo.reponame, along with any other repositories'
             *     sets named foo. If false, throw if the set already exists.
             *
             * \since 0.40
             */
            virtual void add_set(
                    const SetName & base_name,
                    const SetName & combined_name,
                    const std::tr1::function<std::tr1::shared_ptr<const SetSpecTree> ()> & func,
                    const bool combine) const = 0;

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
            virtual const std::tr1::shared_ptr<const SetSpecTree> set(const SetName &) const
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

            ///\name Specific metadata keys
            ///\{

            /**
             * The format_key, if non-zero, holds our environment's format. Environment
             * implementations should not return zero here, but clients should still
             * check.
             */
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const = 0;

            /**
             * The config_location_key, if non-zero, specifies the location of the configuration file or directory,
             * the contents of which depends on the format returned by format_key.
             */
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > config_location_key() const = 0;

            ///\}

            ///\name Output management
            ///\{

            /**
             * Create an output manager.
             *
             * \since 0.36
             */
            virtual const std::tr1::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const = 0;

            /**
             * Set a callback function to use when a particular event occurs.
             *
             * The return value can be passed to remove_notifier_callback.
             *
             * \since 0.40
             */
            virtual NotifierCallbackID add_notifier_callback(const NotifierCallbackFunction &) = 0;

            /**
             * Remove a function added with add_notifier_callback.
             *
             * \since 0.40
             */
            virtual void remove_notifier_callback(const NotifierCallbackID) = 0;

            /**
             * Trigger a notifier callback.
             *
             * \since 0.40
             */
            virtual void trigger_notifier_callback(const NotifierCallbackEvent &) const = 0;

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<CreateOutputManagerForRepositorySyncInfo>;
    extern template class PrivateImplementationPattern<CreateOutputManagerForPackageIDActionInfo>;
#endif

}

#endif
