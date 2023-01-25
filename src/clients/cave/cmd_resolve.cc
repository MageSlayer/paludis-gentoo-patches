/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_resolve.hh"
#include "resolve_cmdline.hh"
#include "resolve_common.hh"

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;

namespace
{
    struct ResolveCommandLine :
        CaveCommandCommandLine
    {
        ResolveCommandLineResolutionOptions resolution_options;
        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineDisplayOptions display_options;
        ResolveCommandLineGraphJobsOptions graph_jobs_options;
        ResolveCommandLineProgramOptions program_options;

        ResolveCommandLine() :
            resolution_options(this),
            execution_options(this),
            display_options(this),
            graph_jobs_options(this),
            program_options(this)
        {
            add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] spec ...");
            add_usage_line("[ -x|--execute ] [ -z|--lazy or -c|--complete or -e|--everything ] set");
            add_usage_line("[ -x|--execute ] !spec ...");
        }

        std::string app_name() const override
        {
            return "cave resolve";
        }

        std::string app_synopsis() const override
        {
            return "Display how to resolve one or more targets, and possibly then "
                "perform that resolution.";
        }

        std::string app_description() const override
        {
            return "Displays how to resolve one or more targets. If instructed, then "
                "executes the relevant install and uninstall actions to perform that "
                "resolution.";
        }
    };
}

int
ResolveCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ResolveCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.resolution_options.apply_shortcuts();
    cmdline.resolution_options.verify(env);

    std::shared_ptr<Sequence<std::pair<std::string, std::string> > > targets(std::make_shared<Sequence<std::pair<std::string, std::string> >>());
    for (const auto & target : cmdline.parameters())
        targets->push_back(std::make_pair(target, ""));

    return resolve_common(env, cmdline.resolution_options, cmdline.execution_options, cmdline.display_options,
            cmdline.graph_jobs_options, cmdline.program_options, nullptr, targets, nullptr, false);
}

std::shared_ptr<args::ArgsHandler>
ResolveCommand::make_doc_cmdline()
{
    return std::make_shared<ResolveCommandLine>();
}

CommandImportance
ResolveCommand::importance() const
{
    return ci_core;
}

