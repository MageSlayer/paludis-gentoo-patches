/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_INSTALLED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_INSTALLED_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declarations for CRANInstalledRepository.
 *
 * \ingroup grpcraninstrepository
 */


namespace paludis
{

#include <paludis/repositories/cran/cran_installed_repository-sr.hh>

    /**
     * A CRANInstalledRepository represents the database used for
     * installed CRAN packages.
     *
     * \ingroup grpcraninstrepository
     */
    class PALUDIS_VISIBLE CRANInstalledRepository :
        public Repository,
        public RepositoryInstalledInterface,
        public RepositoryUninstallableInterface,
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryDestinationInterface,
        public PrivateImplementationPattern<CRANInstalledRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual Contents::ConstPointer do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual time_t do_installed_time(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual DepAtom::Pointer do_package_set(const SetName &) const;

            virtual SetsCollection::ConstPointer sets_list() const;

        public:
            /**
             * Constructor.
             */
            CRANInstalledRepository(const CRANInstalledRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static CountedPtr<Repository> make_cran_installed_repository(
                    Environment * const env,
                    AssociativeCollection<std::string, std::string>::ConstPointer m);

            /**
             * Destructor.
             */
            ~CRANInstalledRepository();

            virtual void invalidate();

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual bool is_suitable_destination_for(const PackageDatabaseEntry &) const;
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
     *
     * \ingroup grpcraninstrepository
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE CRANInstalledRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            CRANInstalledRepositoryConfigurationError(const std::string & msg) throw ();
    };
}

#endif
