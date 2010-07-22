/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#include "command_line.hh"
#include "fix_linkage.hh"

#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>

#include <paludis/about.hh>
#include <paludis/environment_factory.hh>
#include <paludis/args/do_help.hh>

#include <src/output/colour.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;

namespace
{
    struct DoVersion
    {
    };

    void display_version()
    {
        std::cout << "reconcilio, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
        if (! std::string(PALUDIS_GIT_HEAD).empty())
            std::cout << " git " << PALUDIS_GIT_HEAD;
        std::cout << std::endl;
    }
}

int
main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("RECONCILIO_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "reconcilio", "RECONCILIO_OPTIONS", "RECONCILIO_CMDLINE");

        if (CommandLine::get_instance()->a_help.specified())
            throw args::DoHelp();
        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        Log::get_instance()->set_program_name(argv[0]);
        if (CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());

        set_use_colour(! CommandLine::get_instance()->a_no_colour.specified());
        set_force_colour(CommandLine::get_instance()->a_force_colour.specified());

        std::string paludis_command("paludis");

        if (CommandLine::get_instance()->a_environment.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_environment.long_name() + " " +
                    CommandLine::get_instance()->a_environment.argument());

        paludis_command.append(" --" + CommandLine::get_instance()->a_log_level.long_name() + " " +
                CommandLine::get_instance()->a_log_level.argument());

        if (CommandLine::get_instance()->a_resume_command_template.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_resume_command_template.long_name() + " "
                    + CommandLine::get_instance()->a_resume_command_template.argument());

        if (CommandLine::get_instance()->a_no_color.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_no_color.long_name());

        if (CommandLine::get_instance()->a_force_color.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_force_color.long_name());

        paludis_command.append(CommandLine::get_instance()->install_args.paludis_command_fragment());
        paludis_command.append(CommandLine::get_instance()->dl_args.paludis_command_fragment());

        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));
        env->set_paludis_command(paludis_command);

        return do_fix_linkage(env);
    }

    catch (const DoVersion &)
    {
        display_version();
        std::cout << std::endl;
        std::cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << std::endl;
        std::cout << "are welcome to redistribute it under the terms of the GNU General Public" << std::endl;
        std::cout << "License, version 2." << std::endl;

        return EXIT_SUCCESS;
    }

    catch (const args::ArgsError & e)
    {
        std::cerr << "Usage error: " << e.message() << std::endl;
        std::cerr << "Try " << argv[0] << " --help" << std::endl;
        return EXIT_FAILURE;
    }

    catch (const args::DoHelp & h)
    {
        if (h.message.empty())
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << std::endl;
            std::cout << *CommandLine::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            std::cerr << "Usage error: " << h.message << std::endl;
            std::cerr << "Try " << argv[0] << " --help" << std::endl;
            return EXIT_FAILURE;
        }
    }

    catch (const Exception & e)
    {
        std::cout << std::endl;
        std::cerr << "Unhandled exception:" << std::endl
                  << "  * " << e.backtrace("\n  * ")
                  << e.message() << " (" << e.what() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    catch (const std::exception & e)
    {
        std::cout << std::endl;
        std::cerr << "Unhandled exception:" << std::endl
                  << "  * " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    catch (...)
    {
        std::cout << std::endl;
        std::cerr << "Unhandled exception:" << std::endl
                  << "  * Unknown exception type. Ouch..." << std::endl;
        return EXIT_FAILURE;
    }
}

