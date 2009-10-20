/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/spec_tree-fwd.hh>
#include <paludis/name.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/merger-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <string>
#include <tr1/functional>

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
        struct image_dir;
        struct make_virtuals_interface;
        struct manifest_interface;
        struct merged_entries;
        struct mirrors_interface;
        struct options;
        struct output_manager;
        struct package_id;
        struct path;
        struct perform_uninstall;
        struct profile;
        struct provided_by;
        struct provided_by_spec;
        struct provides_interface;
        struct status;
        struct syncable_interface;
        struct used_this_for_config_protect;
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
        NamedValue<n::make_virtuals_interface, RepositoryMakeVirtualsInterface *> make_virtuals_interface;
        NamedValue<n::manifest_interface, RepositoryManifestInterface *> manifest_interface;
        NamedValue<n::mirrors_interface, RepositoryMirrorsInterface *> mirrors_interface;
        NamedValue<n::provides_interface, RepositoryProvidesInterface *> provides_interface;
        NamedValue<n::syncable_interface, RepositorySyncableInterface *> syncable_interface;
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
        NamedValue<n::profile, std::tr1::shared_ptr<const RepositoryEInterfaceProfilesDescLineProfile> > profile;
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

        /**
         * We record things we merged here.
         * \since 0.41
         */
        NamedValue<n::merged_entries, std::tr1::shared_ptr<FSEntrySet> > merged_entries;

        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::output_manager, std::tr1::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::tr1::shared_ptr<const PackageID> > package_id;

        /**
         * Some merges need to do an uninstall mid-way through the merge process.
         *
         * \see InstallActionOptions::perform_uninstall
         * \since 0.36
         */
        NamedValue<n::perform_uninstall, std::tr1::function<void (
                const std::tr1::shared_ptr<const PackageID> &,
                const UninstallActionOptions &)> > perform_uninstall;

        NamedValue<n::used_this_for_config_protect, std::tr1::function<void (const std::string &)> > used_this_for_config_protect;
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

            virtual ~NoSuchSetError() throw ();

            ///\}

            /**
             * Name of the set.
             */
            const std::string name() const;
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

            virtual ~RecursivelyDefinedSetError() throw ();

            ///\}

            /**
             * Name of the set.
             */
            const std::string name() const;
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

            /**
             * Purge any invalid on-disk cache entries.
             */
            virtual void purge_invalid_cache() const;

            /**
             * Perform a hook.
             *
             * \since 0.40 (previously in an interface)
             */
            virtual HookResult perform_hook(const Hook & hook)
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;


            ///\}

            ///\name Set methods
            ///\{

            /**
             * Call Environment::add_set for every set we define.
             *
             * Environment will call this method at most once, so no cache or check for
             * repeats is required. Nothing else should call this method.
             *
             * \since 0.40
             */
            virtual void populate_sets() const = 0;

            ///\}
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
            virtual bool sync(const std::tr1::shared_ptr<OutputManager> &) const = 0;

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
            virtual void set_profile_by_arch(const std::string &) = 0;

            ///\}

            virtual ~RepositoryEInterface();
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
}

#endif
