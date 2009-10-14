/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/environment-fwd.hh>
#include <tr1/memory>
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
        struct builddir;
        struct distdir;
        struct environment;
        struct library;
        struct location;
        struct mirror;
        struct sync;
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
        NamedValue<n::builddir, FSEntry> builddir;
        NamedValue<n::distdir, FSEntry> distdir;
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::library, FSEntry> library;
        NamedValue<n::location, FSEntry> location;
        NamedValue<n::mirror, std::string> mirror;
        NamedValue<n::sync, std::string> sync;
    };

    /**
     * A CRANRepository is a Repository that handles the layout used by
     * the GNU R project for the Comprehensive R Archive Network
     *
     * \ingroup grpcranrepository
     */
    class PALUDIS_VISIBLE CRANRepository :
        public Repository,
        public RepositorySyncableInterface,
        private PrivateImplementationPattern<CRANRepository>,
        public std::tr1::enable_shared_from_this<CRANRepository>
    {
        private:
            PrivateImplementationPattern<CRANRepository>::ImpPtr & _imp;
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
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<Repository> repository_factory_create(
                    Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            static std::tr1::shared_ptr<const RepositoryNameSet> repository_factory_dependencies(
                    const Environment * const env,
                    const std::tr1::function<std::string (const std::string &)> &);

            ///\}

            /**
             * Destructor.
             */
            virtual ~CRANRepository();

            virtual void invalidate();
            virtual void invalidate_masks();

            /* RepositorySyncableInterface */

            virtual bool sync(const std::tr1::shared_ptr<OutputManager> & output_deviant) const;

            /* Repository */

            virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            /* Keys */

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;

            ///\name Set methods
            ///\{

            virtual void populate_sets() const;

            ///\}

            virtual HookResult perform_hook(const Hook & hook);
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
