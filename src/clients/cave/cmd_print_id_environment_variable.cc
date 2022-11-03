/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2010 David Leverton
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

#include "cmd_print_id_environment_variable.hh"
#include "format_string.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/map.hh>
#include <paludis/environment.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <iostream>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct PrintIDEnvironmentVariableCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-id-environment-variable";
        }

        std::string app_synopsis() const override
        {
            return "Prints ID environment variables.";
        }

        std::string app_description() const override
        {
            return "Prints ID environment variables. No formatting is used, "
                "making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        args::ArgsGroup g_filters;
        args::StringSequenceArg a_variable_name;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintIDEnvironmentVariableCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true),
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once."),
            a_variable_name(&g_filters, "variable-name", '\0', "Show this environment variable. If specified more than once, "
                    "any name match is accepted.  Must be specified at least once."),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", '\0', "Select the output format. Special tokens recognised are "
                    "%n for variable name, %v for value, "
                    "\\n for newline, \\t for tab. Default is '%n=%v\\n'.")
        {
            a_format.set_argument("%n=%v\\n");
            add_usage_line("--variable-name var [ --format format ] spec");
        }
    };

    void do_one_var(
            const Environment * const env,
            const PackageDepSpec & s,
            const std::shared_ptr<const PackageID> & id,
            const std::string & n,
            const PrintIDEnvironmentVariableCommandLine & cmdline
            )
    {
        auto repo(env->fetch_repository(id->repository_name()));

        if (nullptr != repo->environment_variable_interface())
        {
            std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
            m->insert('n', n);
            m->insert('v', repo->environment_variable_interface()->get_environment_variable(id, n));

            cout << format_string(cmdline.a_format.argument(), m);
        }
        else
            throw BadIDForCommand(s, id, "does not support getting environment variables");
    }
}

int
PrintIDEnvironmentVariableCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDEnvironmentVariableCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_ENVIRONMENT_VARIABLE_OPTIONS", "CAVE_PRINT_ID_ENVIRONMENT_VARIABLE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != cmdline.parameters().size())
        throw args::DoHelp("print-id-environment-variable takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { updso_allow_wildcards }));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, nullptr, { }))]);

    if (entries->empty())
        throw NothingMatching(spec);

    if ((! cmdline.a_best.specified()) && (! cmdline.a_all.specified())
            && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    if (! cmdline.a_variable_name.specified())
        throw args::DoHelp("print-id-enviroment-variable requires at least one variable name");

    for (auto i(cmdline.a_best.specified() ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
        for (const std::string & var_name : cmdline.a_variable_name.args())
            do_one_var(env.get(), spec, *i, var_name, cmdline);

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDEnvironmentVariableCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDEnvironmentVariableCommandLine>();
}

CommandImportance
PrintIDEnvironmentVariableCommand::importance() const
{
    return ci_scripting;
}

