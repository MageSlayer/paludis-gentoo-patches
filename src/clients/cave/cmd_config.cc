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

#include "cmd_config.hh"
#include "exceptions.hh"
#include "parse_spec_with_nice_error.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/hook.hh>
#include <paludis/output_manager_from_environment.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct ConfigCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave config";
        }

        std::string app_synopsis() const override
        {
            return "Perform post-install configuration on a package.";
        }

        std::string app_description() const override
        {
            return "Perform post-install configuration on a package. Note that most packages do "
                "not provide a post-install configuration script.";
        }

        ConfigCommandLine()
        {
            add_usage_line("spec");
        }
    };
}

int
ConfigCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ConfigCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_CONFIG_OPTIONS", "CAVE_CONFIG_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != cmdline.parameters().size())
        throw args::DoHelp("config takes exactly one parameter");

    PackageDepSpec spec(parse_spec_with_nice_error(*cmdline.begin_parameters(), env.get(), { }, filter::SupportsAction<ConfigAction>()));
    const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                generator::Matches(spec, nullptr, { }) | filter::SupportsAction<ConfigAction>())]);
    if (ids->empty())
        nothing_matching_error(env.get(), *cmdline.begin_parameters(), filter::SupportsAction<ConfigAction>());
    else if (1 != std::distance(ids->begin(), ids->end()))
        throw BeMoreSpecific(spec, ids);
    const std::shared_ptr<const PackageID> id(*ids->begin());

    OutputManagerFromEnvironment output_manager_holder(env.get(), id, oe_exclusive, ClientOutputFeatures());
    ConfigAction action(make_named_values<ConfigActionOptions>(
                n::make_output_manager() = std::ref(output_manager_holder)
                ));
    id->perform_action(action);

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
ConfigCommand::make_doc_cmdline()
{
    return std::make_shared<ConfigCommandLine>();
}

CommandImportance
ConfigCommand::importance() const
{
    return ci_supplemental;
}

