/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
    /**
     * A repository holding packages representing virtuals.
     *
     * \ingroup grpvirtualsrepository
     */
    class PALUDIS_VISIBLE VirtualsRepository :
        public Repository,
        public RepositoryInstallableInterface,
        public RepositoryMaskInterface,
        private PrivateImplementationPattern<VirtualsRepository>
    {
        private:
            void need_names() const;
            void need_entries() const;

        protected:
            virtual bool do_query_repository_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_query_profile_masks(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual bool do_has_version(const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

        public:
            ///\name Basic operations
            ///\{

            VirtualsRepository(const Environment * const env);

            virtual ~VirtualsRepository();

            ///\}

            /**
             * Create a VirtualsRepository instance.
             */
            static tr1::shared_ptr<Repository> make_virtuals_repository(
                    Environment * const env,
                    tr1::shared_ptr<const AssociativeCollection<std::string, std::string> >);

            virtual void invalidate();

            virtual bool can_be_favourite_repository() const
            {
                return false;
            }

    };
}

#endif
