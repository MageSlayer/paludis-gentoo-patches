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

#ifndef PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_VDB_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>

/** \file
 * Declarations for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

namespace paludis
{

#include <paludis/repositories/gentoo/vdb_repository-sr.hh>

    /**
     * A VDBRepository represents the /var/db/pkg database used for
     * installed packages.
     *
     * It has a stupid name because Portage called it that.
     *
     * \ingroup grpvdbrepository
     */
    class PALUDIS_VISIBLE VDBRepository :
        public Repository,
        public RepositoryInstalledInterface,
        public RepositoryUseInterface,
        public RepositoryUninstallableInterface,
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryProvidesInterface,
        public RepositoryDestinationInterface,
        public RepositoryContentsInterface,
        public RepositoryConfigInterface,
        public RepositoryHookInterface,
        public PrivateImplementationPattern<VDBRepository>
    {
        private:
            bool load_provided_using_cache() const;
            void load_provided_the_slow_way() const;

            void regenerate_provides_cache() const;

            void _uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const UninstallOptions &, bool reinstalling) const;

        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const Contents> do_contents(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual time_t do_installed_time(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &, 
                    const UninstallOptions &) const;

            virtual std::tr1::shared_ptr<DepSpec> do_package_set(const SetName &) const;

            /* RepositoryUseInterface */

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual bool do_query_use_mask(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual bool do_query_use_force(const UseFlagName &, const PackageDatabaseEntry &) const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_arch_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_flags() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_hidden_prefixes() const;
            virtual std::tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_prefixes() const;

            /* end of RepositoryUseInterface */

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names_containing_package(
                    const PackageNamePart &) const;

            /**
             * Add a string to world.
             */
            virtual void add_string_to_world(const std::string &) const;

            /**
             * Remove a string from world.
             */
            virtual void remove_string_from_world(const std::string &) const;

            virtual void do_config(const QualifiedPackageName &, const VersionSpec &) const;

        public:
            /**
             * Constructor.
             */
            VDBRepository(const VDBRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static std::tr1::shared_ptr<Repository> make_vdb_repository(
                    Environment * const env,
                    std::tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual void invalidate();

            virtual void regenerate_cache() const;

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const SetName &) const;

            virtual std::string get_environment_variable(
                    const PackageDatabaseEntry & for_package,
                    const std::string & var) const;

            virtual std::tr1::shared_ptr<const ProvidesCollection> provided_packages() const;

            virtual std::tr1::shared_ptr<const VersionMetadata> provided_package_version_metadata(
                    const RepositoryProvidesEntry &) const;

            virtual std::tr1::shared_ptr<const SetNameCollection> sets_list() const;

            virtual bool is_suitable_destination_for(const PackageDatabaseEntry &) const;

            virtual bool is_default_destination() const;

            virtual bool want_pre_post_phases() const;

            void merge(const MergeOptions &);

            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageDatabaseEntry &) const;

            virtual FSEntry root() const;

            int perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Thrown if invalid parameters are provided for
     * PortageRepository::make_portage_repository.
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
