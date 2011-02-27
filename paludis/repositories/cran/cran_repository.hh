/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp.hh>
#include <paludis/environment-fwd.hh>
#include <memory>
#include <string>

/** \file
 * Declaration for the CRANRepository class.
 *
 * \ingroup grpcranrepository
 */

namespace paludis
{
    class PackageDatabase;

    namespace n
    {
        typedef Name<struct name_builddir> builddir;
        typedef Name<struct name_distdir> distdir;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_library> library;
        typedef Name<struct name_location> location;
        typedef Name<struct name_mirror> mirror;
        typedef Name<struct name_sync> sync;
    }

    /**
     * Parameters used to create a CRANRepository
     *
     * \see CRANRepository
     * \ingroup grpcranrepository
     * \nosubgrouping
     */
    struct CRANRepositoryParams
    {
        NamedValue<n::builddir, FSPath> builddir;
        NamedValue<n::distdir, FSPath> distdir;
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::library, FSPath> library;
        NamedValue<n::location, FSPath> location;
        NamedValue<n::mirror, std::string> mirror;
        NamedValue<n::sync, std::shared_ptr<Map<std::string, std::string> > > sync;
    };

    /**
     * A CRANRepository is a Repository that handles the layout used by
     * the GNU R project for the Comprehensive R Archive Network
     *
     * \ingroup grpcranrepository
     */
    class PALUDIS_VISIBLE CRANRepository :
        public Repository,
        public std::enable_shared_from_this<CRANRepository>
    {
        private:
            Pimp<CRANRepository> _imp;

            void _add_metadata_keys() const;

            void need_ids() const;

        protected:
            virtual void need_keys_added() const;

        protected:
            /**
             * Try to get the repository name for a particular repository.
             */
            static RepositoryName fetch_repo_name(const std::string & location);

        public:
            /**
             * Constructor.
             */
            CRANRepository(const CRANRepositoryParams &);

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
            virtual ~CRANRepository();

            virtual void invalidate();
            virtual void invalidate_masks();

            /* RepositorySyncableInterface */

            virtual bool sync(const std::string &, const std::shared_ptr<OutputManager> & output_deviant) const;

            /* Repository */

            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names(const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const bool is_unimportant() const;

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            virtual bool some_ids_might_not_be_masked() const;

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

            virtual HookResult perform_hook(
                    const Hook & hook,
                    const std::shared_ptr<OutputManager> &);
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
