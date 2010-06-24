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

#include "cmd_purge.hh"
#include "cmd_resolve_cmdline.hh"
#include "resolve_common.hh"
#include "exceptions.hh"

#include <paludis/util/make_shared_ptr.hh>
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
        std::tr1::shared_ptr<ResolveCommandLineResolutionOptions> resolution_options;
        std::tr1::shared_ptr<ResolveCommandLineExecutionOptions> execution_options;
        std::tr1::shared_ptr<ResolveCommandLineDisplayOptions> display_options;
        std::tr1::shared_ptr<ResolveCommandLineProgramOptions> program_options;

        PurgeCommandLine(const bool for_docs) :
            resolution_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineResolutionOptions(this))),
            execution_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineExecutionOptions(this))),
            display_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineDisplayOptions(this))),
            program_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineProgramOptions(this)))
        {
            add_usage_line("[ -x|--execute ]");
            add_note("All options available for 'cave resolve' are also permitted. See 'man cave-resolve' for details.");
        }

        std::string app_name() const
        {
            return "cave purge";
        }

        std::string app_synopsis() const
        {
            return "Uninstall unused packages.";
        }

        std::string app_description() const
        {
            return "Uninstalls any package that is not either in 'world' or a dependency of a package "
                "in 'world'.";
        }
    };
}

bool
PurgeCommand::important() const
{
    return true;
}

int
PurgeCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    PurgeCommandLine cmdline(false);
    cmdline.run(args, "CAVE", "CAVE_PURGE_OPTIONS", "CAVE_PURGE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.resolution_options->apply_shortcuts();
    cmdline.resolution_options->verify(env);

    cmdline.resolution_options->a_purge.set_specified(true);
    cmdline.resolution_options->a_purge.add_argument("*/*");

    return resolve_common(env, *cmdline.resolution_options, *cmdline.execution_options, *cmdline.display_options,
            *cmdline.program_options, make_null_shared_ptr(), make_null_shared_ptr(), true);
}

std::tr1::shared_ptr<args::ArgsHandler>
PurgeCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PurgeCommandLine(true));
}

