/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/repository_factory.hh>
#include <paludis/repositories/accounts/accounts_repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include "config.h"

using namespace paludis;
using namespace paludis::accounts_repository;

namespace
{
    int generic_importance(const Environment * const, const std::tr1::function<std::string (const std::string &)> & f)
    {
        if (! f("importance").empty())
            return destringify<int>(f("importance"));
        else
            return 1;
    }
}

namespace paludis
{
    namespace repository_groups
    {
        REPOSITORY_GROUPS_DECLS;
    }

    template <>
    void register_repositories<repository_groups::accounts>(const repository_groups::accounts * const,
            RepositoryFactory * const factory)
    {
        std::tr1::shared_ptr<Set<std::string> > accounts_formats(new Set<std::string>);
        accounts_formats->insert("accounts");

        factory->add_repository_format(accounts_formats,
                &AccountsRepository::repository_factory_name,
                &generic_importance,
                &AccountsRepository::repository_factory_create,
                &AccountsRepository::repository_factory_dependencies
                );

        std::tr1::shared_ptr<Set<std::string> > installed_accounts_formats(new Set<std::string>);
        installed_accounts_formats->insert("installed_accounts");
        installed_accounts_formats->insert("installed-accounts");

        factory->add_repository_format(installed_accounts_formats,
                &AccountsRepository::repository_factory_installed_name,
                &generic_importance,
                &AccountsRepository::repository_factory_installed_create,
                &AccountsRepository::repository_factory_installed_dependencies
                );
    }
}

