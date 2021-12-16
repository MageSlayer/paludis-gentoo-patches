/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_INSTALLED_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_INSTALLED_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/repositories/e/e_repository_id.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_builddir> builddir;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_root> root;
    }

    namespace erepository
    {
        struct EInstalledRepositoryParams
        {
            NamedValue<n::builddir, FSPath> builddir;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::root, FSPath> root;
        };

        class EInstalledRepository :
            public Repository,
            public RepositoryEnvironmentVariableInterface,
            public RepositoryDestinationInterface
        {
            private:
                Pimp<EInstalledRepository> _imp;

            protected:
                EInstalledRepository(const EInstalledRepositoryParams &, const RepositoryName &, const RepositoryCapabilities &);
                ~EInstalledRepository() override;

                std::string snoop_variable_from_environment_file(
                        const FSPath &,
                        const std::string & var) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

            public:
                /* RepositoryEnvironmentVariableInterface */

                std::string get_environment_variable(
                        const std::shared_ptr<const PackageID> & for_package,
                        const std::string & var) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                /* RepositoryDestinationInterface */

                bool is_suitable_destination_for(const std::shared_ptr<const PackageID> &) const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                bool want_pre_post_phases() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                /* Repository */

                std::shared_ptr<const CategoryNamePartSet> unimportant_category_names(
                        const RepositoryContentMayExcludes &) const override;

                const bool is_unimportant() const override;

                bool some_ids_might_support_action(const SupportsActionTestBase &) const override;

                bool some_ids_might_not_be_masked() const override;

                HookResult perform_hook(
                        const Hook & hook,
                        const std::shared_ptr<OutputManager> &)
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                bool sync(
                        const std::string &,
                        const std::string &,
                        const std::shared_ptr<OutputManager> &) const override;

                const std::shared_ptr<const Set<std::string> > maybe_expand_licence_nonrecursively(
                        const std::string &) const override;

                ///\name For use by EInstalledRepositoryID
                ///\{

                virtual void perform_uninstall(
                        const std::shared_ptr<const erepository::ERepositoryID> & id,
                        const UninstallAction &) const = 0;

                virtual void perform_config(
                        const std::shared_ptr<const erepository::ERepositoryID> & id,
                        const ConfigAction &) const;

                virtual void perform_info(
                        const std::shared_ptr<const erepository::ERepositoryID> & id,
                        const InfoAction &) const;

                ///\}

                ///\name Set methods
                ///\{

                void populate_sets() const override;

                ///\}

                virtual void perform_updates() = 0;
        };
    }
}

#endif
