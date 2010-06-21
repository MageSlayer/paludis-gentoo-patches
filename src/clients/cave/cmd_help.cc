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

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <functional>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>

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

        virtual std::string app_name() const
        {
            return "cave help";
        }

        virtual std::string app_synopsis() const
        {
            return "display help information";
        }

        virtual std::string app_description() const
        {
            return "Display help information for a particular command.";
        }
    };

    struct IsImportantAndLonger :
        std::binary_function<std::string, std::string, bool>
    {
        bool
        operator() (const std::string & left, const std::string & right)
        {
            std::tr1::shared_ptr<Command> lhs(CommandFactory::get_instance()->create(left));
            std::tr1::shared_ptr<Command> rhs(CommandFactory::get_instance()->create(right));

            if (lhs->important() && rhs->important())
                return left.length() < right.length();
            else if (lhs->important() && ! rhs->important())
                return false;
            else if (! lhs->important() && rhs->important())
                return true;
            else
                return false;
        }
    };
}

bool
HelpCommand::important() const
{
    return true;
}

int
HelpCommand::run(const std::tr1::shared_ptr<Environment> & env,
                 const std::tr1::shared_ptr< const Sequence<std::string> > & args)
{
    HelpCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_HELP_OPTIONS", "CAVE_HELP_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.a_all.specified())
    {
        if (cmdline.begin_parameters() == cmdline.end_parameters())
        {
            cout << "All available cave commands:" << std::endl;
            std::copy(CommandFactory::get_instance()->begin(), CommandFactory::get_instance()->end(),
                    std::ostream_iterator<std::string>(cout, "\n"));
            return EXIT_SUCCESS;
        }
        else
        {
            throw args::DoHelp("--" + cmdline.a_all.long_name() + " takes no arguments");
        }
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) > 1)
        throw args::DoHelp("help takes at most one parameter");

    if (cmdline.begin_parameters() == cmdline.end_parameters())
    {
        size_t length(0);

        CommandFactory::ConstIterator name(std::max_element(CommandFactory::get_instance()->begin(),CommandFactory::get_instance()->end(),
                    IsImportantAndLonger()));
        if (name != CommandFactory::get_instance()->end())
            length = name->length();

        cout << "The most commonly used cave commands (add --all for the rest) are:" << std::endl;
        for (CommandFactory::ConstIterator cmd(CommandFactory::get_instance()->begin()), cmd_end(CommandFactory::get_instance()->end()) ;
                cmd != cmd_end ; ++cmd)
        {
            std::tr1::shared_ptr<Command> instance(CommandFactory::get_instance()->create(*cmd));

            if (instance->important())
                cout << "    " << *cmd << std::string(length - cmd->length(), ' ') << "        "
                     << instance->make_doc_cmdline()->app_synopsis() << std::endl;
        }

        return EXIT_SUCCESS;
    }
    else
    {
        std::tr1::shared_ptr< Sequence<std::string> > help(make_shared_ptr(new Sequence<std::string>));
        help->push_back("--help");

        return CommandFactory::get_instance()->create(*cmdline.begin_parameters())->run(env, help);
    }

    return EXIT_FAILURE;
}

std::tr1::shared_ptr<args::ArgsHandler>
HelpCommand::make_doc_cmdline()
{
    return make_shared_ptr(new HelpCommandLine);
}

