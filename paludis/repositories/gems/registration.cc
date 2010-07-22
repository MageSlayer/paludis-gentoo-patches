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

#include <paludis/repository_factory.hh>
#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/exceptions.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/destringify.hh>
#include <paludis/distribution.hh>
#include <paludis/environment.hh>
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
    void register_repositories<repository_groups::gems>(const repository_groups::gems * const,
            RepositoryFactory * const factory)
    {
        std::shared_ptr<Set<std::string> > gems_formats(new Set<std::string>);
        gems_formats->insert("gems");

        factory->add_repository_format(
                gems_formats,
                GemsRepository::repository_factory_name,
                &generic_importance,
                GemsRepository::repository_factory_create,
                GemsRepository::repository_factory_dependencies
                );

        std::shared_ptr<Set<std::string> > installed_gems_formats(new Set<std::string>);
        installed_gems_formats->insert("installed_gems");
        installed_gems_formats->insert("installed-gems");

        factory->add_repository_format(
                installed_gems_formats,
                InstalledGemsRepository::repository_factory_name,
                &generic_importance,
                InstalledGemsRepository::repository_factory_create,
                InstalledGemsRepository::repository_factory_dependencies
                );
    }
}

