/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_REPOSITORY_REPOSITORY_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_REPOSITORY_REPOSITORY_REPOSITORY_HH 1

#include <paludis/repositories/repository/repository_repository-fwd.hh>
#include <paludis/repository.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_config_filename> config_filename;
        typedef Name<struct name_config_template> config_template;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_name> name;
        typedef Name<struct name_root> root;
    }

    namespace repository_repository
    {
        class PALUDIS_VISIBLE RepositoryRepositoryConfigurationError :
            public ConfigurationError
        {
            public:
                RepositoryRepositoryConfigurationError(const std::string &) throw ();
        };

        struct RepositoryRepositoryParams
        {
            NamedValue<n::config_filename, std::string> config_filename;
            NamedValue<n::config_template, std::string> config_template;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::name, RepositoryName> name;
            NamedValue<n::root, FSPath> root;
        };

        class PALUDIS_VISIBLE RepositoryRepository :
            public Repository,
            public RepositoryDestinationInterface,
            public std::enable_shared_from_this<RepositoryRepository>
        {
            private:
                Pimp<RepositoryRepository> _imp;

                void _add_metadata_keys();

            protected:
                virtual void need_keys_added() const;

            public:
                RepositoryRepository(const RepositoryRepositoryParams &);
                ~RepositoryRepository();

                virtual const bool is_unimportant() const;

                virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const;
                virtual const std::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
                virtual const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const;

                virtual bool has_category_named(const CategoryNamePart & c, const RepositoryContentMayExcludes &) const;
                virtual bool has_package_named(const QualifiedPackageName & q, const RepositoryContentMayExcludes &) const;
                virtual std::shared_ptr<const CategoryNamePartSet> category_names(const RepositoryContentMayExcludes &) const;
                virtual std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(const RepositoryContentMayExcludes &) const;
                virtual std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                        const PackageNamePart & p, const RepositoryContentMayExcludes &) const;
                virtual std::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c, const RepositoryContentMayExcludes &) const;
                virtual std::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p, const RepositoryContentMayExcludes &) const;

                virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;
                virtual bool some_ids_might_not_be_masked() const;
                virtual void invalidate();

                virtual bool sync(const std::string &, const std::string &, const std::shared_ptr<OutputManager> &) const;

                virtual bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool is_default_destination() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool want_pre_post_phases() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void merge(const MergeParams &);

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

                virtual void populate_sets() const;

                ///\}

                virtual HookResult perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &);
            };
    }

    extern template class Pimp<repository_repository::RepositoryRepository>;
}

#endif
