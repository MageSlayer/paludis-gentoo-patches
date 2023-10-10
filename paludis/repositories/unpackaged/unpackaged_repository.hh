/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/map.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/tribool.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_build_dependencies_target> build_dependencies_target;
        typedef Name<struct name_build_dependencies_host> build_dependencies_host;
        typedef Name<struct name_description> description;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_install_under> install_under;
        typedef Name<struct name_location> location;
        typedef Name<struct name_name> name;
        typedef Name<struct name_preserve_work> preserve_work;
        typedef Name<struct name_rewrite_ids_over_to_root> rewrite_ids_over_to_root;
        typedef Name<struct name_run_dependencies_target> run_dependencies_target;
        typedef Name<struct name_run_dependencies_host> run_dependencies_host;
        typedef Name<struct name_slot> slot;
        typedef Name<struct name_strip> strip;
        typedef Name<struct name_version> version;
    }

    namespace unpackaged_repositories
    {
        struct UnpackagedRepositoryParams
        {
            NamedValue<n::build_dependencies_target, std::string> build_dependencies_target;
            NamedValue<n::build_dependencies_host, std::string> build_dependencies_host;
            NamedValue<n::description, std::string> description;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::install_under, FSPath> install_under;
            NamedValue<n::location, FSPath> location;
            NamedValue<n::name, QualifiedPackageName> name;
            NamedValue<n::preserve_work, Tribool> preserve_work;
            NamedValue<n::rewrite_ids_over_to_root, int> rewrite_ids_over_to_root;
            NamedValue<n::run_dependencies_target, std::string> run_dependencies_target;
            NamedValue<n::run_dependencies_host, std::string> run_dependencies_host;
            NamedValue<n::slot, SlotName> slot;
            NamedValue<n::strip, Tribool> strip;
            NamedValue<n::version, VersionSpec> version;
        };
    }

    class PALUDIS_VISIBLE UnpackagedRepository :
        public Repository
    {
        private:
            Pimp<UnpackagedRepository> _imp;

            void _add_metadata_keys() const;

        protected:
            void need_keys_added() const override;

        public:
            UnpackagedRepository(
                    const RepositoryName &,
                    const unpackaged_repositories::UnpackagedRepositoryParams &);

            ~UnpackagedRepository() override;

            void invalidate() override;

            std::shared_ptr<const PackageIDSequence> package_ids(
                    const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const QualifiedPackageNameSet> package_names(
                    const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const CategoryNamePartSet> category_names(const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_package_named(const QualifiedPackageName &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool has_category_named(const CategoryNamePart &, const RepositoryContentMayExcludes &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool some_ids_might_support_action(const SupportsActionTestBase &) const override;
            bool some_ids_might_not_be_masked() const override;

            const bool is_unimportant() const override;

            bool sync(const std::string &, const std::string &, const std::shared_ptr<OutputManager> &) const override;

            const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                    const std::string &) const override;

            /* Keys */

            const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > location_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > installed_root_key() const override;
            const std::shared_ptr<const MetadataCollectionKey<Map<std::string, std::string> > > sync_host_key() const override;

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

            HookResult perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> &)
                override PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
