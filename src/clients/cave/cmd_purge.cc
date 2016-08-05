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

#include "cmd_purge.hh"
#include "resolve_cmdline.hh"
#include "resolve_common.hh"
#include "exceptions.hh"

#include <paludis/args/do_help.hh>
#include <paludis/util/stringify.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    struct PurgeCommandLine :
        CaveCommandCommandLine
    {
        std::shared_ptr<ResolveCommandLineResolutionOptions> resolution_options;
        std::shared_ptr<ResolveCommandLineExecutionOptions> execution_options;
        std::shared_ptr<ResolveCommandLineDisplayOptions> display_options;
        std::shared_ptr<ResolveCommandLineGraphJobsOptions> graph_jobs_options;
        std::shared_ptr<ResolveCommandLineProgramOptions> program_options;

        PurgeCommandLine(const bool for_docs) :
            resolution_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineResolutionOptions>(this)),
            execution_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineExecutionOptions>(this)),
            display_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineDisplayOptions>(this)),
            graph_jobs_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineGraphJobsOptions>(this)),
            program_options(for_docs ? nullptr : std::make_shared<ResolveCommandLineProgramOptions>(this))
        {
            add_usage_line("[ -x|--execute ]");
            add_note("All options available for 'cave resolve' are also permitted. See 'man cave-resolve' for details.");
        }

        std::string app_name() const override
        {
            return "cave purge";
        }

        std::string app_synopsis() const override
        {
            return "Uninstall unused packages.";
        }

        std::string app_description() const override
        {
            return "Uninstalls any package that is not either in 'world' or a dependency of a package "
                "in 'world'.";
        }
    };
}

int
PurgeCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PurgeCommandLine cmdline(false);
    cmdline.run(args, "CAVE", "CAVE_PURGE_OPTIONS", "CAVE_PURGE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("purge takes no parameters");

    cmdline.resolution_options->apply_shortcuts();
    cmdline.resolution_options->verify(env);

    cmdline.resolution_options->a_purge.set_specified(args::aos_weak);
    cmdline.resolution_options->a_purge.add_argument("*/*");

    return resolve_common(env, *cmdline.resolution_options, *cmdline.execution_options, *cmdline.display_options,
            *cmdline.graph_jobs_options, *cmdline.program_options, nullptr, nullptr, nullptr, true);
}

std::shared_ptr<args::ArgsHandler>
PurgeCommand::make_doc_cmdline()
{
    return std::make_shared<PurgeCommandLine>(true);
}

CommandImportance
PurgeCommand::importance() const
{
    return ci_core;
}

