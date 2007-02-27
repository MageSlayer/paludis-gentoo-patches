/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "do_search.hh"

#include <iostream>
#include <iomanip>
#include <cstdlib>

#include <paludis/about.hh>
#include <paludis/util/join.hh>
#include <paludis/environments/environment_maker.hh>

#include <src/output/colour.hh>
#include <src/common_args/do_help.hh>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct DoVersion
    {
    };

    void display_version()
    {
        cout << "inquisitio" << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
    }
}

int
main(int argc, char *argv[])
{
    Context context("In program " + join(argv, argv + argc, " ") + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "inquisitio", "INQUISITIO_OPTIONS", "INQUISITIO_CMDLINE");
        set_use_colour(! CommandLine::get_instance()->a_no_color.specified());

        if (CommandLine::get_instance()->a_help.specified())
            throw args::DoHelp();

        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());
        else
            Log::get_instance()->set_log_level(ll_qa);

        Log::get_instance()->set_program_name(argv[0]);

        /* need an action */
        if (1 < (CommandLine::get_instance()->a_search.specified() +
                    CommandLine::get_instance()->a_version.specified()))
            throw args::DoHelp("you should specify exactly one action");

        std::string paludis_command(argv[0]), env_spec;
        if (CommandLine::get_instance()->a_config_suffix.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "--config-suffix is deprecated, use --environment ':" +
                    CommandLine::get_instance()->a_config_suffix.argument() + "'");
            env_spec = ":" + CommandLine::get_instance()->a_config_suffix.argument();
            paludis_command.append(" --" + CommandLine::get_instance()->a_config_suffix.long_name() + " " +
                    CommandLine::get_instance()->a_config_suffix.argument());
        }
        else if (CommandLine::get_instance()->a_environment.specified())
        {
            paludis_command.append(" --" + CommandLine::get_instance()->a_environment.long_name() + " " +
                    CommandLine::get_instance()->a_config_suffix.argument());
            env_spec = CommandLine::get_instance()->a_environment.argument();
        }

        paludis_command.append(" --" + CommandLine::get_instance()->a_log_level.long_name() + " " +
                CommandLine::get_instance()->a_log_level.argument());

        if (CommandLine::get_instance()->a_no_color.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_no_color.long_name());

        std::tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(env_spec));
        env->set_paludis_command(paludis_command);

        if (CommandLine::get_instance()->begin_parameters() == CommandLine::get_instance()->end_parameters())
            throw args::DoHelp("search action takes at least one parameter");

        return do_search(*env);
    }
    catch (const DoVersion &)
    {
        display_version();
        cout << endl;
        cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
        cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
        cout << "License, version 2." << endl;

        return EXIT_SUCCESS;
    }
    catch (const paludis::args::ArgsError & e)
    {
        cerr << "Usage error: " << e.message() << endl;
        cerr << "Try " << argv[0] << " --help" << endl;
        return EXIT_FAILURE;
    }
    catch (const args::DoHelp & h)
    {
        if (h.message.empty())
        {
            cout << "Usage: " << argv[0] << " [options]" << endl;
            cout << endl;
            cout << *CommandLine::get_instance();
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Usage error: " << h.message << endl;
            cerr << "Try " << argv[0] << " --help" << endl;
            return EXIT_FAILURE;
        }
    }
    catch (const Exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cerr << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }
}

