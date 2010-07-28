/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/exndbam_repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/destringify.hh>
#include "config.h"

using namespace paludis;

namespace
{
    int generic_importance(const Environment * const, const std::function<std::string (const std::string &)> & f)
    {
        if (! f("importance").empty())
            return destringify<int>(f("importance"));
        else if (! f("master_repository").empty())
            return 10;
        else
            return 1;
    }

    std::shared_ptr<Repository> deprecated_create(
            Environment * const env,
            const std::function<std::string (const std::string &)> & f)
    {
        Log::get_instance()->message("e.format.deprecated", ll_warning, lc_context)
            << "Format '" << f("format") << "' in '" << f("repo_file") << "' is deprecated, use format='e' instead";
        return ERepository::repository_factory_create(env, f);
    }
}

namespace paludis
{
    namespace repository_groups
    {
        REPOSITORY_GROUPS_DECLS;
    }

    template <>
    void register_repositories<repository_groups::e>(const repository_groups::e * const,
            RepositoryFactory * const factory)
    {
        std::shared_ptr<Set<std::string> > ebuild_formats(std::make_shared<Set<std::string>>());
        ebuild_formats->insert("e");

        factory->add_repository_format(
                ebuild_formats,
                &ERepository::repository_factory_name,
                &generic_importance,
                &ERepository::repository_factory_create,
                &ERepository::repository_factory_dependencies
                );

        std::shared_ptr<Set<std::string> > deprecated_ebuild_formats(std::make_shared<Set<std::string>>());
        deprecated_ebuild_formats->insert("ebuild");
        deprecated_ebuild_formats->insert("exheres");

        factory->add_repository_format(
                deprecated_ebuild_formats,
                &ERepository::repository_factory_name,
                &generic_importance,
                &deprecated_create,
                &ERepository::repository_factory_dependencies
                );

        std::shared_ptr<Set<std::string> > vdb_formats(std::make_shared<Set<std::string>>());
        vdb_formats->insert("vdb");

        factory->add_repository_format(
                vdb_formats,
                &VDBRepository::repository_factory_name,
                &generic_importance,
                &VDBRepository::repository_factory_create,
                &VDBRepository::repository_factory_dependencies
                );

        std::shared_ptr<Set<std::string> > exndbam_formats(std::make_shared<Set<std::string>>());
        exndbam_formats->insert("exndbam");

        factory->add_repository_format(
                exndbam_formats,
                &ExndbamRepository::repository_factory_name,
                &generic_importance,
                &ExndbamRepository::repository_factory_create,
                &ExndbamRepository::repository_factory_dependencies
                );
    }
}

