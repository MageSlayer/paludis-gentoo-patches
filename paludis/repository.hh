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

#include <paludis/dep_atom.hh>
#include <paludis/name.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/version_metadata.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/contents.hh>

#include <string>
#include <map>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Declarations for the Repository class.
 *
 * \ingroup grprepository
 */

namespace paludis
{
    class Environment;
    class RepositoryNameCache;

    class Repository;
    class RepositoryInstallableInterface;
    class RepositoryInstalledInterface;
    class RepositoryMaskInterface;
    class RepositoryNewsInterface;
    class RepositorySetsInterface;
    class RepositorySyncableInterface;
    class RepositoryUninstallableInterface;
    class RepositoryUseInterface;
    class RepositoryWorldInterface;
    class RepositoryEnvironmentVariableInterface;
    class RepositoryMirrorsInterface;
    class RepositoryProvidesInterface;
    class RepositoryVirtualsInterface;
    class RepositoryDestinationInterface;
    class RepositoryContentsInterface;
    class RepositoryConfigInterface;
    class RepositoryLicensesInterface;

    /**
     * What debug build option to use when installing a package.
     *
     * \ingroup grprepository
     */
    enum InstallDebugOption
    {
        ido_none,
        ido_split,
        ido_internal
    };

#include <paludis/repository-sr.hh>

    /**
     * A section of information about a Repository.
     *
     * \see RepositoryInfo
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryInfoSection :
        private PrivateImplementationPattern<RepositoryInfoSection>
    {
        public:
            ///\name Basic operations
            ///\{

            RepositoryInfoSection(const std::string & heading);

            ~RepositoryInfoSection();

            ///\}

            /// Our heading.
            std::string heading() const;

            ///\name Iterate over our key/values
            ///\{

            typedef libwrapiter::ForwardIterator<RepositoryInfoSection,
                    const std::pair<const std::string, std::string> > KeyValueIterator;

            KeyValueIterator begin_kvs() const;

            KeyValueIterator end_kvs() const;

            ///\}

            /// Add a key/value pair.
            RepositoryInfoSection & add_kv(const std::string &, const std::string &);
    };

    /**
     * Information about a Repository, for the end user.
     *
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryInfo :
        private PrivateImplementationPattern<RepositoryInfo>
    {
        public:
            ///\name Basic operations
            ///\{

            RepositoryInfo();
            ~RepositoryInfo();

            ///\}

            ///\name Iterator over our sections
            ///\{

            typedef libwrapiter::ForwardIterator<RepositoryInfo,
                    const std::tr1::shared_ptr<const RepositoryInfoSection> > SectionIterator;

            SectionIterator begin_sections() const;

            SectionIterator end_sections() const;

            ///\}

            /// Add a section.
            RepositoryInfo & add_section(std::tr1::shared_ptr<const RepositoryInfoSection>);
    };

    /**
     * A Repository provides a representation of a physical repository to a
     * PackageDatabase.
     *
     * We make pretty heavy use of the non-virtual interface idiom here. See
     * \ref EffCpp items 35 and 37. There's a lot of optional functionality
     * available. These are split off via get_interface() style functions,
     * which return 0 if an interface is not available.
     *
     * \ingroup grprepository
     * \nosubgrouping
     */
    class Repository :
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
            mutable std::tr1::shared_ptr<RepositoryInfo> _info;

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
             * Override in descendents: fetch the metadata.
             */
            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            /**
             * Override in descendents: check for a version.
             */
            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            /**
             * Override in descendents: fetch version specs.
             */
            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const = 0;

            /**
             * Override in descendents: fetch package names.
             */
            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const = 0;

            /**
             * Override in descendents: fetch category names.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const = 0;

            /**
             * Override in descendents if a fast implementation is available: fetch category names
             * that contain a particular package.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names_containing_package(
                    const PackageNamePart &) const;

            /**
             * Override in descendents: check for a package.
             */
            virtual bool do_has_package_named(const QualifiedPackageName &) const = 0;

            /**
             * Override in descendents: check for a category.
             */
            virtual bool do_has_category_named(const CategoryNamePart &) const = 0;

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
            virtual std::tr1::shared_ptr<const RepositoryInfo> info(bool verbose) const;

            /**
             * Return our name.
             */
            const RepositoryName & name() const PALUDIS_ATTRIBUTE((nothrow));

            /**
             * Return our format.
             */
            std::string format() const;

            /**
             * Are we allowed to be favourite repository?
             */
            virtual bool can_be_favourite_repository() const
            {
                return true;
            }

            ///\}

            ///\name Repository content queries
            ///\{

            /**
             * Do we have a category with the given name?
             */
            bool has_category_named(const CategoryNamePart & c) const
            {
                return do_has_category_named(c);
            }

