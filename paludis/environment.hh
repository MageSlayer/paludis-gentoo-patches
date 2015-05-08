/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#include <paludis/output_manager-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/selection-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <paludis/notifier_callback-fwd.hh>
#include <paludis/filter-fwd.hh>

#include <paludis/util/options-fwd.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/sequence-fwd.hh>

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
     * Thrown if a query results in more than one matching package.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     */
    class PALUDIS_VISIBLE AmbiguousPackageNameError :
        public Exception
    {
        private:
            struct NameData;
            NameData * const _name_data;

            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            AmbiguousPackageNameError(const std::string & name, const std::shared_ptr<const Sequence<std::string> > &) noexcept;

            AmbiguousPackageNameError(const AmbiguousPackageNameError &);

            virtual ~AmbiguousPackageNameError();

            ///\}

            /**
             * The name of the package.
             */
            const std::string & name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over possible matches
            ///\{

            struct OptionsConstIteratorTag;
            typedef WrappedForwardIterator<OptionsConstIteratorTag, const std::string> OptionsConstIterator;

            OptionsConstIterator begin_options() const PALUDIS_ATTRIBUTE((warn_unused_result));
            OptionsConstIterator end_options() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Thrown if a Repository with the same name as an existing member is added
     * to an Environment.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     */
    class PALUDIS_VISIBLE DuplicateRepositoryError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            DuplicateRepositoryError(const std::string & name) noexcept;
    };

    /**
     * Thrown if there is no Package with the given name.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     */
    class PALUDIS_VISIBLE NoSuchPackageError :
        public Exception
    {
        private:
            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            NoSuchPackageError(const std::string & name) noexcept;

            virtual ~NoSuchPackageError()
            {
            }

            ///\}

            /**
             * Name of the package.
             */
            const std::string & name() const
            {
                return _name;
            }
    };

    /**
     * Thrown if there is no Repository in a RepositoryDatabase with the given
     * name.
     *
     * \ingroup g_exceptions
     * \ingroup g_environment
     */
    class PALUDIS_VISIBLE NoSuchRepositoryError :
        public Exception
    {
        private:
            const RepositoryName _name;

        public:
            ///\name Basic operations
            ///\{

            NoSuchRepositoryError(const RepositoryName &) noexcept;

            ~NoSuchRepositoryError();

            ///\}

            /**
             * The name of our repository.
             */
            RepositoryName name() const;
    };

    /**
     * Represents a working environment, which contains an available packages
     * database and provides various methods for querying package visibility
     * and options.
     *
     * Holds a number of Repository instances.
     *
     * Environment itself is purely an interface class. Actual Environment
     * implementations usually descend from EnvironmentImplementation, which
     * provides much of the common implementation details. EnvironmentFactory is
     * often used to create the appropriate Environment subclass for an
     * application.
     *
     * \ingroup g_environment
     * \see EnvironmentFactory
     * \see EnvironmentImplementation
     * \see Repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Environment :
        public MetadataKeyHolder
    {
        protected:
            static const Filter & all_filter() PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            Environment() = default;
            virtual ~Environment() = 0;

            Environment(const Environment &) = delete;
            Environment & operator= (const Environment &) = delete;

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
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
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
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
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
            virtual std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Suggestion-related queries
            ///\{

            /**
             * Do we want to ignore or take a particular suggestion from a
             * particular package?
             *
             * Command line things override this.
             *
             * \since 0.58
             */
            virtual Tribool interest_in_suggestion(
                    const std::shared_ptr<const PackageID> & from_id,
                    const PackageDepSpec & spec) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Mask-related queries
            ///\{

            /**
             * Do we accept a particular license for a particular package?
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             *
             * \since 0.58 takes id by shared_ptr
             */
            virtual bool accept_license(
                    const std::string &,
                    const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Expand a licence group into its constituent licences, recursively (if any
             * of our repositories thinks it is a group).
             *
             * The original group is included in the result.
             *
             * \since 0.68
             */
            virtual const std::shared_ptr<const Set<std::string> > expand_licence(
                    const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we accept any of the specified keywords for a particular package?
             *
             * If the collection includes "*", should return true.
             *
             * Used by PackageID implementations. Generally PackageID's masks methods
             * should be used rather than calling this directly.
             *
             * \since 0.58 takes id by shared_ptr
             */
            virtual bool accept_keywords(
                    const std::shared_ptr<const KeywordNameSet> &,
                    const std::shared_ptr<const PackageID> &) const
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
             *
             * \since 0.58 takes id by shared_ptr
             */
            virtual const std::shared_ptr<const Mask> mask_for_user(
                    const std::shared_ptr<const PackageID> &,
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
             *
             * \since 0.58 takes id by shared_ptr
             * \since 0.60 takes optional extra reason string
             */
            virtual bool unmasked_by_user(
                    const std::shared_ptr<const PackageID> &,
                    const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            /**
             * Select some packages.
             */
            virtual std::shared_ptr<PackageIDSequence> operator[] (const Selection &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Create a repository from a particular file.
             *
             * Does not add the repository to the Environment.
             *
             * This allows RepositoryRepository to add a repo config file, then
             * sync that repo. If you aren't RepositoryRepository you shouldn't
             * be calling this.
             *
             * \since 0.48
             */
            virtual const std::shared_ptr<Repository> repository_from_new_config_file(
                    const FSPath &) PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name System information
            ///\{

            /**
             * Return a collection of bashrc files to be used by the various components
             * that are implemented in bash.
             */
            virtual std::shared_ptr<const FSPathSequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for syncer scripts.
             */
            virtual std::shared_ptr<const FSPathSequence> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for fetcher scripts.
             */
            virtual std::shared_ptr<const FSPathSequence> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return directories to search for hooks.
             */
            virtual std::shared_ptr<const FSPathSequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

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
            virtual std::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
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
                    const std::function<std::shared_ptr<const SetSpecTree> ()> & func,
                    const bool combine) const = 0;

            /**
             * Return all known named sets.
             */
            virtual std::shared_ptr<const SetNameSet> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a named set.
             *
             * If the named set is not known, returns a zero pointer.
             */
            virtual const std::shared_ptr<const SetSpecTree> set(const SetName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Hook methods
            ///\{

            /**
             * Perform a hook.
             *
             * \since 0.53 takes optional_output_manager
             */
            virtual HookResult perform_hook(
                    const Hook &,
                    const std::shared_ptr<OutputManager> & optional_output_manager) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Distribution information
            ///\{

            virtual std::string distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name World and configuration functionality
            ///\{

            /**
             * Add this package to world.
             *
             * \return whether anything was added \since 0.49
             */
            virtual bool add_to_world(const QualifiedPackageName &) const = 0;

            /**
             * Add this set to world.
             *
             * \return whether anything was added \since 0.49
             */
            virtual bool add_to_world(const SetName &) const = 0;

            /**
             * Remove this package from world, if it is present.
             *
             * \return whether anything was removed \since 0.49
             */
            virtual bool remove_from_world(const QualifiedPackageName &) const = 0;

            /**
             * Remove this set from world, if it is present.
             *
             * \return whether anything was removed \since 0.49
             */
            virtual bool remove_from_world(const SetName &) const = 0;

            /**
             * Where possible, update configuration files with the first spec to use the second package name.
             *
             * Does not necessarily invalidate any in-memory configuration.
             *
             * \since 0.48
             */
            virtual void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const = 0;

            ///\}

            ///\name Specific metadata keys
            ///\{

            /**
             * The preferred_root_key, which must not be null, specifies the
             * preferred filesystem root for actions.
             *
             * \since 0.54
             */
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > preferred_root_key() const = 0;

            /**
             * The system_root_key, which must not be null, specifies the
             * filesystem root for dependencies etc. This is usually "/",
             * unless something funky is going on.
             *
             * \since 0.55
             */
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > system_root_key() const = 0;

            /**
             * The format_key, if non-zero, holds our environment's format. Environment
             * implementations should not return zero here, but clients should still
             * check.
             */
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const = 0;

            /**
             * The config_location_key, if non-zero, specifies the location of the configuration file or directory,
             * the contents of which depends on the format returned by format_key.
             */
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > config_location_key() const = 0;

            ///\}

            ///\name Output management
            ///\{

            /**
             * Create an output manager.
             *
             * \since 0.36
             */
            virtual const std::shared_ptr<OutputManager> create_output_manager(
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

            ///\name Repositories
            ///\{

            /**
             * Add a repository.
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             *
             * \exception DuplicateRepositoryError if a Repository with the
             * same name as the new Repository already exists in our
             * collection.
             */
            virtual void add_repository(int importance, const std::shared_ptr<Repository> &) = 0;

            /**
             * Fetch a named repository.
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             */
            virtual const std::shared_ptr<const Repository> fetch_repository(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch a named repository.
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             */
            virtual const std::shared_ptr<Repository> fetch_repository(const RepositoryName &)
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Do we have a named repository?
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             */
            virtual bool has_repository_named(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Disambiguate a package name.  If a filter is specified,
             * limit the potential results to packages that match.
             *
             * \throw AmbiguousPackageNameError if there is no unambiguous
             * disambiguation. If disambiguate is set to false, the
             * exception will be always thrown in presence of ambiguity.
             * \since 0.56 takes the disambiguate flag.
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             */
            virtual QualifiedPackageName fetch_unique_qualified_package_name(
                    const PackageNamePart &, const Filter & = all_filter(), const bool disambiguate = true) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return true if the first repository is more important than the second.
             *
             * \since 0.61 is in Environment rather than PackageDatabase
             */
            virtual bool more_important_than(const RepositoryName &, const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            ///\name Iterate over our repositories
            ///\{

            struct RepositoryConstIteratorTag;
            typedef WrappedForwardIterator<RepositoryConstIteratorTag, const std::shared_ptr<Repository> > RepositoryConstIterator;

            virtual RepositoryConstIterator begin_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            virtual RepositoryConstIterator end_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}
    };

    extern template class Pimp<CreateOutputManagerForRepositorySyncInfo>;
    extern template class Pimp<CreateOutputManagerForPackageIDActionInfo>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Environment::RepositoryConstIteratorTag, const std::shared_ptr<Repository> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<AmbiguousPackageNameError::OptionsConstIteratorTag, const std::string>;
}

#endif
