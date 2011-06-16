/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH 1

#include <paludis/repositories/e/e_installed_repository.hh>
#include <paludis/repository.hh>
#include <paludis/action-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/map.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <memory>

/** \file
 * Declarations for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_builddir> builddir;
        typedef Name<struct name_eapi_when_unknown> eapi_when_unknown;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_location> location;
        typedef Name<struct name_name> name;
        typedef Name<struct name_names_cache> names_cache;
        typedef Name<struct name_root> root;
    }

    namespace erepository
    {
        struct VDBRepositoryParams
        {
            NamedValue<n::builddir, FSPath> builddir;
            NamedValue<n::eapi_when_unknown, std::string> eapi_when_unknown;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::location, FSPath> location;
            NamedValue<n::name, RepositoryName> name;
            NamedValue<n::names_cache, FSPath> names_cache;
            NamedValue<n::root, FSPath> root;
        };
    }

    /**
     * A VDBRepository represents the /var/db/pkg database used for
     * installed packages.
     *
     * It has a stupid name because Portage called it that.
     *
     * \ingroup grpvdbrepository
     */
    class PALUDIS_VISIBLE VDBRepository :
        public erepository::EInstalledRepository,
        public std::enable_shared_from_this<VDBRepository>
    {
        private:
            Pimp<VDBRepository> _imp;

            void _add_metadata_keys() const;

            void need_category_names() const;
            void need_package_ids(const CategoryNamePart &) const;

            const std::shared_ptr<const erepository::ERepositoryID> package_id_if_exists(const QualifiedPackageName &,
                    const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const erepository::ERepositoryID> make_id(const QualifiedPackageName &, const VersionSpec &,
                    const FSPath &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        protected:
            virtual void need_keys_added() const;

        public:
            /**
             * Constructor.
             */
            VDBRepository(const erepository::VDBRepositoryParams &);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual void invalidate();

            virtual void regenerate_cache() const;

            virtual void perform_uninstall(
                    const std::shared_ptr<const erepository::ERepositoryID> & id,
                    const UninstallAction &) const;

            /* RepositoryDestinationInterface */

            virtual void merge(const MergeParams &);

            /* Repository */

            virtual std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names(
                    const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const bool is_unimportant() const;

            /* Keys */
            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const;
            virtual const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const;

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

            virtual void perform_updates();
    };

    /**
     * Thrown if invalid parameters are provided for
     * VDBRepository::make_vdb_repository.
     *
     * \ingroup grpvdbrepository
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE VDBRepositoryConfigurationError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            VDBRepositoryConfigurationError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a key read fails.
     *
     * \ingroup grpvdbrepository
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE VDBRepositoryKeyReadError : public ConfigurationError
    {
        public:
            /**
             * Constructor.
             */
            VDBRepositoryKeyReadError(const std::string & msg) throw ();
    };
}

#endif
