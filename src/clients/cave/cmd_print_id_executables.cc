/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "cmd_print_id_executables.hh"
#include "command_command_line.hh"
#include "executables_common.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <memory>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDExecutablesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-id-executables";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of executables belonging to an ID.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of executables belonging to an ID. "
                "No formating is used, making the script suitable for parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        PrintIDExecutablesCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true)
        {
            add_usage_line("spec");
        }
    };

    void print_fsentry(const FSPath & e)
    {
        cout << e << endl;
    }
}

int
PrintIDExecutablesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDExecutablesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_EXECUTABLES_OPTIONS", "CAVE_PRINT_ID_EXECUTABLES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-id-executables takes exactly one parameter");

    return executables_common(env, *cmdline.begin_parameters(), &print_fsentry, cmdline.a_all.specified(),
            cmdline.a_best.specified());
}

std::shared_ptr<args::ArgsHandler>
PrintIDExecutablesCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDExecutablesCommandLine>();
}

