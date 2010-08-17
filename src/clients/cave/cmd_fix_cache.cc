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

#include "cmd_fix_cache.hh"
#include "formats.hh"
#include "format_user_config.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/indirect_iterator-impl.hh>

#include <iostream>
#include <set>
#include <algorithm>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_fix_cache-fmt.hh"

    struct FixCacheCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave fix-cache";
        }

        virtual std::string app_synopsis() const
        {
            return "Fix on-disk caches.";
        }

        virtual std::string app_description() const
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
        for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
                r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories());
                r != r_end; ++r)
            if (! r->installed_root_key())
                repository_names.insert(r->name());
    }

    if (cmdline.a_installed.specified())
    {
        all = false;
        for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
                r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories());
                r != r_end; ++r)
            if (r->installed_root_key())
                repository_names.insert(r->name());
    }

    if (all)
        for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
                r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories());
                r != r_end; ++r)
            repository_names.insert(r->name());

    for (std::set<RepositoryName>::const_iterator r(repository_names.begin()), r_end(repository_names.end()) ;
            r != r_end; ++r)
    {
        cout << fuc(fs_fixing(), fv<'s'>(stringify(*r)));
        const std::shared_ptr<Repository> repo(env->package_database()->fetch_repository(*r));
        repo->regenerate_cache();
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
FixCacheCommand::make_doc_cmdline()
{
    return std::make_shared<FixCacheCommandLine>();
}
