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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_INSTALLED_VIRTUALS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_INSTALLED_VIRTUALS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/sequence-fwd.hh>

namespace paludis
{
    /**
     * Repository representing installed virtual packages.
     *
     * \ingroup grpvirtualsrepository
     */
    class PALUDIS_VISIBLE InstalledVirtualsRepository :
        public Repository,
        public RepositoryInstalledInterface,
        public RepositoryHookInterface,
        public tr1::enable_shared_from_this<InstalledVirtualsRepository>,
        private PrivateImplementationPattern<InstalledVirtualsRepository>
    {
        private:
            void need_ids() const;

        protected:
            /* RepositoryInstalledInterface */

            virtual time_t do_installed_time(const PackageID &) const;

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_some_ids_might_support_action(const SupportsActionTestBase &) const;

        public:
            ///\name Basic operations
            //\{

            InstalledVirtualsRepository(const Environment * const env,
                    const FSEntry & root);

            virtual ~InstalledVirtualsRepository();

            ///\}

            /**
             * Create an InstalledVirtualsRepository instance.
             */
            static tr1::shared_ptr<Repository> make_installed_virtuals_repository(
                    Environment * const env,
                    tr1::shared_ptr<const Map<std::string, std::string> >);

            virtual tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual bool can_be_favourite_repository() const;

            virtual FSEntry root() const;

            HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
