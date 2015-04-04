/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_ACCOUNTS_ACCOUNTS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_ACCOUNTS_ACCOUNTS_REPOSITORY_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/pimp.hh>
#include <paludis/repository.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_handler> handler;
        typedef Name<struct name_installed> installed;
        typedef Name<struct name_name> name;
        typedef Name<struct name_root> root;
    }

    namespace accounts_repository
    {
        struct AccountsRepositoryParams
        {
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::name, RepositoryName> name;
        };

        struct InstalledAccountsRepositoryParams
        {
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::handler, std::string> handler;
            NamedValue<n::name, RepositoryName> name;
            NamedValue<n::root, FSPath> root;
        };

        class PALUDIS_VISIBLE AccountsRepository :
            public Repository,
            public RepositoryDestinationInterface,
            public std::enable_shared_from_this<AccountsRepository>
        {
            private:
                Pimp<AccountsRepository> _imp;

                void _add_metadata_keys();

            protected:
                void need_keys_added() const override;

            public:
                AccountsRepository(const AccountsRepositoryParams &);
                AccountsRepository(const InstalledAccountsRepositoryParams &);

                ~AccountsRepository() override;

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

                ///\name RepositoryFactory functions for Installed
                ///\{

                static RepositoryName repository_factory_installed_name(
                        const Environment * const env,
                        const std::function<std::string (const std::string &)> &);

                static std::shared_ptr<Repository> repository_factory_installed_create(
                        Environment * const env,
                        const std::function<std::string (const std::string &)> &);

                static std::shared_ptr<const RepositoryNameSet> repository_factory_installed_dependencies(
                        const Environment * const env,
                        const std::function<std::string (const std::string &)> &);

                ///\}

                ///\name Specific metadata keys
                ///\{

                const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const override;

                ///\}

                ///\name Repository content queries
                ///\{

                const bool is_unimportant() const override;
                bool has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const override;
                bool has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const override;
                std::shared_ptr<const CategoryNamePartSet> category_names(const RepositoryContentMayExcludes &) const override;
                std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(const RepositoryContentMayExcludes &) const override;
                std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                        const PackageNamePart & p, const RepositoryContentMayExcludes &) const override;
                std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c, const RepositoryContentMayExcludes &) const override;
                std::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p, const RepositoryContentMayExcludes &) const override;
                bool some_ids_might_support_action(const SupportsActionTestBase &) const override;
                bool some_ids_might_not_be_masked() const override;
                const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                        const std::string &) const override;

                ///\}

                ///\name Repository behaviour methods
                ///\{

                void invalidate() override;
                void regenerate_cache() const override;

                HookResult perform_hook(
                        const Hook & hook,
                        const std::shared_ptr<OutputManager> &) override;

                bool sync(
                        const std::string &,
                        const std::string &,
                        const std::shared_ptr<OutputManager> &) const override;

                ///\}

                ///\name Destination functions
                ///\{

                bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));
                bool want_pre_post_phases() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));
                void merge(const MergeParams &) override;

                ///\}

                ///\name Set methods
                ///\{

                void populate_sets() const override;

                ///\}
        };
    }
}

#endif
