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

#include "cmd_uninstall.hh"
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
    struct UninstallCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_target_options;
        args::SwitchArg a_all_versions;

        std::tr1::shared_ptr<ResolveCommandLineResolutionOptions> resolution_options;
        std::tr1::shared_ptr<ResolveCommandLineExecutionOptions> execution_options;
        std::tr1::shared_ptr<ResolveCommandLineDisplayOptions> display_options;
        std::tr1::shared_ptr<ResolveCommandLineProgramOptions> program_options;

        UninstallCommandLine(const bool for_docs) :
            g_target_options(main_options_section(), "Target options", "Target options"),
            a_all_versions(&g_target_options, "all-versions", 'a', "If a supplied spec matches multiple versions, "
                    "uninstall all versions rather than erroring", true),
            resolution_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineResolutionOptions(this))),
            execution_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineExecutionOptions(this))),
            display_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineDisplayOptions(this))),
            program_options(for_docs ? make_null_shared_ptr() : make_shared_ptr(new ResolveCommandLineProgramOptions(this)))
        {
            add_usage_line("[ -x|--execute ] [ --uninstalls-may-break */* ] [ --remove-if-dependent */* ] spec ...");
            add_note("All options available for 'cave resolve' are also permitted. See 'man cave-resolve' for details.");
        }

        std::string app_name() const
        {
            return "cave uninstall";
        }

        std::string app_synopsis() const
        {
            return "Uninstall one or more packages.";
        }

        std::string app_description() const
        {
            return "Uninstalls one or more packages. Note that 'cave uninstall' simply rewrites the supplied "
                "dependency specifications and then uses 'cave resolve' to do the work; 'cave uninstall foo' is "
                "the same as 'cave resolve !foo'.";
        }
    };
}

bool
UninstallCommand::important() const
{
    return true;
}

int
UninstallCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    UninstallCommandLine cmdline(false);
    cmdline.run(args, "CAVE", "CAVE_UNINSTALL_OPTIONS", "CAVE_UNINSTALL_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.resolution_options->apply_shortcuts();
    cmdline.resolution_options->verify(env);

    std::tr1::shared_ptr<Sequence<std::string> > targets(new Sequence<std::string>);
    for (UninstallCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        PackageDepSpec spec(parse_user_package_dep_spec(*p, env.get(), UserPackageDepSpecOptions()));
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Matches(spec, MatchPackageOptions()) | filter::SupportsAction<UninstallAction>())]);
        if (ids->empty())
            throw NothingMatching(spec);
        else if (1 != std::distance(ids->begin(), ids->end()) && ! cmdline.a_all_versions.specified())
            throw BeMoreSpecific(spec, ids);
        else
        {
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                std::string target("!" + stringify((*i)->name()));
                if ((*i)->slot_key())
                    target.append(":" + stringify((*i)->slot_key()->value()));
                targets->push_back(target);
            }
        }
    }

    return resolve_common(env, *cmdline.resolution_options, *cmdline.execution_options, *cmdline.display_options,
            *cmdline.program_options, make_null_shared_ptr(), targets);
}

std::tr1::shared_ptr<args::ArgsHandler>
UninstallCommand::make_doc_cmdline()
{
    return make_shared_ptr(new UninstallCommandLine(true));
}

