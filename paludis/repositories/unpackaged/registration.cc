/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/attributes.hh>
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include <paludis/repository_factory.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include "config.h"

using namespace paludis;

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
    void register_repositories<repository_groups::unpackaged>(const repository_groups::unpackaged * const,
            RepositoryFactory * const factory)
    {
        std::shared_ptr<Set<std::string> > unpackaged_formats(std::make_shared<Set<std::string>>());
        unpackaged_formats->insert("unpackaged");

        factory->add_repository_format(
                unpackaged_formats,
                &UnpackagedRepository::repository_factory_name,
                &generic_importance,
                &UnpackagedRepository::repository_factory_create,
                &UnpackagedRepository::repository_factory_dependencies
                );

        std::shared_ptr<Set<std::string> > installed_unpackaged_formats(std::make_shared<Set<std::string>>());
        installed_unpackaged_formats->insert("installed_unpackaged");
        installed_unpackaged_formats->insert("installed-unpackaged");

        factory->add_repository_format(
                installed_unpackaged_formats,
                &InstalledUnpackagedRepository::repository_factory_name,
                &generic_importance,
                &InstalledUnpackagedRepository::repository_factory_create,
                &InstalledUnpackagedRepository::repository_factory_dependencies
                );
    }
}

