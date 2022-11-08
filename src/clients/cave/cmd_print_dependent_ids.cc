/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include "cmd_print_dependent_ids.hh"
#include "format_package_id.hh"
#include "parse_spec_with_nice_error.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/resolver/collect_depped_upon.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct PrintDependentIDsCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-dependent-ids";
        }

        std::string app_synopsis() const override
        {
            return "Prints a list of installed IDs that are dependent upon another installed ID.";
        }

        std::string app_description() const override
        {
            return "Prints a list of installed IDs that are dependent upon another installed ID. "
                "No formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintDependentIDsCommandLine() :
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", 'f', format_package_id_help)
        {
            add_usage_line("spec");
            a_format.set_argument("%F\\n");
        }
    };
}

int
PrintDependentIDsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintDependentIDsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_DEPENDENT_IDS_OPTIONS", "CAVE_PRINT_DEPENDENT_IDS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != cmdline.parameters().size())
        throw args::DoHelp("print-dependent-ids requires exactly one parameter");

    auto installed_filter(filter::InstalledAtRoot(env->system_root_key()->parse_value()));
    auto spec(parse_spec_with_nice_error(*cmdline.begin_parameters(), env.get(), { }, installed_filter));
    auto ids((*env)[selection::AllVersionsSorted(
                generator::Matches(spec, nullptr, { }) |
                installed_filter)]);

    if (ids->empty())
        throw NothingMatching(spec);
    else if (next(ids->begin()) != ids->end())
        throw BeMoreSpecific(spec, ids);

    auto installed_ids((*env)[selection::AllVersionsSorted(
                generator::All() |
                installed_filter)]);

    auto dependents(resolver::collect_dependents(env.get(), *ids->begin(), installed_ids));
    for (const auto & dependent : *dependents)
        cout << format_package_id(dependent, cmdline.a_format.argument());

    return dependents->empty() ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintDependentIDsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintDependentIDsCommandLine>();
}

CommandImportance
PrintDependentIDsCommand::importance() const
{
    return ci_scripting;
}

