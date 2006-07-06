/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/virtual_constructor.hh>
#include <paludis/version_metadata.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/contents.hh>

#include <map>
#include <string>

/** \file
 * Declarations for the Repository class.
 *
 * \ingroup grprepository
 */

namespace paludis
{
    class Environment;

    /**
     * Keys for InstallOptions.
     *
     * \see InstallOptions
     * \ingroup grprepository
     */
    enum InstallOptionsKeys
    {
        io_noconfigprotect,     ///< Disable config protection
        io_fetchonly,           ///< Fetch only
        last_io                 ///< Number of entries
    };

    /**
     * Tag for InstallOptions.
     *
     * \see InstallOptions
     * \ingroup grprepository
     */
    struct InstallOptionsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<InstallOptionsKeys, last_io>,
        SmartRecordKey<io_noconfigprotect, bool>,
        SmartRecordKey<io_fetchonly, bool>
    {
    };

    /**
     * Defines various options for package installation.
     *
     * \ingroup grprepository
     */
    typedef MakeSmartRecord<InstallOptionsTag>::Type InstallOptions;

    /**
     * Keys for PackageSetOptions.
     *
     * \see PackageSetOptions
     * \ingroup grprepository
     */
    enum PackageSetOptionsKeys
    {
        pso_list_affected_only,  ///< Only list affected packages in the set
        last_pso
    };

    /**
     * Tag for PackageSetOptions.
     *
     * \see PackageSetOptions
     * \ingroup grprepository
     */
    struct PackageSetOptionsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<PackageSetOptionsKeys, last_pso>,
        SmartRecordKey<pso_list_affected_only, bool>
    {
    };

    /**
     * Defines various options for package installation.
     *
     * \ingroup grprepository
     */
    typedef MakeSmartRecord<PackageSetOptionsTag>::Type PackageSetOptions;

    /**
     * Capability keys for a repository.
     */
    enum RepositoryCapabilitiesKeys
    {
        repo_installable,
        repo_installed,
        repo_mask,
        repo_news,
        repo_sets,
        repo_syncable,
        repo_uninstallable,
        repo_use,
        repo_world,
        repo_environment_variable,
        last_repo
    };

    /**
     * A section of information about a Repository.
     *
     * \see RepositoryInfo
     * \ingroup grprepository
     */
    class RepositoryInfoSection
    {
        private:
            std::string _heading;
            std::map<std::string, std::string> _kvs;

        public:
            RepositoryInfoSection(const std::string & heading);

            std::string heading() const
            {
                return _heading;
            }

            typedef std::map<std::string, std::string>::const_iterator KeyValueIterator;

            KeyValueIterator begin_kvs() const
            {
                return _kvs.begin();
            }

            KeyValueIterator end_kvs() const
            {
                return _kvs.end();
            }

            RepositoryInfoSection & add_kv(const std::string &, const std::string &);
    };

