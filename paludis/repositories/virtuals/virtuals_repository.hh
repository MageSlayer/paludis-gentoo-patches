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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VIRTUALS_REPOSITORY_HH 1

#include <paludis/repository.hh>

namespace paludis
{
    class VirtualsRepository :
        public Repository,
        public RepositoryInstallableInterface,
        public RepositoryMaskInterface,
        private PrivateImplementationPattern<VirtualsRepository>
    {
        private:
            void need_entries() const;

        protected:
            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual VersionMetadata::ConstPointer do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual VersionSpecCollection::ConstPointer do_version_specs(
                    const QualifiedPackageName &) const;

            virtual QualifiedPackageNameCollection::ConstPointer do_package_names(
                    const CategoryNamePart &) const;

            virtual CategoryNamePartCollection::ConstPointer do_category_names() const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_is_licence(const std::string &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

        public:
            VirtualsRepository(const Environment * const env);

            virtual ~VirtualsRepository();

            static CountedPtr<Repository> make_virtuals_repository(
                    const Environment * const env,
                    const PackageDatabase * const db,
                    AssociativeCollection<std::string, std::string>::ConstPointer);

            virtual void invalidate() const;

            virtual bool can_be_favourite_repository() const
            {
                return false;
            }

    };

    static const RepositoryMaker::RegisterMaker register_virtuals_repository(
            "virtuals", &VirtualsRepository::make_virtuals_repository);
}

#endif
