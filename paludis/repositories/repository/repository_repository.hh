/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace n
    {
        struct config_filename;
        struct config_template;
        struct environment;
        struct name;
        struct root;
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
            NamedValue<n::root, FSEntry> root;
        };

        class PALUDIS_VISIBLE RepositoryRepository :
            private PrivateImplementationPattern<RepositoryRepository>,
            public Repository,
            public std::tr1::enable_shared_from_this<RepositoryRepository>
        {
            private:
                PrivateImplementationPattern<RepositoryRepository>::ImpPtr & _imp;

                void _add_metadata_keys();

            protected:
                virtual void need_keys_added() const;

            public:
                RepositoryRepository(const RepositoryRepositoryParams &);
                ~RepositoryRepository();

                virtual bool can_be_favourite_repository() const;

                virtual const bool is_unimportant() const;

                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > location_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > installed_root_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > accept_keywords_key() const;
                virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > sync_host_key() const;

                virtual bool has_category_named(const CategoryNamePart & c) const;
                virtual bool has_package_named(const QualifiedPackageName & q) const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names() const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;
                virtual std::tr1::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                        const PackageNamePart & p) const;
                virtual std::tr1::shared_ptr<const QualifiedPackageNameSet> package_names(
                        const CategoryNamePart & c) const;
                virtual std::tr1::shared_ptr<const PackageIDSequence> package_ids(const QualifiedPackageName & p) const;

                virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;
                virtual void invalidate();
                virtual void invalidate_masks();

                virtual bool sync(const std::tr1::shared_ptr<OutputManager> &) const;

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

                ///\name Set methods
                ///\{

                virtual void populate_sets() const;

                ///\}

                virtual HookResult perform_hook(const Hook & hook);
            };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<repository_repository::RepositoryRepository>;
#endif
}

#endif
