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

#include "cmd_print_id_actions.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/set.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/stringify.hh>
#include <paludis/environment.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/action.hh>
#include <paludis/action_names.hh>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDActionsCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-id-actions";
        }

        std::string app_synopsis() const override
        {
            return "Prints ID actions.";
        }

        std::string app_description() const override
        {
            return "Prints ID actions. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        PrintIDActionsCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true)
        {
            add_usage_line("spec");
        }
    };

    template <typename Action_>
    void do_one_action(const std::shared_ptr<const PackageID> & id)
    {
        if (id->supports_action(SupportsActionTest<Action_>()))
            cout << ActionNames<Action_>::value << endl;
    }
}

int
PrintIDActionsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDActionsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_ACTIONS_OPTIONS", "CAVE_PRINT_ID_ACTIONS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.parameters().size() != 1)
        throw args::DoHelp("print-id-actions takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { updso_allow_wildcards }));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, nullptr, { }))]);

    if (entries->empty())
        throw NothingMatching(spec);

    if ((! cmdline.a_best.specified()) && (! cmdline.a_all.specified())
            && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    for (auto i(cmdline.a_best.specified() ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
    {
        do_one_action<ConfigAction>(*i);
        do_one_action<FetchAction>(*i);
        do_one_action<InfoAction>(*i);
        do_one_action<InstallAction>(*i);
        do_one_action<PretendAction>(*i);
        do_one_action<PretendFetchAction>(*i);
        do_one_action<UninstallAction>(*i);
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDActionsCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDActionsCommandLine>();
}

CommandImportance
PrintIDActionsCommand::importance() const
{
    return ci_scripting;
}

