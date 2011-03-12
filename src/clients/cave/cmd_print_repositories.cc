/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_repositories.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/metadata_key.hh>

#include <set>
#include <iostream>
#include <algorithm>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintRepositoriesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-repositories";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of repositories.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of repositories. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_repository_format;

        PrintRepositoriesCommandLine() :
            g_filters(main_options_section(), "Filters", "Filter the output."),
            a_repository_format(&g_filters, "format", '\0', "Show only repositories of a specific format")
        {
            add_usage_line("[ --format type ]");
        }
    };
}

int
PrintRepositoriesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintRepositoriesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_REPOSITORIES_OPTIONS", "CAVE_PRINT_REPOSITORIES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-repositories takes no parameters");

    std::set<RepositoryName> repository_names;

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories());
            r != r_end; ++r)
    {
        if (cmdline.a_repository_format.specified())
        {
            if (r->format_key())
            {
                if (cmdline.a_repository_format.end_args() == std::find(cmdline.a_repository_format.begin_args(), cmdline.a_repository_format.end_args(),
                        r->format_key()->value()))
                    continue;

                repository_names.insert(r->name());
            }
        }
        else
            repository_names.insert(r->name());
    }

    std::copy(repository_names.begin(), repository_names.end(), std::ostream_iterator<RepositoryName>(cout, "\n"));

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintRepositoriesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintRepositoriesCommandLine>();
}
CommandImportance
PrintRepositoriesCommand::importance() const
{
    return ci_scripting;
}

