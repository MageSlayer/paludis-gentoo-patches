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

#include "cmd_fix_cache.hh"
#include "colours.hh"
#include "format_user_config.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/stringify.hh>

#include <iostream>
#include <set>
#include <algorithm>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
#include "cmd_fix_cache-fmt.hh"

    struct FixCacheCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave fix-cache";
        }

        std::string app_synopsis() const override
        {
            return "Fix on-disk caches.";
        }

        std::string app_description() const override
        {
            return "Fixes on-disk caches. Normally this is handled automatically when needed (e.g. after a sync, "
                "or when a package is installed), but a forced fix-cache is required after a different package "
                "manager has been used, or if a repository is modified without a sync.";
        }

        args::ArgsGroup g_repositories;
        args::StringSetArg a_repository;
        args::SwitchArg a_installable;
        args::SwitchArg a_installed;

        FixCacheCommandLine() :
            g_repositories(main_options_section(), "Repositories", "Select repositories whose cache is to be "
                    "regenerated. If none of these restrictions are specified, all repositories are selected. "
                    "Otherwise, only repositories matching any of these restrictions are regenerated."),
            a_repository(&g_repositories, "repository", 'r', "Select the repository with the specified name. May "
                    "be specified multiple times."),
            a_installable(&g_repositories, "installable", 'i', "Select all installable repositories.", true),
            a_installed(&g_repositories, "installed", 'I', "Select all installed repositories", true)
        {
        }
    };
}

int
FixCacheCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    FixCacheCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_FIX_CACHE_OPTIONS", "CAVE_FIX_CACHE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("fix-cache takes no parameters");

    std::set<RepositoryName> repository_names;

    bool all(true);

    if (cmdline.a_repository.specified())
    {
        all = false;
        for (args::StringSetArg::ConstIterator p(cmdline.a_repository.begin_args()),
                p_end(cmdline.a_repository.end_args()) ;
                p != p_end ; ++p)
            repository_names.insert(RepositoryName(*p));
    }

    if (cmdline.a_installable.specified())
    {
        all = false;
        for (const auto & repository : env->repositories())
            if (! repository->installed_root_key())
                repository_names.insert(repository->name());
    }

    if (cmdline.a_installed.specified())
    {
        all = false;
        for (const auto & repository : env->repositories())
            if (repository->installed_root_key())
                repository_names.insert(repository->name());
    }

    if (all)
        for (const auto & repository : env->repositories())
            repository_names.insert(repository->name());

    for (const auto & repository_name : repository_names)
    {
        cout << fuc(fs_fixing(), fv<'s'>(stringify(repository_name)));
        const std::shared_ptr<Repository> repo(env->fetch_repository(repository_name));
        repo->regenerate_cache();
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
FixCacheCommand::make_doc_cmdline()
{
    return std::make_shared<FixCacheCommandLine>();
}
CommandImportance
FixCacheCommand::importance() const
{
    return ci_supplemental;
}

