/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/map-fwd.hh>

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
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryDestinationInterface,
        public PrivateImplementationPattern<CRANInstalledRepository>
    {
        private:
            void need_ids() const;
            void add_string_to_world(const std::string & n) const;
            void remove_string_from_world(const std::string &) const;

        protected:
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

            /* RepositorySetsInterface */

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> do_package_set(const SetName & id) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            /**
             * Constructor.
             */
            CRANInstalledRepository(const CRANInstalledRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static tr1::shared_ptr<Repository> make_cran_installed_repository(
                    Environment * const env,
                    tr1::shared_ptr<const Map<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~CRANInstalledRepository();

            virtual void invalidate();
            virtual void invalidate_masks();

            /* RepositoryInstalledInterface */

            virtual FSEntry root() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositorySetsInterface */

            virtual tr1::shared_ptr<const SetNameSet> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryWorldInterface */

            virtual void add_to_world(const QualifiedPackageName &) const;
            virtual void add_to_world(const SetName &) const;
            virtual void remove_from_world(const QualifiedPackageName &) const;
            virtual void remove_from_world(const SetName &) const;

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeOptions &);
    };

    /**
     * Thrown if invalid parameters are provided for
     * CRANInstalledRepositoryConfigurationError::make_cran_installed_repository.
     *
     * \ingroup grpcraninstrepository
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE CRANInstalledRepositoryConfigurationError :
        public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            CRANInstalledRepositoryConfigurationError(const std::string & msg) throw ();
    };
}

#endif
