/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/mask-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/repositories/e/e_repository_params.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/mask_info.hh>
#include <memory>
#include <string>

/** \file
 * Declaration for the ERepository class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
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
        public RepositoryEnvironmentVariableInterface,
        public RepositoryDestinationInterface,
        public RepositoryManifestInterface,
        public std::enable_shared_from_this<ERepository>
    {
        private:
            Pimp<ERepository> _imp;

            void _add_metadata_keys() const;

            void need_mirrors() const;

        protected:
            void need_keys_added() const override;

        public:
            /**
             * Constructor.
             */
            ERepository(const erepository::ERepositoryParams &);

            /**
             * Destructor.
             */
            ~ERepository() override;

            void invalidate() override;

            void purge_invalid_cache() const override;

            /* RepositoryDestinationInterface */

            bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool want_pre_post_phases() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            void merge(const MergeParams &) override;

            /* RepositoryManifestInterface */
            void make_manifest(const QualifiedPackageName & qpn) override;

            /* RepositorySyncableInterface */

            bool sync(
                    const std::string &,
                    const std::string &,
                    const std::shared_ptr<OutputManager> &) const override;

            /* RepositoryEnvironmentVariableInterface */

            std::string get_environment_variable(
                    const std::shared_ptr<const PackageID> & for_package,
                    const std::string & var) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            /* Repository */

            std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const CategoryNamePartSet> category_names(const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &, const RepositoryContentMayExcludes &) const override;

            bool has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool some_ids_might_support_action(const SupportsActionTestBase &) const override;

            bool some_ids_might_not_be_masked() const override;

            const erepository::ERepositoryParams & params() const;

            std::string profile_variable(const std::string &) const;
            std::string environment_updated_profile_variable(const std::string &) const;

            HookResult perform_hook(const Hook &, const std::shared_ptr<OutputManager> &)
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(const RepositoryContentMayExcludes &) const override;
            const bool is_unimportant() const override;

            const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                    const std::string &) const override;

            /**
             * Update GLEP 42 news files.
             */
            void update_news() const;

            const std::shared_ptr<const erepository::Layout> layout() const;
            const std::shared_ptr<const erepository::Profile> profile() const;

            void regenerate_cache() const override;

            /* Keys */

            const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
            virtual const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > > info_vars_key() const;
            const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;
            const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const override;

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

            const std::shared_ptr<const Set<UnprefixedChoiceName> > arch_flags() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const UseDesc> use_desc() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string eapi_for_file(const FSPath &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const MirrorsSequence> get_mirrors(const std::string & m) const;

            ///\name Set methods
            ///\{

            void populate_sets() const override;

            ///\}

            ///\name Entries
            ///\{

            VersionSpec extract_package_file_version(const QualifiedPackageName &, const FSPath &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const erepository::ERepositoryID> make_id(
                    const QualifiedPackageName &, const FSPath &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string binary_ebuild_name(const QualifiedPackageName &, const VersionSpec &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };
}

#endif
