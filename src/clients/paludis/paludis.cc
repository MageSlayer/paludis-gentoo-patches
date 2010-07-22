/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include "applets.hh"
#include "command_line.hh"
#include "config.h"
#include "do_contents.hh"
#include "do_executables.hh"
#include "do_config.hh"
#include "install.hh"
#include "list.hh"
#include "info.hh"
#include "owner.hh"
#include "query.hh"
#include "report.hh"
#include "sync.hh"
#include "uninstall.hh"

#include <paludis/args/do_help.hh>
#include <src/output/colour.hh>

#include <paludis/paludis.hh>
#include <paludis/environment_factory.hh>
#include <paludis/util/system.hh>
#include <paludis/util/util.hh>
#include <paludis/util/log.hh>
#include <paludis/fuzzy_finder.hh>

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iterator>

#include <time.h>
#include <unistd.h>

/** \file
 * Main paludis program.
 */

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    struct DoVersion
    {
    };
}

namespace
{
    void display_version()
    {
        cout << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
        if (! std::string(PALUDIS_GIT_HEAD).empty())
            cout << " git " << PALUDIS_GIT_HEAD;
        cout << endl;
    }
}

int
main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("PALUDIS_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "paludis", "PALUDIS_OPTIONS", "PALUDIS_CMDLINE");
        set_use_colour(! CommandLine::get_instance()->a_no_colour.specified());
        set_force_colour(CommandLine::get_instance()->a_force_colour.specified());
        if (1 != isatty(1))
            CommandLine::get_instance()->a_no_suggestions.set_specified(true);

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
        if (1 != (CommandLine::get_instance()->a_query.specified() +
                    CommandLine::get_instance()->a_version.specified() +
                    CommandLine::get_instance()->a_install.specified() +
                    CommandLine::get_instance()->a_uninstall.specified() +
                    CommandLine::get_instance()->a_uninstall_unused.specified() +
                    CommandLine::get_instance()->a_sync.specified() +
                    CommandLine::get_instance()->a_report.specified() +
                    CommandLine::get_instance()->a_list_repositories.specified() +
                    CommandLine::get_instance()->a_list_categories.specified() +
                    CommandLine::get_instance()->a_list_packages.specified() +
                    CommandLine::get_instance()->a_list_sets.specified() +
                    CommandLine::get_instance()->a_list_sync_protocols.specified() +
                    CommandLine::get_instance()->a_list_repository_formats.specified() +
                    CommandLine::get_instance()->a_contents.specified() +
                    CommandLine::get_instance()->a_executables.specified() +
                    CommandLine::get_instance()->a_owner.specified() +
                    CommandLine::get_instance()->a_config.specified() +
                    CommandLine::get_instance()->a_has_version.specified() +
                    CommandLine::get_instance()->a_regenerate_installed_cache.specified() +
                    CommandLine::get_instance()->a_regenerate_installable_cache.specified() +
                    CommandLine::get_instance()->a_environment_variable.specified() +
                    CommandLine::get_instance()->a_configuration_variable.specified() +
                    CommandLine::get_instance()->a_info.specified() +
                    CommandLine::get_instance()->a_best_version.specified() +
                    CommandLine::get_instance()->a_match.specified()
                    ))
            throw args::DoHelp("you should specify exactly one action");

        /* these actions don't need Environment or paludis_command. */

        if (CommandLine::get_instance()->a_list_repository_formats.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw args::DoHelp("list-repository-formats action takes no parameters");

            return do_list_repository_formats();
        }

        /* these actions do need Environment or paludis_command */

        std::string paludis_command(argv[0]), env_spec;

        if (CommandLine::get_instance()->a_environment.specified())
        {
            env_spec = CommandLine::get_instance()->a_environment.argument();
            paludis_command.append(" --" + CommandLine::get_instance()->a_environment.long_name() + " " +
                    CommandLine::get_instance()->a_environment.argument());
        }

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

        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(env_spec));
        env->set_paludis_command(paludis_command);

        try
        {
            if (CommandLine::get_instance()->a_list_sync_protocols.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("list-sync-protocols action takes no parameters");

                return do_list_sync_protocols(env);
            }

            if (CommandLine::get_instance()->a_info.specified())
            {
                display_version();
                return do_info(env);
            }

            if (CommandLine::get_instance()->a_query.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("query action requires at least one parameter");

                return do_query(env);
            }

            if (CommandLine::get_instance()->a_install.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("install action requires at least one parameter");

                return do_install(env);
            }

            if (CommandLine::get_instance()->a_uninstall.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("uninstall action requires at least one parameter");

                return do_uninstall(env);
            }

            if (CommandLine::get_instance()->a_config.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("config action requires at least one parameter");

                return do_config(env);
            }

            if (CommandLine::get_instance()->a_uninstall_unused.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("uninstall-unused action takes no parameters");

                return do_uninstall_unused(env);
            }

            if (CommandLine::get_instance()->a_sync.specified())
            {
                return do_sync(env);
            }

            if (CommandLine::get_instance()->a_report.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("report action takes no parameters");
                return do_report(env);
            }

            if (CommandLine::get_instance()->a_list_repositories.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("list-repositories action takes no parameters");

                return do_list_repositories(env);
            }

            if (CommandLine::get_instance()->a_list_categories.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("list-categories action takes no parameters");

                return do_list_categories(env);
            }

            if (CommandLine::get_instance()->a_list_packages.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("list-packages action takes no parameters");

                return do_list_packages(env);
            }

            if (CommandLine::get_instance()->a_list_sets.specified())
            {
                if (! CommandLine::get_instance()->empty())
                    throw args::DoHelp("list-sets action takes no parameters");

                return do_list_sets(env);
            }

            if (CommandLine::get_instance()->a_contents.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("contents action requires at least one parameter");

                return do_contents(env);
            }

            if (CommandLine::get_instance()->a_executables.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("executables action requires at least one parameter");

                return do_executables(env);
            }

            if (CommandLine::get_instance()->a_owner.specified())
            {
                if (CommandLine::get_instance()->empty())
                    throw args::DoHelp("owner action requires at least one parameter");

                return do_owner(env);
            }

            if (CommandLine::get_instance()->a_has_version.specified())
            {
                if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters()))
                    throw args::DoHelp("has-version action takes exactly one parameter");

                return do_has_version(env);
            }

            if (CommandLine::get_instance()->a_best_version.specified())
            {
                if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters()))
                    throw args::DoHelp("best-version action takes exactly one parameter");

                return do_best_version(env);
            }

            if (CommandLine::get_instance()->a_match.specified())
            {
                if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters()))
                    throw args::DoHelp("match action takes exactly one parameter");

                return do_match(env);
            }

            if (CommandLine::get_instance()->a_environment_variable.specified())
            {
                if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters()))
                    throw args::DoHelp("environment-variable action takes exactly two parameters (depspec var)");

                return do_environment_variable(env);
            }

            if (CommandLine::get_instance()->a_configuration_variable.specified())
            {
                if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                            CommandLine::get_instance()->end_parameters()))
                    throw args::DoHelp("configuration-variable action takes exactly two parameters (repository var)");

                return do_configuration_variable(env);
            }

            if (CommandLine::get_instance()->a_regenerate_installed_cache.specified() ||
                    CommandLine::get_instance()->a_regenerate_installable_cache.specified())
            {
                return do_regenerate_cache(env, CommandLine::get_instance()->a_regenerate_installed_cache.specified());
            }

            throw InternalError(__PRETTY_FUNCTION__, "no action?");
        }
        catch (const NoSuchRepositoryError & e)
        {
            cout << endl;
            cerr << "Unhandled exception:" << endl
                << "  * " << e.backtrace("\n  * ")
                << e.message() << " (" << e.what() << ")" << endl;

            if (! CommandLine::get_instance()->a_no_suggestions.specified())
            {
                cerr << "  * Looking for suggestions:" << endl;

                FuzzyRepositoriesFinder f(*env, stringify(e.name()));

                if (f.begin() == f.end())
                    cerr << "No suggestions found." << endl;
                else
                    cerr << "Suggestions:" << endl;

                for (FuzzyRepositoriesFinder::RepositoriesConstIterator r(f.begin()), r_end(f.end()) ;
                        r != r_end ; ++r)
                    cerr << " * " << colour(cl_repository_name, *r) << endl;
                cerr << endl;
            }

            return EXIT_FAILURE;
        }
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
    catch (const args::ArgsError & e)
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
    catch (const ActionAbortedError & e)
    {
        cout << endl;
        cerr << "Action aborted:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return 42;
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

