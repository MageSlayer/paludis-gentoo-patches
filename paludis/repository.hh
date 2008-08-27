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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_HH 1

#include <paludis/action-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/name.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/qa-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/merger-fwd.hh>
#include <string>

/** \file
 * Declarations for Repository classes.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - \ref example_repository.cc "example_repository.cc"
 */

namespace paludis
{
    namespace n
    {
        struct arch;
        struct destination_interface;
        struct e_interface;
        struct environment_file;
        struct environment_variable_interface;
        struct hook_interface;
        struct image_dir;
        struct make_virtuals_interface;
        struct manifest_interface;
        struct mirrors_interface;
        struct options;
        struct package_id;
        struct path;
        struct profile;
        struct provided_by;
        struct provided_by_spec;
        struct provides_interface;
        struct qa_interface;
        struct sets_interface;
        struct status;
        struct syncable_interface;
        struct use_interface;
        struct virtual_name;
        struct virtuals_interface;
    }

    /**
     * Optional interfaces that may be provided by a Repository.
     *
     * \see Repository
     * \ingroup g_repository
     * \since 0.30
     */
    struct RepositoryCapabilities
    {
        NamedValue<n::destination_interface, RepositoryDestinationInterface *> destination_interface;
        NamedValue<n::e_interface, RepositoryEInterface *> e_interface;
        NamedValue<n::environment_variable_interface, RepositoryEnvironmentVariableInterface *> environment_variable_interface;
        NamedValue<n::hook_interface, RepositoryHookInterface *> hook_interface;
        NamedValue<n::make_virtuals_interface, RepositoryMakeVirtualsInterface *> make_virtuals_interface;
        NamedValue<n::manifest_interface, RepositoryManifestInterface *> manifest_interface;
        NamedValue<n::mirrors_interface, RepositoryMirrorsInterface *> mirrors_interface;
        NamedValue<n::provides_interface, RepositoryProvidesInterface *> provides_interface;
        NamedValue<n::qa_interface, RepositoryQAInterface *> qa_interface;
        NamedValue<n::sets_interface, RepositorySetsInterface *> sets_interface;
        NamedValue<n::syncable_interface, RepositorySyncableInterface *> syncable_interface;
        NamedValue<n::use_interface, RepositoryUseInterface *> use_interface;
        NamedValue<n::virtuals_interface, RepositoryVirtualsInterface *> virtuals_interface;
    };

    /**
     * A profiles.desc line in a Repository implementing RepositoryEInterface.
     *
     * \see Repository
     * \see RepositoryEInterface
     * \ingroup g_repository
     * \since 0.30
     */
    struct RepositoryEInterfaceProfilesDescLine
    {
        NamedValue<n::arch, std::string> arch;
        NamedValue<n::path, FSEntry> path;
        NamedValue<n::profile, std::tr1::shared_ptr<ERepositoryProfile> > profile;
        NamedValue<n::status, std::string> status;
    };

    /**
     * A provides entry in a Repository implementing RepositoryProvidesInterface.
     *
     * \see Repository
     * \see RepositoryProvidesInterface
     * \ingroup g_repository
     * \since 0.30
     */
    struct RepositoryProvidesEntry
    {
        NamedValue<n::provided_by, std::tr1::shared_ptr<const PackageID> > provided_by;
        NamedValue<n::virtual_name, QualifiedPackageName> virtual_name;
    };

    /**
     * A virtuals entry in a Repository implementing RepositoryVirtualsInterface.
     *
     * \see Repository
     * \see RepositoryVirtualsInterface
     * \ingroup g_repository
     * \since 0.30
     */
    struct RepositoryVirtualsEntry
    {
        NamedValue<n::provided_by_spec, std::tr1::shared_ptr<const PackageDepSpec> > provided_by_spec;
        NamedValue<n::virtual_name, QualifiedPackageName> virtual_name;
    };

    /**
     * Parameters for RepositoryDestinationInterface::merge.
     *
     * \see RepositoryDestinationInterface
     * \ingroup g_repository
     * \since 0.30
     */
    struct MergeParams
    {
        NamedValue<n::environment_file, FSEntry> environment_file;
        NamedValue<n::image_dir, FSEntry> image_dir;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::package_id, std::tr1::shared_ptr<const PackageID> > package_id;
    };

