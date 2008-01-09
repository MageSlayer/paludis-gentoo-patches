/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/action-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/map.hh>
#include <paludis/repositories/e/e_repository_id.hh>

/** \file
 * Declarations for VDBRepository.
 *
 * \ingroup grpvdbrepository
 */

namespace paludis
{

#include <paludis/repositories/e/vdb_repository-sr.hh>

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
        public RepositoryUseInterface,
        public RepositorySetsInterface,
        public RepositoryWorldInterface,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryProvidesInterface,
        public RepositoryDestinationInterface,
        public RepositoryHookInterface,
        public tr1::enable_shared_from_this<VDBRepository>,
        public PrivateImplementationPattern<VDBRepository>
    {
        private:
            PrivateImplementationPattern<VDBRepository>::ImpPtr & _imp;
            void _add_metadata_keys() const;

            bool load_provided_using_cache() const;
            void load_provided_the_slow_way() const;

            void regenerate_provides_cache() const;

            void need_category_names() const;
            void need_package_ids(const CategoryNamePart &) const;

            void add_string_to_world(const std::string & n) const;
            void remove_string_from_world(const std::string &) const;

            const tr1::shared_ptr<const erepository::ERepositoryID> package_id_if_exists(const QualifiedPackageName &,
                    const VersionSpec &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const tr1::shared_ptr<const erepository::ERepositoryID> make_id(const QualifiedPackageName &, const VersionSpec &,
                    const FSEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

        protected:
            virtual void need_keys_added() const;

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
                    tr1::shared_ptr<const Map<std::string, std::string> > m);

            /**
             * Destructor.
             */
            ~VDBRepository();

            virtual void invalidate();

            virtual void invalidate_masks();

            virtual void regenerate_cache() const;

            ///\name For use by VDBID
            ///\{

            void perform_uninstall(const tr1::shared_ptr<const erepository::ERepositoryID> & id,
                    const UninstallActionOptions & o, bool reinstalling) const;

            void perform_config(const tr1::shared_ptr<const erepository::ERepositoryID> & id) const;

            void perform_info(const tr1::shared_ptr<const erepository::ERepositoryID> & id) const;

            ///\}

            /* RepositoryUseInterface */

            virtual UseFlagState query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool query_use_mask(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool query_use_force(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> arch_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> use_expand_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> use_expand_hidden_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> use_expand_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual char use_expand_separator(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string describe_use_flag(const UseFlagName &,
                    const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositorySetsInterface */

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> package_set(const SetName & id) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const SetNameSet> sets_list() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

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

            /* RepositoryHookInterface */

            virtual HookResult perform_hook(const Hook & hook) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

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

            virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

            /* Keys */
            virtual const tr1::shared_ptr<const MetadataStringKey> format_key() const;
            virtual const tr1::shared_ptr<const MetadataFSEntryKey> installed_root_key() const;
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
