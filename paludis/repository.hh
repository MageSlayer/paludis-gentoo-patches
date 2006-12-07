/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
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
        public InternalCounted<RepositoryInfoSection>,
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
        public InternalCounted<RepositoryInfo>,
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
                    const RepositoryInfoSection::ConstPointer> SectionIterator;

            SectionIterator begin_sections() const;

            SectionIterator end_sections() const;

            ///\}

            /// Add a section.
            RepositoryInfo & add_section(RepositoryInfoSection::ConstPointer);
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
        public InternalCounted<Repository>,
        public RepositoryCapabilities
    {
        private:
            const RepositoryName _name;

        protected:
            /**
             * Our information.
             */
            mutable RepositoryInfo::Pointer _info;

            /**
             * Constructor.
             */
            Repository(const RepositoryName &, const RepositoryCapabilities &);

            /**
             * \name Implementations: navigation functions
             *
             * These are implemented in subclasses to handle navigation queries.
             */
            ///\{

            /**
             * Override in descendents: fetch the metadata.
             */
            virtual VersionMetadata::ConstPointer do_version_metadata(
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
            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const = 0;

            /**
             * Override in descendents: fetch package names.
             */
            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const = 0;

            /**
             * Override in descendents: fetch category names.
             */
            virtual CategoryNamePartCollection::ConstPointer do_category_names() const = 0;

            /**
             * Override in descendents if a fast implementation is available: fetch category names
             * that contain a particular package.
             */
            virtual CategoryNamePartCollection::ConstPointer do_category_names_containing_package(
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

            /**
             * \name Implementations: misc files
             *
             * Various utility functions related to misc files.
             */
            ///\{

            /**
             * Override in descendents: is this a licence?
             */
            virtual bool do_is_licence(const std::string &) const = 0;

            ///\}

        public:
            virtual RepositoryInfo::ConstPointer info(bool verbose) const;

            /**
             * Destructor.
             */
            virtual ~Repository();

            /**
             * Return our name.
             */
            const RepositoryName & name() const PALUDIS_ATTRIBUTE((nothrow));

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
            CategoryNamePartCollection::ConstPointer category_names() const
            {
                return do_category_names();
            }

            /**
             * Fetch categories that contain a named package.
             */
            CategoryNamePartCollection::ConstPointer category_names_containing_package(
                    const PackageNamePart & p) const
            {
                return do_category_names_containing_package(p);
            }

            /**
             * Fetch our package names.
             */
            QualifiedPackageNameCollection::ConstPointer package_names(
                    const CategoryNamePart & c) const
            {
                return do_package_names(c);
            }

            /**
             * Fetch our versions.
             */
            VersionSpecCollection::ConstPointer version_specs(
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
            VersionMetadata::ConstPointer version_metadata(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_version_metadata(q, v);
            }

            /**
             * Query whether the specified item is a licence.
             */
            bool is_license(const std::string & u) const
            {
                return do_is_licence(u);
            }

            /**
             * Invalidate any in memory cache.
             */
            virtual void invalidate() const = 0;

            /**
             * Regenerate any on disk cache.
             */
            virtual void regenerate_cache() const;

            /**
             * Are we allowed to be favourite repository?
             */
            virtual bool can_be_favourite_repository() const
            {
                return true;
            }
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

        public:
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

            virtual ~RepositoryMaskInterface() { }
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
            virtual UseFlagNameCollection::ConstPointer do_arch_flags() const = 0;

            /**
             * Override in descendents: fetch all use expand flags.
             */
            virtual UseFlagNameCollection::ConstPointer do_use_expand_flags() const = 0;

            /**
             * Override in descendents: fetch all use expand hidden prefixes.
             */
            virtual UseFlagNameCollection::ConstPointer do_use_expand_hidden_prefixes() const = 0;

            /**
             * Override in descendents: fetch all use expand prefixes.
             */
            virtual UseFlagNameCollection::ConstPointer do_use_expand_prefixes() const = 0;

        public:
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
            UseFlagNameCollection::ConstPointer arch_flags() const
            {
                return do_arch_flags();
            }

            /**
             * Fetch all expand flags.
             */
            UseFlagNameCollection::ConstPointer use_expand_flags() const
            {
                return do_use_expand_flags();
            }

            /**
             * Fetch all expand hidden flags.
             */
            UseFlagNameCollection::ConstPointer use_expand_hidden_prefixes() const
            {
                return do_use_expand_hidden_prefixes();
            }

            /**
             * Fetch all use expand prefixes.
             */
            UseFlagNameCollection::ConstPointer use_expand_prefixes() const
            {
                return do_use_expand_prefixes();
            }

            virtual ~RepositoryUseInterface() { }
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
            /**
             * Override in descendents: fetch the contents.
             */
            virtual Contents::ConstPointer do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

            /**
             * Override in descendents: when was a package installed.
             */
            virtual time_t do_installed_time(
                    const QualifiedPackageName &,
                    const VersionSpec &) const
            {
                return time_t(0);
            }

        public:
            /**
             * Fetch contents.
             */
            Contents::ConstPointer contents(
                    const QualifiedPackageName & q,
                    const VersionSpec & v) const
            {
                return do_contents(q, v);
            }

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

            virtual ~RepositoryInstalledInterface() { }
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
            /**
             * Override in descendents: install.
             */
            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const = 0;

        public:
            /**
             * Install a package.
             */
            void install(const QualifiedPackageName & q, const VersionSpec & v, const InstallOptions & i) const
            {
                do_install(q, v, i);
            }

            virtual ~RepositoryInstallableInterface() { }
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
            /**
             * Override in descendents: uninstall.
             */
            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const = 0;

        public:
            /**
             * Uninstall a package.
             */
            void uninstall(const QualifiedPackageName & q, const VersionSpec & v, const InstallOptions & i) const
            {
                do_uninstall(q, v, i);
            }

            virtual ~RepositoryUninstallableInterface() { }
    };

    /**
     * Contains the names of all the sets provided by the repository.
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
            /**
             * Override in descendents: package list.
             */
            virtual DepAtom::Pointer do_package_set(const SetName & id) const = 0;

        public:
            /**
             * Fetch a package set.
             */
            DepAtom::Pointer package_set(const SetName & s) const
            {
                return do_package_set(s);
            }

            /**
             * Gives a list of the names of all the sets provided by this repo.
             */
            virtual SetsCollection::ConstPointer sets_list() const = 0;

            virtual ~RepositorySetsInterface() { }
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
            /**
             * Override in descendents: sync, if needed (true) or do nothing (false).
             */
            virtual bool do_sync() const = 0;

        public:
            /**
             * Sync, if necessary.
             *
             * \return True if we synced successfully, false if we skipped sync.
             */
            bool sync() const
            {
                return do_sync();
            }

            virtual ~RepositorySyncableInterface() { }
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
            /**
             * Add this package to world.
             */
            virtual void add_to_world(const QualifiedPackageName &) const = 0;

            /**
             * Remove this package from world, if it is present.
             */
            virtual void remove_from_world(const QualifiedPackageName &) const = 0;

            virtual ~RepositoryWorldInterface() { }
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
            /**
             * Update our news.unread file.
             */
            virtual void update_news() const
            {
            }

            virtual ~RepositoryNewsInterface() { }
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
            /**
             * Query an environment variable
             */
            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const = 0;

            virtual ~RepositoryEnvironmentVariableInterface() { }
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

            virtual ~RepositoryMirrorsInterface() { }
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
            typedef SortedCollection<RepositoryVirtualsEntry> VirtualsCollection;

            virtual VirtualsCollection::ConstPointer virtual_packages() const = 0;

            virtual VersionMetadata::ConstPointer virtual_package_version_metadata(
                    const RepositoryVirtualsEntry &, const VersionSpec & v) const = 0;

            virtual ~RepositoryVirtualsInterface() { }
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
            typedef SortedCollection<RepositoryProvidesEntry> ProvidesCollection;

            virtual ProvidesCollection::ConstPointer provided_packages() const = 0;

            virtual VersionMetadata::ConstPointer provided_package_version_metadata(
                    const RepositoryProvidesEntry &) const = 0;

            virtual ~RepositoryProvidesInterface() { }
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

    /**
     * Virtual constructor for repositories.
     *
     * \ingroup grprepository
     */
    typedef VirtualConstructor<std::string,
            Repository::Pointer (*) (const Environment * const, const PackageDatabase * const,
                    AssociativeCollection<std::string, std::string>::ConstPointer),
            virtual_constructor_not_found::ThrowException<NoSuchRepositoryTypeError> > RepositoryMaker;
}

#endif