    /**
     * Information about a Repository, for the end user.
     *
     * \ingroup grprepository
     */
    class RepositoryInfo :
        public InternalCounted<RepositoryInfo>
    {
        private:
            std::list<RepositoryInfoSection> _sections;

        public:
            typedef std::list<RepositoryInfoSection>::const_iterator SectionIterator;

            SectionIterator begin_sections() const
            {
                return _sections.begin();
            }

            SectionIterator end_sections() const
            {
                return _sections.end();
            }

            RepositoryInfo & add_section(const RepositoryInfoSection &);
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
     */
    class Repository :
        private InstantiationPolicy<Repository, instantiation_method::NonCopyableTag>,
        public InternalCounted<Repository>
    {
        public:
            class InstallableInterface;
            class InstalledInterface;
            class MaskInterface;
            class NewsInterface;
            class SetsInterface;
            class SyncableInterface;
            class UninstallableInterface;
            class UseInterface;
            class WorldInterface;
            class EnvironmentVariableInterface;

        protected:
            /**
             * Tag for RepositoryCapabilities.
             *
             * \see RepositoryCapabilities
             * \ingroup grprepository
             */
            struct RepositoryCapabilitiesTag :
                SmartRecordTag<comparison_mode::NoComparisonTag, void>,
                SmartRecordKeys<RepositoryCapabilitiesKeys, last_repo>,
                SmartRecordKey<repo_installable, InstallableInterface *>,
                SmartRecordKey<repo_installed, InstalledInterface *>,
                SmartRecordKey<repo_mask, MaskInterface *>,
                SmartRecordKey<repo_news, NewsInterface *>,
                SmartRecordKey<repo_sets, SetsInterface *>,
                SmartRecordKey<repo_syncable, SyncableInterface *>,
                SmartRecordKey<repo_uninstallable, UninstallableInterface *>,
                SmartRecordKey<repo_use, UseInterface *>,
                SmartRecordKey<repo_world, WorldInterface *>,
                SmartRecordKey<repo_environment_variable, EnvironmentVariableInterface *>
            {
            };

            /**
             * Holds pointers to upcast ourself to different capability interfaces. Each entry
             * is either a this pointer or a zero pointer.
             *
             * \see Repository
             * \ingroup grprepository
             */
            typedef MakeSmartRecord<RepositoryCapabilitiesTag>::Type RepositoryCapabilities;

        private:
            const RepositoryName _name;

            RepositoryCapabilities _caps;

        protected:
            /**
             * Our information.
             */
            mutable RepositoryInfo::Pointer _info;

            /**
             * Constructor.
             */
            Repository(const RepositoryName &, const RepositoryCapabilities &);

            ///\name Implementations: naviagation functions
            ///{

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
             * Override in descendents: check for a package.
             */
            virtual bool do_has_package_named(const QualifiedPackageName &) const = 0;

            /**
             * Override in descendents: check for a category.
             */
            virtual bool do_has_category_named(const CategoryNamePart &) const = 0;

            ///}

            ///\name Implementations: misc files
            ///{

            /**
             * Override in descendents: is this a licence?
             */
            virtual bool do_is_licence(const std::string &) const = 0;

            /**
             * Override in descendents: is this a mirror?
             */
            virtual bool do_is_mirror(const std::string &) const = 0;

            ///}

        public:
            virtual RepositoryInfo::ConstPointer info(bool verbose) const;

            ///\name Interface queries
            ///{

            /**
             * Fetch an interface.
             */
            template <RepositoryCapabilitiesKeys k_>
            typename RepositoryCapabilities::GetKeyType<k_>::Type
            get_interface()
            {
                return _caps.get<k_>();
            }

            /**
             * Fetch an interface, const.
             */
            template <RepositoryCapabilitiesKeys k_>
            const typename RepositoryCapabilities::GetKeyType<k_>::Type
            get_interface() const
            {
                return _caps.get<k_>();
            }

            ///}

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
             * Query whether the specified item is a mirror.
             */
            bool is_mirror(const std::string & u) const
            {
                return do_is_mirror(u);
            }

            /**
             * Invalidate any cache.
             */
            virtual void invalidate() const = 0;

            /**
             * Our provide map iterator type.
             */
            typedef std::map<QualifiedPackageName, QualifiedPackageName>::const_iterator ProvideMapIterator;

            /**
             * Start of our provide map.
             */
            virtual ProvideMapIterator begin_provide_map() const = 0;

            /**
             * Past the end of our provide map.
             */
            virtual ProvideMapIterator end_provide_map() const = 0;
    };

    /**
     * Interface for handling masks for the Repository class.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::MaskInterface
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

            virtual ~MaskInterface() { }
    };

    /**
     * Interface for handling USE flags for the Repository class.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::UseInterface
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
             * Override in descendents: is this an arch flag?
             */
            virtual bool do_is_arch_flag(const UseFlagName &) const = 0;

            /**
             * Override in descendents: is this an expand flag?
             */
            virtual bool do_is_expand_flag(const UseFlagName &) const = 0;

            /**
             * Override in descendents: is this an expand flag that should be
             * ignored?
             */
            virtual bool do_is_expand_hidden_flag(const UseFlagName &) const = 0;

            /**
             * Override in descendents: for a UseFlagName where is_expand_flag
             * is true, return the position of the delimiting underscore that
             * splits name and value.
             */
            virtual std::string::size_type do_expand_flag_delim_pos(const UseFlagName &) const = 0;

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
             * Query whether the specified use flag is an arch flag.
             */
            bool is_arch_flag(const UseFlagName & u) const
            {
                return do_is_arch_flag(u);
            }

            /**
             * Query whether the specified use flag is an expand flag.
             */
            bool is_expand_flag(const UseFlagName & u) const
            {
                return do_is_expand_flag(u);
            }

            /**
             * Query whether the specified use flag is an expand flag that
             * is ignored in visible output.
             */
            bool is_expand_hidden_flag(const UseFlagName & u) const
            {
                return do_is_expand_hidden_flag(u);
            }

            /**
             * Fetch the expand flag name for a given use flag where
             * is_expand_flag returns true.
             */
            UseFlagName expand_flag_name(const UseFlagName & u) const;

            /**
             * Fetch the expand flag value for a given use flag where
             * is_expand_flag returns true.
             */
            UseFlagName expand_flag_value(const UseFlagName & u) const
            {
                return UseFlagName(stringify(u).substr(do_expand_flag_delim_pos(u) + 1));
            }

            virtual ~UseInterface() { }
    };

    /**
     * Interface for handling actions for installed repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::InstalledInterface
    {
        protected:
            /**
             * Override in descendents: fetch the contents.
             */
            virtual Contents::ConstPointer do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const = 0;

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

            virtual ~InstalledInterface() { }
    };

    /**
     * Interface for handling installs for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::InstallableInterface
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

            virtual ~InstallableInterface() { }
    };

    /**
     * Interface for handling uninstalls for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::UninstallableInterface
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

            virtual ~UninstallableInterface() { }
    };

    /**
     * Interface for package sets for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::SetsInterface
    {
        protected:
            /**
             * Override in descendents: package list.
             */
            virtual DepAtom::Pointer do_package_set(const std::string & id, const PackageSetOptions & o) const = 0;

        public:
            /**
             * Fetch a package set.
             */
            virtual DepAtom::Pointer package_set(const std::string & s,
                    const PackageSetOptions & o = PackageSetOptions(false)) const
            {
                return do_package_set(s, o);
            }

            virtual ~SetsInterface() { }
    };

    /**
     * Interface for syncing for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::SyncableInterface
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

            virtual ~SyncableInterface() { }
    };

    /**
     * Interface for world handling for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::WorldInterface
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

            virtual ~WorldInterface() { }
    };

    /**
     * Interface for news handling for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::NewsInterface
    {
        public:
            /**
             * Update our news.unread file.
             */
            virtual void update_news() const
            {
            }

            virtual ~NewsInterface() { }
    };

    /**
     * Interface for environment variable querying for repositories.
     *
     * \see Repository
     * \ingroup grprepository
     */
    class Repository::EnvironmentVariableInterface
    {
        public:
            /**
             * Query an environment variable
             */
            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const = 0;

            virtual ~EnvironmentVariableInterface() { }
    };

    /**
     * Thrown if a repository of the specified type does not exist.
     *
     * \ingroup grpexceptions
     * \ingroup grprepository
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
                    const std::map<std::string, std::string> &),
            virtual_constructor_not_found::ThrowException<NoSuchRepositoryTypeError> > RepositoryMaker;
}

#endif