            /**
             * Do we have a package in the given category with the given name?
             */
            bool has_package_named(const QualifiedPackageName & q) const
            {
                return do_has_package_named(q);
            }

            /**
             * Fetch our category names.
             */
            std::tr1::shared_ptr<const CategoryNamePartCollection> category_names() const
            {
                return do_category_names();
            }

            /**
             * Fetch categories that contain a named package.
             */
            std::tr1::shared_ptr<const CategoryNamePartCollection> category_names_containing_package(
                    const PackageNamePart & p) const
            {
                return do_category_names_containing_package(p);
            }

            /**
             * Fetch our package names.
             */
            std::tr1::shared_ptr<const QualifiedPackageNameCollection> package_names(
                    const CategoryNamePart & c) const
            {
                return do_package_names(c);
            }

            /**
             * Fetch our versions.
             */
            std::tr1::shared_ptr<const VersionSpecCollection> version_specs(
                    const QualifiedPackageName & p) const
            {
                return do_version_specs(p);
            }

            /**
             * Do we have a version spec?
             */
            bool has_version(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return do_has_version(q, v);
            }

            /**
             * Fetch metadata.
             */
            std::tr1::shared_ptr<const VersionMetadata> version_metadata(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_version_metadata(q, v);
            }

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

            ///\}

    };

    /**
     * Interface for handling masks for the Repository class.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryMaskInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: check for a mask.
             */
            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            /**
             * Override in descendents: check for a mask.
             */
            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            ///\}

        public:
            ///\name Mask queries
            ///\{

            /**
             * Query repository masks.
             */
            bool query_repository_masks(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return do_query_repository_masks(q, v);
            }

            /**
             * Query profile masks.
             */
            bool query_profile_masks(const QualifiedPackageName & q, const VersionSpec & v) const
            {
                return do_query_profile_masks(q, v);
            }

            ///\}

            virtual ~RepositoryMaskInterface();
    };

    /**
     * Interface for handling USE flags for the Repository class.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryUseInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: get use.
             */
            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry *) const = 0;

            /**
             * Override in descendents: get use mask.
             */
            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const = 0;

            /**
             * Override in descendents: get use force.
             */
            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const = 0;

            /**
             * Override in descendents: fetch all arch flags.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_arch_flags() const = 0;

            /**
             * Override in descendents: fetch all use expand flags.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_flags() const = 0;

            /**
             * Override in descendents: fetch all use expand hidden prefixes.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_hidden_prefixes() const = 0;

            /**
             * Override in descendents: fetch all use expand prefixes.
             */
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_prefixes() const = 0;

            /**
             * Override in descendents: describe a use flag.
             */
            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageDatabaseEntry * const) const = 0;

            ///\}

        public:
            ///\name USE queries
            ///\{

            /**
             * Query the state of the specified use flag.
             */
            UseFlagState query_use(const UseFlagName & u, const PackageDatabaseEntry *pde) const
            {
                if (do_query_use_mask(u, pde))
                    return use_disabled;
                else if (do_query_use_force(u, pde))
                    return use_enabled;
                else
                    return do_query_use(u, pde);
            }

            /**
             * Query whether the specified use flag is masked.
             */
            bool query_use_mask(const UseFlagName & u, const PackageDatabaseEntry *pde) const
            {
                return do_query_use_mask(u, pde);
            }

            /**
             * Query whether the specified use flag is forced.
             */
            bool query_use_force(const UseFlagName & u, const PackageDatabaseEntry *pde) const
            {
                return do_query_use_force(u, pde);
            }

            /**
             * Fetch all arch flags.
             */
            std::tr1::shared_ptr<const UseFlagNameCollection> arch_flags() const
            {
                return do_arch_flags();
            }

            /**
             * Fetch all expand flags.
             */
            std::tr1::shared_ptr<const UseFlagNameCollection> use_expand_flags() const
            {
                return do_use_expand_flags();
            }

            /**
             * Fetch all expand hidden flags.
             */
            std::tr1::shared_ptr<const UseFlagNameCollection> use_expand_hidden_prefixes() const
            {
                return do_use_expand_hidden_prefixes();
            }

            /**
             * Fetch all use expand prefixes.
             */
            std::tr1::shared_ptr<const UseFlagNameCollection> use_expand_prefixes() const
            {
                return do_use_expand_prefixes();
            }

            /**
             * Describe a use flag.
             */
            std::string describe_use_flag(const UseFlagName & n,
                    const PackageDatabaseEntry * const pkg) const
            {
                return do_describe_use_flag(n, pkg);
            }

            ///\}

            virtual ~RepositoryUseInterface();
    };

    /**
     * Interface for handling actions for installed repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryInstalledInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: when was a package installed.
             */
            virtual time_t do_installed_time(
                    const QualifiedPackageName &,
                    const VersionSpec &) const
            {
                return time_t(0);
            }

            ///\}

        public:
            ///\name Installed content queries
            ///\{

            /**
             * When was a package installed.
             *
             * Can return time_t(0) if the installed time is unknown.
             */
            time_t installed_time(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_installed_time(q, v);
            }

            /**
             * What is our filesystem root?
             */
            virtual FSEntry root() const = 0;

            ///\}

            virtual ~RepositoryInstalledInterface();
    };

    /**
     * Interface for handling installs for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryInstallableInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: install.
             */
            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const = 0;

            ///\}

        public:
            ///\name Installable functions
            ///\{

            /**
             * Install a package.
             */
            void install(const QualifiedPackageName & q, const VersionSpec & v, const InstallOptions & i) const
            {
                do_install(q, v, i);
            }

            ///\}

            virtual ~RepositoryInstallableInterface();
    };

    /**
     * Interface for handling uninstalls for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryUninstallableInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: uninstall.
             */
            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const UninstallOptions &) const = 0;

            ///\}

        public:
            ///\name Uninstall functions
            ///\{

            /**
             * Uninstall a package.
             */
            void uninstall(const QualifiedPackageName & q, const VersionSpec & v, const UninstallOptions & i) const
            {
                do_uninstall(q, v, i);
            }

            ///\}

            virtual ~RepositoryUninstallableInterface();
    };

    /**
     * Contains the names of all the sets provided by the repository.
     *
     * \ingroup grpnames
     */
    typedef SortedCollection<SetName> SetsCollection;

    /**
     * Interface for package sets for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositorySetsInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: package list.
             */
            virtual std::tr1::shared_ptr<DepAtom> do_package_set(const SetName & id) const = 0;

            ///\}

        public:
            ///\name Set queries
            ///\{

            /**
             * Fetch a package set.
             */
            std::tr1::shared_ptr<DepAtom> package_set(const SetName & s) const
            {
                return do_package_set(s);
            }

            /**
             * Gives a list of the names of all the sets provided by this repo.
             */
            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const = 0;

            ///\}

            virtual ~RepositorySetsInterface();
    };

    /**
     * Interface for syncing for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositorySyncableInterface
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
            bool sync() const
            {
                return do_sync();
            }

            ///\}

            virtual ~RepositorySyncableInterface();
    };

    /**
     * Interface for world handling for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryWorldInterface
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
     * Interface for news handling for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryNewsInterface
    {
        public:
            ///\name News functionality
            ///\{

            /**
             * Update our news.unread file.
             */
            virtual void update_news() const = 0;

            ///\}

            virtual ~RepositoryNewsInterface();
    };

    /**
     * Interface for environment variable querying for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryEnvironmentVariableInterface
    {
        public:
            ///\name Environment query functionality
            ///\{

            /**
             * Query an environment variable
             */
            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const = 0;

            ///\}

            virtual ~RepositoryEnvironmentVariableInterface();
    };

    /**
     * Interface for mirror querying for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryMirrorsInterface
    {
        public:
            ///\name Iterate over our mirrors
            ///\{

            typedef libwrapiter::ForwardIterator<RepositoryMirrorsInterface,
                    const std::pair<const std::string, std::string> > MirrorsIterator;

            virtual MirrorsIterator begin_mirrors(const std::string & s) const = 0;
            virtual MirrorsIterator end_mirrors(const std::string & s) const = 0;

            /**
             * Is the named item a mirror?
             */
            bool is_mirror(const std::string & s) const
            {
                return begin_mirrors(s) != end_mirrors(s);
            }

            ///\}

            virtual ~RepositoryMirrorsInterface();
    };

    /**
     * Interface for repositories that define virtuals.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryVirtualsInterface
    {
        public:
            ///\name Virtuals functionality
            ///\{

            /**
             * A collection of virtuals.
             *
             * \ingroup grprepository
             */
            typedef SortedCollection<RepositoryVirtualsEntry> VirtualsCollection;

            /**
             * Fetch our virtual packages.
             */
            virtual std::tr1::shared_ptr<const VirtualsCollection> virtual_packages() const = 0;

            /**
             * Fetch version metadata for a virtual
             */
            virtual std::tr1::shared_ptr<const VersionMetadata> virtual_package_version_metadata(
                    const RepositoryVirtualsEntry &, const VersionSpec & v) const = 0;

            ///\}

            virtual ~RepositoryVirtualsInterface();
    };

    /**
     * Interface for repositories that provide packages.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryProvidesInterface
    {
        public:
            ///\name Provides functionality
            ///\{

            /**
             * A collection of provided packages.
             *
             * \ingroup grprepository
             */
            typedef SortedCollection<RepositoryProvidesEntry> ProvidesCollection;

            /**
             * Fetch our provided packages.
             */
            virtual std::tr1::shared_ptr<const ProvidesCollection> provided_packages() const = 0;

            /**
             * Fetch version metadata for a provided package.
             */
            virtual std::tr1::shared_ptr<const VersionMetadata> provided_package_version_metadata(
                    const RepositoryProvidesEntry &) const = 0;

            ///\}

            virtual ~RepositoryProvidesInterface();
    };

    /**
     * Interface for repositories that can be used as an install destination.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryDestinationInterface
    {
        public:
            ///\name Destination functions
            ///\{

            /**
             * Are we a suitable destination for the specified package?
             */
            virtual bool is_suitable_destination_for(const PackageDatabaseEntry &) const = 0;

            /**
             * Are we to be included in the Environment::default_destinations list?
             */
            virtual bool is_default_destination() const = 0;

            /**
             * If true, pre and post install phases will be used when writing to this
             * destination.
             *
             * This should return true for 'real' filesystem destinations (whether or
             * not root is /, if root merges are supported), and false for intermediate
             * destinations such as binary repositories.
             */
            virtual bool want_pre_post_phases() const = 0;

            /**
             * Merge a package.
             */
            virtual void merge(const MergeOptions &) = 0;

            ///\}

            virtual ~RepositoryDestinationInterface();
    };

    /**
     * Interface for handling actions for repositories supporting contents queries.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryContentsInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: fetch the contents.
             */
            virtual std::tr1::shared_ptr<const Contents> do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            ///\}

        public:
            ///\name Installed content queries
            ///\{

            /**
             * Fetch contents.
             */
            std::tr1::shared_ptr<const Contents> contents(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_contents(q, v);
            }

            ///\}

            virtual ~RepositoryContentsInterface();
    };

    /**
     * Interface for handling actions for repositories supporting package configuration.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryConfigInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: do the configuration.
             */
            virtual void do_config(
                    const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            ///\}

        public:
            ///\name Installed content queries
            ///\{

            /**
             * Fetch contents.
             */
            void config(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_config(q, v);
            }

            ///\}

            virtual ~RepositoryConfigInterface();
    };

    /**
     * Interface for handling actions relating to licenses.
     *
     * \see Repository
     * \ingroup grprepository
     * \nosubgrouping
     */
    class RepositoryLicensesInterface
    {
        protected:
            ///\name Implementation details
            ///\{

            /**
             * Override in descendents: do the actual check,
             */
            virtual std::tr1::shared_ptr<FSEntry>
            do_license_exists(const std::string & license) const = 0;

            ///\}

        public:
            ///\name License related queries
            ///\{

            /**
             * Check if a license exists
             */
            std::tr1::shared_ptr<FSEntry>
            license_exists(const std::string & license) const
            {
                return do_license_exists(license);
            }

            ///\}

            virtual ~RepositoryLicensesInterface();
    };

    /**
     * Thrown if a repository of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     * \nosubgrouping
     */
    class NoSuchRepositoryTypeError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchRepositoryTypeError(const std::string & format) throw ();
    };

    /**
     * Parent class for install, uninstall errors.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     * \nosubgrouping
     */
    class PackageActionError : public Exception
    {
        protected:
            /**
             * Constructor.
             */
            PackageActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if an install fails.
     *
     * \ingroup grprepository
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PackageInstallActionError : public PackageActionError
    {
        public:
            /**
             * Constructor.
             */
            PackageInstallActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a fetch fails.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
     * \nosubgrouping
     */
    class PackageFetchActionError : public PackageActionError
    {
        public:
            /**
             * Constructor.
             */
            PackageFetchActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if an uninstall fails.
     *
     * \ingroup grprepository
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PackageUninstallActionError : public PackageActionError
    {
        public:
            /**
             * Constructor.
             */
            PackageUninstallActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a configure fails.
     *
     * \ingroup grprepository
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PackageConfigActionError : public PackageActionError
    {
        public:
            /**
             * Constructor.
             */
            PackageConfigActionError(const std::string & msg) throw ();
    };

    /**
     * Thrown if an environment variable query fails.
     *
     * \ingroup grprepository
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class EnvironmentVariableActionError :
        public PackageActionError
    {
        public:
            /**
             * Constructor.
             */
            EnvironmentVariableActionError(const std::string & msg) throw ();
    };


    class PackageDatabase;
}

#endif
