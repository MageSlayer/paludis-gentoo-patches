/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_E_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_E_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_profile.hh>
#include <paludis/repositories/e/layout.hh>
#include <string>

/** \file
 * Declaration for the ERepository class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class ERepositoryProfile;
    class ERepositoryNews;

    /**
     * A ERepository is a Repository that handles the layout used by
     * Portage for the main Gentoo tree.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepository :
        public Repository,
        public RepositoryUseInterface,
        public RepositorySyncableInterface,
        public RepositorySetsInterface,
        public RepositoryEnvironmentVariableInterface,
        public RepositoryMirrorsInterface,
        public RepositoryVirtualsInterface,
        public RepositoryDestinationInterface,
        public RepositoryLicensesInterface,
        public RepositoryEInterface,
        public RepositoryHookInterface,
        public RepositoryQAInterface,
        public RepositoryManifestInterface,
        public tr1::enable_shared_from_this<ERepository>,
        private PrivateImplementationPattern<ERepository>
    {
        private:
            void need_mirrors() const;

        protected:
            /* RepositoryLicensesInterface */

            virtual tr1::shared_ptr<FSEntry> do_license_exists(
                    const std::string & license) const;

            /* RepositorySetsInterface */

            virtual tr1::shared_ptr<SetSpecTree::ConstItem> do_package_set(const SetName & id) const;

            virtual tr1::shared_ptr<const SetNameSet> sets_list() const;

            /* RepositorySyncableInterface */

            virtual bool do_sync() const;

            /* RepositoryUseInterface */

            virtual UseFlagState do_query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_query_use_mask(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_query_use_force(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> do_arch_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_flags() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_hidden_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const UseFlagNameSet> do_use_expand_prefixes() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string do_describe_use_flag(const UseFlagName &,
                    const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryEnvironmentVariableInterface */

            virtual std::string get_environment_variable(
                    const tr1::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Repository */

            virtual tr1::shared_ptr<const PackageIDSequence> do_package_ids(
                    const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const QualifiedPackageNameSet> do_package_names(
                    const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<const CategoryNamePartSet> do_category_names_containing_package(
                    const PackageNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_has_category_named(const CategoryNamePart &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool do_some_ids_might_support_action(const SupportsActionTestBase &) const;

        public:
            virtual tr1::shared_ptr<const RepositoryInfo> info(bool verbose) const;

            /**
             * Constructor.
             */
            ERepository(const ERepositoryParams &);

            /**
             * Destructor.
             */
            ~ERepository();

            virtual void invalidate();

            virtual void invalidate_masks();

            /* RepositoryMirrorsInterface */

            virtual MirrorsIterator begin_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual MirrorsIterator end_mirrors(const std::string & s) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryVirtualsInterface */

            virtual tr1::shared_ptr<const VirtualsSequence> virtual_packages() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /* RepositoryDestinationInterface */

            virtual bool is_suitable_destination_for(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_default_destination() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool want_pre_post_phases() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void merge(const MergeOptions &);

            virtual void check_qa(
                    QAReporter &,
                    const QACheckProperties &,
                    const QACheckProperties &,
                    const QAMessageLevel,
                    const FSEntry &
                    ) const;

            /* RepositoryManifestInterface */
            virtual void make_manifest(const QualifiedPackageName & qpn);

            ///\name Information about ERepository
            ///\{

            std::string profile_variable(const std::string &) const;
            virtual std::string accept_keywords_variable() const;
            virtual std::string arch_variable() const;

            const ERepositoryParams & params() const;

            virtual FSEntry info_variables_file(const FSEntry &) const;

            ///\}

            ///\name Profile setting and querying functions
            ///\{

            ProfilesIterator begin_profiles() const;
            ProfilesIterator end_profiles() const;

            ProfilesIterator find_profile(const FSEntry & location) const;
            void set_profile(const ProfilesIterator & iter);
            void set_profile_by_arch(const UseFlagName &);

            ///\}

            HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Update GLEP 42 news files.
             */
            void update_news() const;

            const tr1::shared_ptr<const erepository::Layout> layout() const;
            const tr1::shared_ptr<const erepository::ERepositoryEntries> entries() const;
            const tr1::shared_ptr<const ERepositoryProfile> profile() const;

            tr1::shared_ptr<const RepositoryMaskInfo> repository_masked(const PackageID &) const;

            void regenerate_cache() const;
    };
}

#endif
