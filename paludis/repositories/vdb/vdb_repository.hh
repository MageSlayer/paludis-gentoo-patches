/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declarations for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

namespace paludis
{

#include <paludis/repositories/vdb/vdb_repository-sr.hh>

    /**
     * A VDBRepository represents the /var/db/pkg database used for
     * installed packages.
     *
     * It has a stupid name because Portage called it that.
     *
     * \ingroup grpvdbrepository
     */
    class VDBRepository :
        public Repository,
        public RepositoryInstalledInterface,
        public RepositoryUseInterface,
        public RepositoryUninstallableInterface,
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryProvidesInterface,
        public PrivateImplementationPattern<VDBRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual Contents::ConstPointer do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual UseFlagNameCollection::ConstPointer do_arch_flags() const;

            virtual bool do_is_expand_flag(const UseFlagName &) const;
            virtual bool do_is_expand_hidden_flag(const UseFlagName &) const;
            virtual std::string::size_type do_expand_flag_delim_pos(const UseFlagName &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &, 
                    const InstallOptions &) const;

            virtual DepAtom::Pointer do_package_set(const std::string &, const PackageSetOptions & o) const;

        public:
            /**
             * Constructor.
             */
            VDBRepository(const VDBRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_vdb_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    AssociativeCollection<std::string, std::string>::ConstPointer m);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual void invalidate() const;

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const;

            virtual ProvidesCollection::ConstPointer provided_packages() const;

            virtual VersionMetadata::ConstPointer provided_package_version_metadata(
                    const RepositoryProvidesEntry &) const;

            typedef CountedPtr<VDBRepository, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const VDBRepository, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
     *
     * \ingroup grpvdbrepository
     * \ingroup grpexceptions
     */
    class VDBRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            VDBRepositoryConfigurationError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a key read fails.
     *
     * \ingroup grpvdbrepository
     * \ingroup grpexceptions
     */
    class VDBRepositoryKeyReadError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            VDBRepositoryKeyReadError(const std::string & msg) throw ();
    };

    /**
     * Register the VDB repository format.
     *
     * \ingroup grpvdbrepository
     */
    static const RepositoryMaker::RegisterMaker register_vdb_repository(
            "vdb", &VDBRepository::make_vdb_repository);
}

#endif
