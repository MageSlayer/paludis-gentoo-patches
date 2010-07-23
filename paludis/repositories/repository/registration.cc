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

#include <paludis/repository_factory.hh>
#include <paludis/repositories/repository/repository_repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include "config.h"

using namespace paludis;
using namespace paludis::repository_repository;

namespace
{
    int generic_importance(const Environment * const, const std::function<std::string (const std::string &)> & f)
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
    void register_repositories<repository_groups::repository>(const repository_groups::repository * const,
            RepositoryFactory * const factory)
    {
        std::shared_ptr<Set<std::string> > repository_formats(std::make_shared<Set<std::string>>());
        repository_formats->insert("repository");

        factory->add_repository_format(repository_formats,
                &RepositoryRepository::repository_factory_name,
                &generic_importance,
                &RepositoryRepository::repository_factory_create,
                &RepositoryRepository::repository_factory_dependencies
                );
    }
}

