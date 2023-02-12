/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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
#include <paludis/partitioning-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/merger-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <string>
#include <functional>

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
        typedef Name<struct name_arch> arch;
        typedef Name<struct name_build_start_time> build_start_time;
        typedef Name<struct name_check> check;
        typedef Name<struct name_destination_interface> destination_interface;
        typedef Name<struct name_environment_file> environment_file;
        typedef Name<struct name_environment_variable_interface> environment_variable_interface;
        typedef Name<struct name_image_dir> image_dir;
        typedef Name<struct name_is_volatile> is_volatile;
        typedef Name<struct name_manifest_interface> manifest_interface;
        typedef Name<struct name_merged_entries> merged_entries;
        typedef Name<struct name_options> options;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_parts> parts;
        typedef Name<struct name_path> path;
        typedef Name<struct name_perform_uninstall> perform_uninstall;
        typedef Name<struct name_permit_destination> permit_destination;
        typedef Name<struct name_profile> profile;
        typedef Name<struct name_replacing> replacing;
        typedef Name<struct name_status> status;
        typedef Name<struct name_used_this_for_config_protect> used_this_for_config_protect;
        typedef Name<struct name_want_phase> want_phase;
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
        NamedValue<n::environment_variable_interface, RepositoryEnvironmentVariableInterface *> environment_variable_interface;
        NamedValue<n::manifest_interface, RepositoryManifestInterface *> manifest_interface;
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
        /**
         * The start of the build time (for binaries, should really be when the
         * binary was originally built).
         *
         * \since 0.44
         */
        NamedValue<n::build_start_time, Timestamp> build_start_time;

        /**
         * Whether to check or perform the merge.
         *
         * A check must be performed before a merge.
         *
         * \since 0.59
         */
        NamedValue<n::check, bool> check;

        NamedValue<n::environment_file, FSPath> environment_file;
        NamedValue<n::image_dir, FSPath> image_dir;

        /**
         * Whether or not a file is volatile.
         * \since 2.0.0
         */
        NamedValue<n::is_volatile, std::function<bool (const FSPath &)> > is_volatile;

        /**
         * We record things we merged here.
         * \since 0.41
         */
        NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;

        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;

        /**
         * Package partioning.
         * \since 1.1.0
         */
        NamedValue<n::parts, std::shared_ptr<Partitioning> > parts;

        /**
         * Some merges need to do an uninstall mid-way through the merge process.
         *
         * \see InstallActionOptions::perform_uninstall
         * \since 0.36
         */
        NamedValue<n::perform_uninstall, std::function<void (
                const std::shared_ptr<const PackageID> &,
                const UninstallActionOptions &)> > perform_uninstall;

        ///\since 0.66
        NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;

        /**
         * Someone needs to replace these (either the merge or the install).
         *
         * \since 0.57
         */
        NamedValue<n::replacing, std::shared_ptr<const PackageIDSequence> > replacing;

        NamedValue<n::used_this_for_config_protect, std::function<void (const std::string &)> > used_this_for_config_protect;

        /**
         * Sometimes merging runs phase functions, possibly via perform_uninstall.
         *
         * \since 0.77
         */
        NamedValue<n::want_phase, std::function<WantPhase (const std::string &)> > want_phase;
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

            NoSuchSetError(const std::string & name) noexcept;

            ~NoSuchSetError() override;

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

            RecursivelyDefinedSetError(const std::string & name) noexcept;

            ~RecursivelyDefinedSetError() override;

            ///\}

            /**
             * Name of the set.
             */
            const std::string name() const;
    };

    /**
     * A Repository provides a representation of a physical repository to an
     * Environment.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Repository :
        public RepositoryCapabilities,
        public MetadataKeyHolder
    {
        private:
            Pimp<Repository> _imp;

        protected:
            ///\name Basic operations
            ///\{

            Repository(const Environment * const, const RepositoryName &, const RepositoryCapabilities &);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            ~Repository() override;

            Repository(const Repository &) = delete;
            Repository & operator= (const Repository &) = delete;

            ///\}

            ///\name Repository information
            ///\{

            /**
             * Return our name.
             */
            const RepositoryName name() const noexcept PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Specific metadata keys
            ///\{

            virtual const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const = 0;

            /**
             * The format_key, if non-zero, holds our repository's format. Repository
             * implementations should not return zero here, but clients should still
             * check.
             */
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const = 0;

            /**
             * The location_key, if non-zero, holds the file or directory containing
             * our repository's data, the format of which depends on the value of
             * format_key.
             */
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const = 0;

            /**
             * The installed_root_key, if non-zero, specifies that we contain installed
             * packages at the specified root.
             *
             * This key is currently used in various places to determine whether a repository is
             * an 'installed' repository or not.
             */
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const = 0;

            /**
             * The sync_host_key, if present, should have value containing
             * the host against which a sync will be performed for each suffix.
             *
             * This is used to avoid starting multiple parallel syncs against
             * the same host.
             *
             * \since 0.44
             * \since 0.55 is a Map<std::string, std::string>.
             */
            virtual const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const = 0;

            virtual const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const = 0;

            ///\}

            ///\name Repository content queries
            ///\{

            /**
             * Do we have a category with the given name?
             *
             * \since 0.59 takes repository_content_may_excludes
             */
            virtual bool has_category_named(const CategoryNamePart & c,
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const = 0;

            /**
             * Do we have a package in the given category with the given name?
             */
            virtual bool has_package_named(const QualifiedPackageName & q,
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const = 0;

            /**
             * Fetch our category names.
             */
            virtual std::shared_ptr<const CategoryNamePartSet> category_names(
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const = 0;

            /**
             * Fetch unimportant categories.
             */
            virtual std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const;

            /**
             * Are we unimportant?
             *
             * In disambiguation, anything gets preferred over packages from unimportant repositories.
             *
             * \since 0.46
             */
            virtual const bool is_unimportant() const = 0;

            /**
             * Fetch categories that contain a named package.
             */
            virtual std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart & p,
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const;

            /**
             * Fetch our package names.
             */
            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart & c,
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const = 0;

            /**
             * Fetch our IDs.
             */
            virtual std::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p,
                    const RepositoryContentMayExcludes & repository_content_may_excludes) const = 0;

            /**
             * Might some of our IDs support a particular action?
             *
             * Used to optimise various Generator and Filter queries. If a
             * repository doesn't support, say, InstallAction, a query can skip
             * searching it entirely when looking for installable packages.
             */
            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const = 0;

            /**
             * Might some of our IDs be not masked?
             *
             * Used to optimise various Generator and Filter queries.
             *
             * \since 0.49
             */
            virtual bool some_ids_might_not_be_masked() const = 0;

            /**
             * Possibly expand a licence.
             *
             * May return a null pointer, if we don't define any licence
             * groups, or if the thing being passed in doesn't look like a
             * licence group.
             *
             * This should not be recursive.
             *
             * Callers should Environment::expand_licence, not this method.
             *
             * \since 0.66
             */
            virtual const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                    const std::string &) const = 0;

            ///\}

            ///\name Repository behaviour methods
            ///\{

            /**
             * Invalidate any in memory cache.
             */
            virtual void invalidate() = 0;

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
             * \since 0.53 takes optional_output_manager
             */
            virtual HookResult perform_hook(
                    const Hook & hook,
                    const std::shared_ptr<OutputManager> & optional_output_manager)
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Sync, if necessary.
             *
             * \return True if we synced successfully, false if we skipped sync.
             * \since 0.42 (previously in an interface)
             * \since 0.55 takes a source (may be empty)
             * \since 0.61 takes a revision (may be empty)
             */
            virtual bool sync(
                    const std::string & source,
                    const std::string & revision,
                    const std::shared_ptr<OutputManager> &) const = 0;

            /**
             * Allow the Repository to drop any memory-cached metadata and
             * PackageIDs it holds.
             *
             * \since 0.42
             */
            virtual void can_drop_in_memory_cache() const;

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
                    const std::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositoryEnvironmentVariableInterface();
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
             *
             * \since 0.58 takes id by shared_ptr
             */
            virtual bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
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

            virtual std::string split_debug_location() const = 0;

            ///\}

            virtual ~RepositoryDestinationInterface();
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
