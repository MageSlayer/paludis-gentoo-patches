/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
        struct builddir;
        struct environment;
        struct root;
    }

    namespace erepository
    {
        struct EInstalledRepositoryParams
        {
            NamedValue<n::builddir, FSEntry> builddir;
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::root, FSEntry> root;
        };

        class EInstalledRepository :
            public Repository,
            public RepositoryEnvironmentVariableInterface,
            public RepositoryDestinationInterface,
            private PrivateImplementationPattern<EInstalledRepository>
        {
            private:
                PrivateImplementationPattern<EInstalledRepository>::ImpPtr & _imp;

            protected:
                EInstalledRepository(const EInstalledRepositoryParams &, const RepositoryName &, const RepositoryCapabilities &);
                ~EInstalledRepository();

            public:
                /* RepositoryEnvironmentVariableInterface */

                virtual std::string get_environment_variable(
                        const std::tr1::shared_ptr<const PackageID> & for_package,
                        const std::string & var) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                /* RepositoryDestinationInterface */

                virtual bool is_suitable_destination_for(const PackageID &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool is_default_destination() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool want_pre_post_phases() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                /* Repository */

                virtual std::tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const;

                virtual bool some_ids_might_support_action(const SupportsActionTestBase &) const;

                HookResult perform_hook(const Hook & hook)
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                ///\name For use by EInstalledRepositoryID
                ///\{

                virtual void perform_uninstall(
                        const std::tr1::shared_ptr<const erepository::ERepositoryID> & id,
                        const UninstallAction &) const = 0;

                virtual void perform_config(
                        const std::tr1::shared_ptr<const erepository::ERepositoryID> & id,
                        const ConfigAction &) const;

                virtual void perform_info(
                        const std::tr1::shared_ptr<const erepository::ERepositoryID> & id,
                        const InfoAction &) const;

                ///\}

                ///\name Set methods
                ///\{

                virtual void populate_sets() const;

                ///\}
        };
    }
}

#endif
