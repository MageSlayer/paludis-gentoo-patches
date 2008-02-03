/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EXNDBAM_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_EXNDBAM_REPOSITORY_HH 1

#include <paludis/repositories/e/e_installed_repository.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/map.hh>
#include <paludis/repository.hh>

namespace paludis
{
    namespace erepository
    {
        class ERepositoryID;
    }

#include <paludis/repositories/e/exndbam_repository-sr.hh>

    class PALUDIS_VISIBLE ExndbamRepository :
        public erepository::EInstalledRepository,
        public tr1::enable_shared_from_this<ExndbamRepository>,
        public PrivateImplementationPattern<ExndbamRepository>
    {
        private:
            PrivateImplementationPattern<ExndbamRepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

        protected:
            virtual void need_keys_added() const;

        public:
            /**
             * Constructor.
             */
            ExndbamRepository(const RepositoryName & n, const ExndbamRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static tr1::shared_ptr<Repository> make_exndbam_repository(
                    Environment * const env,
                    tr1::shared_ptr<const Map<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~ExndbamRepository();

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual void regenerate_cache() const;

            /* RepositoryDestinationInterface */

            virtual void merge(const MergeOptions &);

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Keys */

            virtual const tr1::shared_ptr<const MetadataStringKey> format_key() const;
            virtual const tr1::shared_ptr<const MetadataFSEntryKey> installed_root_key() const;

            ///\name For use by ExndbamID
            ///\{

            void perform_uninstall(const tr1::shared_ptr<const erepository::ERepositoryID> & id,
                    const UninstallActionOptions & o, bool reinstalling) const;

            ///\}

    };

    class PALUDIS_VISIBLE ExndbamRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            ExndbamRepositoryConfigurationError(const std::string & msg) throw ();
    };
}

#endif
