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

#ifndef PALUDIS_GUARD_PALUDIS_CRAN_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/environment-fwd.hh>
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
        public RepositorySyncableInterface,
        public RepositorySetsInterface,
        private PrivateImplementationPattern<CRANRepository>,
        public tr1::enable_shared_from_this<CRANRepository>
    {
        private:
            void need_ids() const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

            /* RepositorySyncableInterface */

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> do_package_set(const SetName &) const;

            virtual tr1::shared_ptr<const SetNameSet> sets_list() const;

            /* RepositorySyncableInterface */

            virtual bool do_sync() const;

            /* Repository */

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_some_ids_might_support_action(const SupportsActionTestBase &) const;

        public:
            /**
             * Constructor.
             */
            CRANRepository(const CRANRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static tr1::shared_ptr<Repository> make_cran_repository(
                    Environment * const env,
                    tr1::shared_ptr<const Map<std::string, std::string> > m);

            /**
             * Destructor.
             */
            virtual ~CRANRepository();

            virtual void invalidate();
            virtual void invalidate_masks();
    };

    /**
     * Thrown if invalid parameters are provided for
     * CRANRepository::make_cran_repository.
     *
     * \ingroup grpexceptions
     * \ingroup grpcranrepository
     */
    class PALUDIS_VISIBLE CRANRepositoryConfigurationError :
        public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            CRANRepositoryConfigurationError(const std::string & msg) throw ();
    };
}

#endif
