/* vim: set sw=4 sts=4 et fdm=syntax: */

/*
 * Copyright (c) 2008 Saleem Abdulrasool
 *
 * This file is part of the Paludis package manager.  Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 55 Temple
 * Place, Suite 330, Boston, MA  02111-1308  USA
 */

#include "cmd_help.hh"
#include "colours.hh"

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/enum_iterator.hh>

#include "command_factory.hh"
#include "command_command_line.hh"

using namespace paludis;
using namespace cave;

using std::cout;

namespace
{
    struct HelpCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_general;
        args::SwitchArg a_all;

        HelpCommandLine() :
            g_general(main_options_section(), "General Options", "General Options"),
            a_all(&g_general, "all", 'a', "Print all available commands to standard output.", false)
        {
            add_usage_line("[-a|--all] [COMMAND]");
        }

        std::string app_name() const override
        {
            return "cave help";
        }

        std::string app_synopsis() const override
        {
            return "display help information";
        }

        std::string app_description() const override
        {
            return "Display help information for a particular command.";
        }
    };
}

int
HelpCommand::run(const std::shared_ptr<Environment> & env,
                 const std::shared_ptr< const Sequence<std::string> > & args)
{
    HelpCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_HELP_OPTIONS", "CAVE_HELP_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) > 1)
        throw args::DoHelp("help takes at most one parameter");

    if (cmdline.begin_parameters() == cmdline.end_parameters())
    {
        if (cmdline.a_all.specified())
            cout << "All available cave commands:" << std::endl;
        else
            cout << "The most commonly used cave commands (add --all for the rest) are:" << std::endl;

        for (EnumIterator<CommandImportance> e, e_end(cmdline.a_all.specified() ? last_ci : CommandImportance(ci_supplemental + 1)) ;
                e != e_end ; ++e)
        {
            if (*e == ci_ignore)
                continue;

            for (const auto & cmd : *CommandFactory::get_instance())
            {
                std::shared_ptr<Command> instance(CommandFactory::get_instance()->create(cmd));

                if (instance->importance() == *e)
                    cout << "    " << (*e == ci_core ? c::bold_blue().colour_string() : "") << std::left << std::setw(30) << cmd
                        << c::normal().colour_string() << " " << instance->make_doc_cmdline()->app_synopsis() << std::endl;
            }
        }

        return EXIT_SUCCESS;
    }
    else
    {
        std::shared_ptr< Sequence<std::string> > help(std::make_shared<Sequence<std::string>>());
        help->push_back("--help");

        return CommandFactory::get_instance()->create(*cmdline.begin_parameters())->run(env, help);
    }

    return EXIT_FAILURE;
}

std::shared_ptr<args::ArgsHandler>
HelpCommand::make_doc_cmdline()
{
    return std::make_shared<HelpCommandLine>();
}

CommandImportance
HelpCommand::importance() const
{
    return ci_supplemental;
}