    /**
     * Thrown if a Set does not exist
     *
     * \ingroup g_exceptions
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchSetError :
        public Exception
    {
        private:
            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            NoSuchSetError(const std::string & name) throw ();

            virtual ~NoSuchSetError() throw ()
            {
            }

            ///\}

            /**
             * Name of the set.
             */
            const std::string & name() const
            {
                return _name;
            }
    };

    /**
     * Thrown if a Set is recursively defined
     *
     * \ingroup g_exceptions
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RecursivelyDefinedSetError :
        public Exception
    {
        private:
            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            RecursivelyDefinedSetError(const std::string & name) throw ();

            virtual ~RecursivelyDefinedSetError() throw ()
            {
            }

            ///\}

            /**
             * Name of the set.
             */
            const std::string & name() const
            {
                return _name;
            }
    };

    /**
     * A Repository provides a representation of a physical repository to a
     * PackageDatabase.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Repository :
        private InstantiationPolicy<Repository, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<Repository>,
        public RepositoryCapabilities,
        public MetadataKeyHolder
    {
        private:
            PrivateImplementationPattern<Repository>::ImpPtr & _imp;

        protected:
            ///\name Basic operations
            ///\{

            Repository(const Environment * const, const RepositoryName &, const RepositoryCapabilities &);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Repository();

            ///\}

            ///\name Repository information
            ///\{

            /**
             * Return our name.
             */
            const RepositoryName name() const PALUDIS_ATTRIBUTE((nothrow))
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Are we allowed to be favourite repository?
             */
            virtual bool can_be_favourite_repository() const;

            ///\}

            ///\name Specific metadata keys
            ///\{

            /**
             * The format_key, if non-zero, holds our repository's format. Repository
             * implementations should not return zero here, but clients should still
             * check.
             */
            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const = 0;

            /**
             * The location_key, if non-zero, holds the file or directory containing
             * our repository's data, the format of which depends on the value of
             * format_key.
             */
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const = 0;

            /**
             * The installed_root_key, if non-zero, specifies that we contain installed
             * packages at the specified root.
             *
             * This key is currently used in various places to determine whether a repository is
             * an 'installed' repository or not.
             */
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const = 0;

            ///\}

            ///\name Repository content queries
            ///\{

            /**
             * Do we have a category with the given name?
             */
            virtual bool has_category_named(const CategoryNamePart & c) const = 0;

            /**
             * Do we have a package in the given category with the given name?
             */
            virtual bool has_package_named(const QualifiedPackageName & q) const = 0;

            /**
             * Fetch our category names.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const = 0;

            /**
             * Fetch unimportant categories.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;

            /**
             * Fetch categories that contain a named package.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart & p) const;

            /**
             * Fetch our package names.
             */
            virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart & c) const = 0;

            /**
             * Fetch our IDs.
             */
            virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p) const = 0;

            /**
             * Might some of our IDs support a particular action?
             *
             * Used to optimise PackageDatabase::query. If a repository doesn't
             * support, say, InstallAction, a query can skip searching it
             * entirely when looking for installable packages.
             */
            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const = 0;

            ///\}

            ///\name Repository behaviour methods
            ///\{

            /**
             * Invalidate any in memory cache.
             */
            virtual void invalidate() = 0;

            /**
             * Invalidate cached masks.
             */
            virtual void invalidate_masks() = 0;

            /**
             * Regenerate any on disk cache.
             */
            virtual void regenerate_cache() const;

            ///\}

    };

    /**
     * Interface for handling USE flags for the Repository class.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryUseInterface
    {
        public:
            ///\name USE queries
            ///\{

            /**
             * Query the state of the specified use flag.
             */
            virtual UseFlagState query_use(const UseFlagName & u, const PackageID &) const = 0;

            /**
             * Query whether the specified use flag is masked.
             */
            virtual bool query_use_mask(const UseFlagName & u, const PackageID & pde) const = 0;

            /**
             * Query whether the specified use flag is forced.
             */
            virtual bool query_use_force(const UseFlagName & u, const PackageID & pde) const = 0;

            /**
             * Fetch all arch flags.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameSet> arch_flags() const = 0;

            /**
             * Fetch all expand flags.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameSet> use_expand_flags() const = 0;

            /**
             * Fetch all expand hidden flags.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameSet> use_expand_hidden_prefixes() const = 0;

            /**
             * Fetch all use expand prefixes.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameSet> use_expand_prefixes() const = 0;

            /**
             * Fetch the use expand separator (eg _ or :) for the
             * specified package, or null if unknown.
             */
            virtual char use_expand_separator(const PackageID & pkg) const = 0;

            /**
             * Describe a use flag.
             */
            virtual std::string describe_use_flag(const UseFlagName & n, const PackageID & pkg) const = 0;

            ///\}

            virtual ~RepositoryUseInterface();
    };

    /**
     * Interface for package sets for repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositorySetsInterface
    {
        public:
            ///\name Set queries
            ///\{

            /**
             * Fetch a package set.
             */
            virtual std::tr1::shared_ptr<SetSpecTree::ConstItem> package_set(const SetName & s) const = 0;

            /**
             * Gives a list of the names of all the sets provided by this repo.
             */
            virtual std::tr1::shared_ptr<const SetNameSet> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositorySetsInterface();
    };

    /**
     * Interface for syncing for repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositorySyncableInterface
    {
        public:
            ///\name Sync functions
            ///\{

            /**
             * Sync, if necessary.
             *
             * \return True if we synced successfully, false if we skipped sync.
             */
            virtual bool sync() const = 0;

            ///\}

            virtual ~RepositorySyncableInterface();
    };

    /**
     * Interface for environment variable querying for repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryEnvironmentVariableInterface
    {
        public:
            ///\name Environment query functionality
            ///\{

            /**
             * Query an environment variable
             */
            virtual std::string get_environment_variable(
                    const std::tr1::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositoryEnvironmentVariableInterface();
    };

    /**
     * Interface for mirror querying for repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryMirrorsInterface
    {
        public:
            ///\name Iterate over our mirrors
            ///\{

            struct MirrorsConstIteratorTag;
            typedef WrappedForwardIterator<MirrorsConstIteratorTag,
                    const std::pair<const std::string, std::string> > MirrorsConstIterator;

            virtual MirrorsConstIterator begin_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual MirrorsConstIterator end_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Is the named item a mirror?
             */
            bool is_mirror(const std::string & s) const;

            ///\}

            virtual ~RepositoryMirrorsInterface();
    };

    /**
     * Interface for repositories that define virtuals.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryVirtualsInterface
    {
        public:
            ///\name Virtuals functionality
            ///\{

            /**
             * A collection of virtuals.
             */
            typedef Sequence<RepositoryVirtualsEntry> VirtualsSequence;

            /**
             * Fetch our virtual packages.
             */
            virtual std::tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositoryVirtualsInterface();
    };

    /**
     * Interface for repositories that can make virtuals on the fly.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryMakeVirtualsInterface
    {
        public:
            virtual ~RepositoryMakeVirtualsInterface();

            virtual const std::tr1::shared_ptr<const PackageID> make_virtual_package_id(
                    const QualifiedPackageName & virtual_name, const std::tr1::shared_ptr<const PackageID> & provider) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * Interface for repositories that provide packages.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryProvidesInterface
    {
        public:
            ///\name Provides functionality
            ///\{

            /**
             * A collection of provided packages.
             */
            typedef Sequence<RepositoryProvidesEntry> ProvidesSequence;

            /**
             * Fetch our provided packages.
             */
            virtual std::tr1::shared_ptr<const ProvidesSequence> provided_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositoryProvidesInterface();
    };

    /**
     * Interface for repositories that can be used as an install destination.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryDestinationInterface
    {
        public:
            ///\name Destination functions
            ///\{

            /**
             * Are we a suitable destination for the specified package?
             */
            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Are we to be included in the Environment::default_destinations list?
             */
            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * If true, pre and post install phases will be used when writing to this
             * destination.
             *
             * This should return true for 'real' filesystem destinations (whether or
             * not root is /, if root merges are supported), and false for intermediate
             * destinations such as binary repositories.
             */
            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Merge a package.
             */
            virtual void merge(const MergeParams &) = 0;

            ///\}

            virtual ~RepositoryDestinationInterface();
    };

    /**
     * Interface for handling ERepository specific functionality.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryEInterface
    {
        public:
            ///\name Information about a ERepository
            ///\{

            virtual std::string profile_variable(const std::string &) const = 0;
            virtual std::string accept_keywords_variable() const = 0;
            virtual std::string arch_variable() const = 0;

            ///\}

            ///\name Profile setting and querying functions
            ///\{

            typedef RepositoryEInterfaceProfilesDescLine ProfilesDescLine;

            struct ProfilesConstIteratorTag;
            typedef WrappedForwardIterator<ProfilesConstIteratorTag, const ProfilesDescLine> ProfilesConstIterator;
            virtual ProfilesConstIterator begin_profiles() const = 0;
            virtual ProfilesConstIterator end_profiles() const = 0;

            virtual ProfilesConstIterator find_profile(const FSEntry & location) const = 0;
            virtual void set_profile(const ProfilesConstIterator & iter) = 0;
            virtual void set_profile_by_arch(const UseFlagName &) = 0;

            ///\}

            virtual ~RepositoryEInterface();
    };

    /**
     * Interface for handling QA tasks.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryQAInterface
    {
        public:
            /**
             * Perform QA checks on the repository.
             */
            virtual void check_qa(
                    QAReporter &,
                    const QACheckProperties &,
                    const QACheckProperties &,
                    const QAMessageLevel,
                    const FSEntry &
                    ) const = 0;

            ///\name Basic operations
            ///\{

            virtual ~RepositoryQAInterface();

            ///\}
    };

    /**
     * Interface for making and verifying Manifest2-style manifests
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryManifestInterface
    {
        public:
            /**
             * Makes the Manifest for a given package. Requires that all
             * the needed DIST files, etc, have already been fetched.
             */
            virtual void make_manifest(const QualifiedPackageName &) = 0;

            ///\name Basic operations
            ///\{

            virtual ~RepositoryManifestInterface();

            ///\}
    };

    /**
     * Interface for handling hooks.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryHookInterface
    {
        public:
            /**
             * Perform a hook.
             */
            virtual HookResult perform_hook(const Hook & hook) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\name Basic operations
            ///\{

            virtual ~RepositoryHookInterface();

            ///\}
    };
}

#endif
