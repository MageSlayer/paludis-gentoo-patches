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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_HH 1

#include <paludis/action-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/repository_info-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/name.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/qa-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/version_spec.hh>
#include <string>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

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

#include <paludis/repository-sr.hh>

    /**
     * A Repository provides a representation of a physical repository to a
     * PackageDatabase.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Repository :
        private InstantiationPolicy<Repository, instantiation_method::NonCopyableTag>,
        public RepositoryCapabilities
    {
        private:
            const RepositoryName _name;
            std::string _format;

        protected:
            ///\name Implementation data
            ///\{

            /**
             * Our information.
             */
            mutable tr1::shared_ptr<RepositoryInfo> _info;

            ///\}

            ///\name Basic operations
            ///\{

            Repository(const RepositoryName &, const RepositoryCapabilities &,
                    const std::string & our_format);

            ///\}

            /**
             * \name Implementations: navigation functions
             *
             * These are implemented in subclasses to handle navigation queries.
             */
            ///\{

            /**
             * Override in descendents: fetch package IDs.
             */
            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch package names.
             */
            virtual tr1::shared_ptr<const QualifiedPackageNameSet> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch category names.
             */
            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents if a fast implementation is available: fetch category names
             * that contain a particular package.
             */
            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents: check for a package.
             */
            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: check for a category.
             */
            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: might some of our IDs support a
             * particular action?
             */
            virtual bool do_some_ids_might_support_action(const SupportsActionTestBase &) const = 0;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Repository();

            ///\}

            ///\name Repository information
            ///\{

            /**
             * Fetch information about the repository.
             */
            virtual tr1::shared_ptr<const RepositoryInfo> info(bool verbose) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return our name.
             */
            const RepositoryName & name() const PALUDIS_ATTRIBUTE((nothrow))
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return our format.
             */
            std::string format() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Are we allowed to be favourite repository?
             */
            virtual bool can_be_favourite_repository() const;

            ///\}

            ///\name Repository content queries
            ///\{

            /**
             * Do we have a category with the given name?
             */
            bool has_category_named(const CategoryNamePart & c) const;

            /**
             * Do we have a package in the given category with the given name?
             */
            bool has_package_named(const QualifiedPackageName & q) const;

            /**
             * Fetch our category names.
             */
            tr1::shared_ptr<const CategoryNamePartSet> category_names() const;

            /**
             * Fetch categories that contain a named package.
             */
            tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart & p) const;

            /**
             * Fetch our package names.
             */
            tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart & c) const;

            /**
             * Fetch our IDs.
             */
            tr1::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p) const;

            /**
             * Might some of our IDs support a particular action?
             *
             * Used to optimise PackageDatabase::query. If a repository doesn't
             * support, say, InstallAction, a query can skip searching it
             * entirely when looking for installable packages.
             */
            bool some_ids_might_support_action(const SupportsActionTestBase &) const;

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
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: get use.
             */
            virtual UseFlagState do_query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: get use mask.
             */
            virtual bool do_query_use_mask(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: get use force.
             */
            virtual bool do_query_use_force(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch all arch flags.
             */
            virtual tr1::shared_ptr<const UseFlagNameSet> do_arch_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch all use expand flags.
             */
            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch all use expand hidden prefixes.
             */
            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_hidden_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: fetch all use expand prefixes.
             */
            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Override in descendents: describe a use flag.
             */
            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

        public:
            ///\name USE queries
            ///\{

            /**
             * Query the state of the specified use flag.
             */
            UseFlagState query_use(const UseFlagName & u, const PackageID &) const;

            /**
             * Query whether the specified use flag is masked.
             */
            bool query_use_mask(const UseFlagName & u, const PackageID & pde) const;

            /**
             * Query whether the specified use flag is forced.
             */
            bool query_use_force(const UseFlagName & u, const PackageID & pde) const;

            /**
             * Fetch all arch flags.
             */
            tr1::shared_ptr<const UseFlagNameSet> arch_flags() const;

            /**
             * Fetch all expand flags.
             */
            tr1::shared_ptr<const UseFlagNameSet> use_expand_flags() const;

            /**
             * Fetch all expand hidden flags.
             */
            tr1::shared_ptr<const UseFlagNameSet> use_expand_hidden_prefixes() const;

            /**
             * Fetch all use expand prefixes.
             */
            tr1::shared_ptr<const UseFlagNameSet> use_expand_prefixes() const;

            /**
             * Describe a use flag.
             */
            std::string describe_use_flag(const UseFlagName & n,
                    const PackageID & pkg) const;

            ///\}

            virtual ~RepositoryUseInterface();
    };

    /**
     * Interface for handling actions for installed repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryInstalledInterface
    {
        public:
            ///\name Installed content queries
            ///\{

            /**
             * What is our filesystem root?
             */
            virtual FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

            virtual ~RepositoryInstalledInterface();
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
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: package list.
             */
            virtual tr1::shared_ptr<SetSpecTree::ConstItem> do_package_set(const SetName & id) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

        public:
            ///\name Set queries
            ///\{

            /**
             * Fetch a package set.
             */
            tr1::shared_ptr<SetSpecTree::ConstItem> package_set(const SetName & s) const
            {
                return do_package_set(s);
            }

            /**
             * Gives a list of the names of all the sets provided by this repo.
             */
            virtual tr1::shared_ptr<const SetNameSet> sets_list() const
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
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: sync, if needed (true) or do nothing (false).
             */
            virtual bool do_sync() const = 0;

            ///\}

        public:
            ///\name Sync functions
            ///\{

            /**
             * Sync, if necessary.
             *
             * \return True if we synced successfully, false if we skipped sync.
             */
            bool sync() const;

            ///\}

            virtual ~RepositorySyncableInterface();
    };

    /**
     * Interface for world handling for repositories.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryWorldInterface
    {
        public:
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

            virtual ~RepositoryWorldInterface();
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
                    const tr1::shared_ptr<const PackageID> & for_package,
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

            typedef libwrapiter::ForwardIterator<RepositoryMirrorsInterface,
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
            virtual tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
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

            virtual const tr1::shared_ptr<const PackageID> make_virtual_package_id(
                    const QualifiedPackageName & virtual_name, const tr1::shared_ptr<const PackageID> & provider) const
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
            virtual tr1::shared_ptr<const ProvidesSequence> provided_packages() const
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
            virtual void merge(const MergeOptions &) = 0;

            ///\}

            virtual ~RepositoryDestinationInterface();
    };

    /**
     * Interface for handling actions relating to licenses.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryLicensesInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: do the actual check,
             */
            virtual tr1::shared_ptr<FSEntry>
            do_license_exists(const std::string & license) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}

        public:
            ///\name License related queries
            ///\{

            /**
             * Check if a license exists
             */
            tr1::shared_ptr<FSEntry>
            license_exists(const std::string & license) const;

            ///\}

            virtual ~RepositoryLicensesInterface();
    };

    class ERepositoryParams;

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

            virtual const ERepositoryParams & params() const = 0;

            ///\}

            ///\name Profile setting and querying functions
            ///\{

            typedef RepositoryEInterfaceProfilesDescLine ProfilesDescLine;

            typedef libwrapiter::ForwardIterator<RepositoryEInterface,
                    const ProfilesDescLine> ProfilesConstIterator;
            virtual ProfilesConstIterator begin_profiles() const = 0;
            virtual ProfilesConstIterator end_profiles() const = 0;

            virtual ProfilesConstIterator find_profile(const FSEntry & location) const = 0;
            virtual void set_profile(const ProfilesConstIterator & iter) = 0;
            virtual void set_profile_by_arch(const UseFlagName &) = 0;

            ///\}

            ///\name Layout helpers
            ///\{

            virtual FSEntry info_variables_file(const FSEntry &) const = 0;

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
