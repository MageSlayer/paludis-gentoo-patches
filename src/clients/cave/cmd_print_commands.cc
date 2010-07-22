/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include "cmd_print_commands.hh"
#include "command_factory.hh"
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintCommandsCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-commands";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of known cave commands.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of known cave commands. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_general;
        args::SwitchArg a_all;

        PrintCommandsCommandLine() :
            g_general(main_options_section(), "General Options", "General Options"),
            a_all(&g_general, "all", 'a', "Print all available commands.", false)
        {
            add_usage_line("[-a|--all]");
        }
    };
}

int
PrintCommandsCommand::run(
        const std::shared_ptr<Environment> &,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintCommandsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_COMMANDS_OPTIONS", "CAVE_PRINT_COMMANDS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-commands takes no parameters");

    for (CommandFactory::ConstIterator cmd(CommandFactory::get_instance()->begin()), cmd_end(CommandFactory::get_instance()->end()) ;
            cmd != cmd_end ; ++cmd)
    {
        std::shared_ptr<Command> instance(CommandFactory::get_instance()->create(*cmd));

        if (! cmdline.a_all.specified() && ! instance->important())
            continue;

        cout << stringify(*cmd) << endl;
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintCommandsCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintCommandsCommandLine);
}

