/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNAVAILABLE_UNAVAILABLE_REPOSITORY_HH 1

#include <paludis/repositories/unavailable/unavailable_repository-fwd.hh>
#include <paludis/repository.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_location> location;
        typedef Name<struct name_name> name;
        typedef Name<struct name_sync> sync;
        typedef Name<struct name_sync_options> sync_options;
    }

    namespace unavailable_repository
    {
        class PALUDIS_VISIBLE UnavailableRepositoryConfigurationError :
            public ConfigurationError
        {
            public:
                UnavailableRepositoryConfigurationError(const std::string &) noexcept;
        };

        struct UnavailableRepositoryParams
        {
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::location, FSPath> location;
            NamedValue<n::name, RepositoryName> name;
            NamedValue<n::sync, std::shared_ptr<Map<std::string, std::string> > > sync;
            NamedValue<n::sync_options, std::shared_ptr<Map<std::string, std::string> > > sync_options;
        };

        class PALUDIS_VISIBLE UnavailableRepository :
            public Repository,
            public std::enable_shared_from_this<UnavailableRepository>
        {
            private:
                Pimp<UnavailableRepository> _imp;

                void _add_metadata_keys();

            protected:
                void need_keys_added() const override;

            public:
                UnavailableRepository(const UnavailableRepositoryParams &);
                ~UnavailableRepository() override;

                const bool is_unimportant() const override;

                const std::shared_ptr<const MetadataValueKey<std::string>> cross_compile_host_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
                const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
                const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;
                const std::shared_ptr<const MetadataValueKey<std::string>> tool_prefix_key() const override;

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
                void invalidate() override;

                bool sync(const std::string &, const std::string &, const std::shared_ptr<OutputManager> &) const override;

                const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                        const std::string &) const override;

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

                ///\name Set methods
                ///\{

                void populate_sets() const override;

                ///\}

                HookResult perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &) override;
            };
    }

    extern template class Pimp<unavailable_repository::UnavailableRepository>;
}

#endif
