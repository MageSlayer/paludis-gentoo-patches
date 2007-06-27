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
#include <paludis/util/tr1_memory.hh>

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
        public tr1::enable_shared_from_this<VDBRepository>,
        public PrivateImplementationPattern<VDBRepository>
    {
        private:
            bool load_provided_using_cache() const;
            void load_provided_the_slow_way() const;

            void regenerate_provides_cache() const;

            void _uninstall(const tr1::shared_ptr<const PackageID> &, const UninstallOptions &,
                    bool reinstalling) const;

            void need_category_names() const;
            void need_package_ids(const CategoryNamePart &) const;

            void add_string_to_world(const std::string & n) const;
            void remove_string_from_world(const std::string &) const;

            const tr1::shared_ptr<const PackageID> package_id_if_exists(const QualifiedPackageName &,
                    const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const tr1::shared_ptr<const PackageID> make_id(const QualifiedPackageName &, const VersionSpec &,
                    const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryInstalledInterface */

            virtual time_t do_installed_time(const PackageID &) const;

            virtual FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryUseInterface */

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_query_use_mask(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_query_use_force(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameCollection> do_arch_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_hidden_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameCollection> do_use_expand_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositorySetsInterface */

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> do_package_set(const SetName & id) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const SetNameCollection> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryUninstallableInterface */

            virtual void do_uninstall(const tr1::shared_ptr<const PackageID> &, const UninstallOptions &) const;

            /* RepositoryWorldInterface */

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const SetName &) const;

            /* RepositoryProvidesInterface */

            virtual tr1::shared_ptr<const ProvidesSequence> provided_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryEnvironmentVariableInterface */

            virtual std::string get_environment_variable(
                    const tr1::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeOptions &);

            /* RepositoryContentsInterface */

            virtual tr1::shared_ptr<const Contents> do_contents(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryConfigInterface */

            virtual void do_config(const tr1::shared_ptr<const PackageID> &) const;

            /* RepositoryHookInterface */

            virtual HookResult perform_hook(const Hook & hook) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents: fetch package names.
             */
            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents: fetch category names.
             */
            virtual tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents if a fast implementation is available: fetch category names
             * that contain a particular package.
             */
            virtual tr1::shared_ptr<const CategoryNamePartCollection> do_category_names_containing_package(
                    const PackageNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents: check for a package.
             */
            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Override in descendents: check for a category.
             */
            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            /**
             * Constructor.
             */
            VDBRepository(const VDBRepositoryParams &);

            /**
             * Virtual constructor.
             */
            static tr1::shared_ptr<Repository> make_vdb_repository(
                    Environment * const env,
                    tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual void invalidate();

            virtual void regenerate_cache() const;
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
