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

#include "cmd_print_id_size.hh"
#include "command_command_line.hh"
#include "size_common.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDSizeCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-id-size";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints the size of files installed by a package.";
        }

        virtual std::string app_description() const
        {
            return "Prints the size of files installed by a package. No formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        PrintIDSizeCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true)
        {
            add_usage_line("spec");
        }
    };
}

int
PrintIDSizeCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDSizeCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_SIZE_OPTIONS", "CAVE_PRINT_ID_SIZE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("print-id-size takes exactly one parameter");

    return size_common(env, false, *cmdline.begin_parameters(), cmdline.a_all.specified(), cmdline.a_best.specified());
}

std::shared_ptr<args::ArgsHandler>
PrintIDSizeCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDSizeCommandLine>();
}

