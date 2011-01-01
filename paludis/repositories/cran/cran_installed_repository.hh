/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/pimp.hh>
#include <paludis/util/map-fwd.hh>

/** \file
 * Declarations for CRANInstalledRepository.
 *
 * \ingroup grpcraninstrepository
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_location> location;
        typedef Name<struct name_root> root;
    }

    /**
     * Parameters used to create a CRANInstalledRepository
     *
     * \see CRANInstalledRepository
     * \ingroup grpcraninstrepository
     * \nosubgrouping
     */
    struct CRANInstalledRepositoryParams
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::location, FSPath> location;
        NamedValue<n::root, FSPath> root;
    };

    /**
     * A CRANInstalledRepository represents the database used for
     * installed CRAN packages.
     *
     * \ingroup grpcraninstrepository
     */
    class PALUDIS_VISIBLE CRANInstalledRepository :
        public Repository,
        public RepositoryDestinationInterface,
        public Pimp<CRANInstalledRepository>
    {
        private:
            Pimp<CRANInstalledRepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

            void need_ids() const;

        protected:
            virtual void need_keys_added() const;

        public:
            /**
             * Constructor.
             */
            CRANInstalledRepository(const CRANInstalledRepositoryParams &);

            ///\name RepositoryFactory functions
            ///\{

            static RepositoryName repository_factory_name(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<Repository> repository_factory_create(
                    Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            static std::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::function<std::string (const std::string &)> &);

            ///\}

            /**
             * Destructor.
             */
            ~CRANInstalledRepository();

            virtual void invalidate();
            virtual void invalidate_masks();
            virtual HookResult perform_hook(
                    const Hook & hook,
                    const std::shared_ptr<OutputManager> &);

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeParams &);

            /* Repository */

            virtual std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const bool is_unimportant() const;

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual bool some_ids_might_not_be_masked() const;

            virtual bool sync(const std::string &, const std::shared_ptr<OutputManager> &) const;

            /* Keys */

            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
            virtual const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const;

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}
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
