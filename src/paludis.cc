/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "src/command_line.hh"
#include "src/install.hh"
#include "src/uninstall.hh"
#include "src/list.hh"
#include "src/query.hh"
#include "src/applets.hh"
#include "src/sync.hh"
#include <paludis/paludis.hh>
#include <paludis/util/util.hh>

#include <iostream>
#include <string>
#include <cstdlib>

/** \file
 * Main paludis program.
 */

namespace p = paludis;

using std::cout;
using std::cerr;
using std::endl;

#ifndef DOXYGEN
struct DoVersion
{
};
#endif

int
main(int argc, char *argv[])
{
    p::Context context("In main program:");

    try
    {
        context.change_context("When parsing command line arguments:");

        CommandLine::get_instance()->run(argc, argv);

        if (CommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (! CommandLine::get_instance()->a_log_level.specified())
            p::Log::get_instance()->set_log_level(p::ll_qa);
        else if (CommandLine::get_instance()->a_log_level.argument() == "debug")
            p::Log::get_instance()->set_log_level(p::ll_debug);
        else if (CommandLine::get_instance()->a_log_level.argument() == "qa")
            p::Log::get_instance()->set_log_level(p::ll_qa);
        else if (CommandLine::get_instance()->a_log_level.argument() == "warning")
            p::Log::get_instance()->set_log_level(p::ll_warning);
        else if (CommandLine::get_instance()->a_log_level.argument() == "silent")
            p::Log::get_instance()->set_log_level(p::ll_silent);
        else
            throw DoHelp("bad value for --log-level");

        p::Log::get_instance()->set_program_name(argv[0]);

        if (1 != (CommandLine::get_instance()->a_query.specified() +
                    CommandLine::get_instance()->a_version.specified() +
                    CommandLine::get_instance()->a_install.specified() +
                    CommandLine::get_instance()->a_uninstall.specified() +
                    CommandLine::get_instance()->a_sync.specified() +
                    CommandLine::get_instance()->a_list_repositories.specified() +
                    CommandLine::get_instance()->a_list_categories.specified() +
                    CommandLine::get_instance()->a_list_packages.specified() +
                    CommandLine::get_instance()->a_list_sync_protocols.specified() +
                    CommandLine::get_instance()->a_list_repository_formats.specified() +
                    CommandLine::get_instance()->a_list_dep_tag_categories.specified() +
                    CommandLine::get_instance()->a_has_version.specified() +
                    CommandLine::get_instance()->a_best_version.specified()))
        {
            if ((1 == std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters())) &&
                    ("moo" == *CommandLine::get_instance()->begin_parameters()))
            {
                cout << endl;
                cout << " ______________________________________" << endl;
                cout << "( Why do people keep doing this to me? )" << endl;
                cout << " -------------------------------------- " << endl;
                cout << "    o" << endl;
                cout << "     o" << endl;
                cout << "    ^__^         /" << endl;
                cout << "    (oo)\\_______/  _________" << endl;
                cout << "    (__)\\       )=(  ____|_ \\_____" << endl;
                cout << "        ||----w |  \\ \\     \\_____ |" << endl;
                cout << "        ||     ||   ||           ||" << endl;
                cout << endl;
                return EXIT_SUCCESS;
            }
            else
                throw DoHelp("you should specify exactly one action");
        }

        /* these actions don't need DefaultConfig */

        if (CommandLine::get_instance()->a_list_sync_protocols.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-sync-protocols action takes no parameters");

            return do_list_sync_protocols();
        }

        if (CommandLine::get_instance()->a_list_repository_formats.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-repository-formats action takes no parameters");

            return do_list_repository_formats();
        }

        if (CommandLine::get_instance()->a_list_dep_tag_categories.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-dep-tag-categories action takes no parameters");

            return do_list_dep_tag_categories();
        }

        /* these actions do need DefaultConfig */

        std::string paludis_command(argv[0]);
        if (CommandLine::get_instance()->a_config_suffix.specified())
        {
            p::DefaultConfig::set_config_suffix(CommandLine::get_instance()->a_config_suffix.argument());
            paludis_command.append(" --config-suffix " +
                    CommandLine::get_instance()->a_config_suffix.argument());
        }
        paludis_command.append(" --log-level " + CommandLine::get_instance()->a_log_level.argument());
        p::DefaultConfig::get_instance()->set_paludis_command(paludis_command);

        if (CommandLine::get_instance()->a_query.specified())
        {
            if (CommandLine::get_instance()->empty())
                throw DoHelp("query action requires at least one parameter");

            return do_query();
        }

        if (CommandLine::get_instance()->a_install.specified())
        {
            if (CommandLine::get_instance()->empty())
                throw DoHelp("install action requires at least one parameter");

            return do_install();
        }

        if (CommandLine::get_instance()->a_uninstall.specified())
        {
            if (CommandLine::get_instance()->empty())
                throw DoHelp("uninstall action requires at least one parameter");

            return do_uninstall();
        }

        if (CommandLine::get_instance()->a_sync.specified())
        {
            return do_sync();
        }

        if (CommandLine::get_instance()->a_list_repositories.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-repositories action takes no parameters");

            return do_list_repositories();
        }

        if (CommandLine::get_instance()->a_list_categories.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-categories action takes no parameters");

            return do_list_categories();
        }

        if (CommandLine::get_instance()->a_list_packages.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-packages action takes no parameters");

            return do_list_packages();
        }

        if (CommandLine::get_instance()->a_has_version.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("has-version action takes exactly one parameter");

            return do_has_version();
        }

        if (CommandLine::get_instance()->a_best_version.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("best-version action takes exactly one parameter");

            return do_best_version();
        }

        throw p::InternalError(__PRETTY_FUNCTION__, "no action?");
    }
    catch (const DoVersion &)
    {
        cout << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
        if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
            cout << " svn " << PALUDIS_SUBVERSION_REVISION;
        cout << endl << endl;
        cout << "Built by " << PALUDIS_BUILD_USER << "@" << PALUDIS_BUILD_HOST
            << " on " << PALUDIS_BUILD_DATE << endl;
        cout << "CXX:         " << PALUDIS_BUILD_CXX
#if defined(__ICC)
            << " " << __ICC
#elif defined(__VERSION__)
            << " " << __VERSION__
#endif
            << endl;
        cout << "CXXFLAGS:    " << PALUDIS_BUILD_CXXFLAGS << endl;
        cout << "LDFLAGS:     " << PALUDIS_BUILD_LDFLAGS << endl;
        cout << "SYSCONFDIR:  " << SYSCONFDIR << endl;
        cout << "LIBEXECDIR:  " << LIBEXECDIR << endl;
        cout << "BIGTEMPDIR:  " << BIGTEMPDIR << endl;
        cout << "stdlib:      "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
            << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
            << endl;

        cout << "libebt:      " << LIBEBT_VERSION_MAJOR << "." << LIBEBT_VERSION_MINOR
            << "." << LIBEBT_VERSION_MICRO << endl;
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
    catch (const DoHelp & h)
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
    catch (const p::Exception & e)
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

