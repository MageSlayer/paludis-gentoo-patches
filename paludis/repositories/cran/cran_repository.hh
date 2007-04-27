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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

/** \file
 * Declaration for the CRANRepository class.
 *
 * \ingroup grpcranrepository
 */

namespace paludis
{
    class PackageDatabase;

#include <paludis/repositories/cran/cran_repository-sr.hh>

    /**
     * A CRANRepository is a Repository that handles the layout used by
     * the GNU R project for the Comprehensive R Archive Network
     *
     * \ingroup grpcranrepository
     */
    class PALUDIS_VISIBLE CRANRepository :
        public Repository,
        public RepositoryInstallableInterface,
        public RepositorySyncableInterface,
        public RepositorySetsInterface,
        private PrivateImplementationPattern<CRANRepository>
    {
        private:
            void need_packages() const;
            void need_version_names(const QualifiedPackageName &) const;
            void need_virtual_names() const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

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

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual std::tr1::shared_ptr<DepSpec> do_package_set(const SetName &) const;

            virtual std::tr1::shared_ptr<const SetNameCollection> sets_list() const;

            virtual bool do_sync() const;

        public:
            /**
             * Constructor.
             */
            CRANRepository(const CRANRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static std::tr1::shared_ptr<Repository> make_cran_repository(
                    Environment * const env,
                    std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m);

            /**
             * Destructor.
             */
            virtual ~CRANRepository();

            virtual void invalidate();
    };

    /**
     * Thrown if invalid parameters are provided for
     * CRANRepository::make_cran_repository.
     *
     * \ingroup grpexceptions
     * \ingroup grpportagerepository
     */
    class PALUDIS_VISIBLE CRANRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            CRANRepositoryConfigurationError(const std::string & msg) throw ();
    };
}

#endif
