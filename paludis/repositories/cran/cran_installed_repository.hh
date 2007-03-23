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
        public RepositoryContentsInterface,
        public RepositoryUninstallableInterface,
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryDestinationInterface,
        public PrivateImplementationPattern<CRANInstalledRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const Contents> do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual time_t do_installed_time(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const UninstallOptions &) const;

            virtual std::tr1::shared_ptr<DepSpec> do_package_set(const SetName &) const;

            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const;

            virtual void add_string_to_world(const std::string &) const;
            virtual void remove_string_from_world(const std::string &) const;

        public:
            /**
             * Constructor.
             */
            CRANInstalledRepository(const CRANInstalledRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static std::tr1::shared_ptr<Repository> make_cran_installed_repository(
                    Environment * const env,
                    std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~CRANInstalledRepository();

            virtual void invalidate();

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const SetName &) const;

            virtual bool is_suitable_destination_for(const PackageDatabaseEntry &) const;

            virtual bool is_default_destination() const;

            virtual bool want_pre_post_phases() const;

            virtual FSEntry root() const;

            virtual void merge(const MergeOptions &);
    };

    /**
     * Thrown if invalid parameters are provided for
     * CRANInstalledRepositoryConfigurationError::make_cran_installed_repository.
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
