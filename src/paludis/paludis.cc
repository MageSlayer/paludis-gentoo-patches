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

#include "applets.hh"
#include "colour.hh"
#include "command_line.hh"
#include "contents.hh"
#include "install.hh"
#include "list.hh"
#include "news.hh"
#include "owner.hh"
#include "query.hh"
#include "sync.hh"
#include "uninstall.hh"
#include "config.h"

#include <paludis/paludis.hh>
#include <paludis/util/util.hh>

#include <libebt/libebt.hh>
#include <libwrapiter/libwrapiter.hh>

#include <iostream>
#include <iomanip>
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

namespace
{
    void display_version()
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
        cout << "DATADIR:     " << DATADIR << endl;
        cout << "LIBDIR:      " << LIBDIR << endl;
        cout << "LIBEXECDIR:  " << LIBEXECDIR << endl;
        cout << "SYSCONFDIR:  " << SYSCONFDIR << endl;
        cout << "stdlib:      "
#if defined(__GLIBCXX__)
#  define XSTRINGIFY(x) #x
#  define STRINGIFY(x) XSTRINGIFY(x)
            << "GNU libstdc++ " << STRINGIFY(__GLIBCXX__)
#endif
            << endl;

        cout << "libebt:      " << LIBEBT_VERSION_MAJOR << "." << LIBEBT_VERSION_MINOR
            << "." << LIBEBT_VERSION_MICRO << endl;
        cout << "libwrapiter: " << LIBWRAPITER_VERSION_MAJOR << "." << LIBWRAPITER_VERSION_MINOR
            << "." << LIBWRAPITER_VERSION_MICRO << endl;
#if HAVE_SANDBOX
        cout << "sandbox:     enabled" << endl;
#else
        cout << "sandbox:     disabled" << endl;
#endif
    }

    void display_info()
    {
        p::Environment * const env(p::DefaultEnvironment::get_instance());

        for (p::IndirectIterator<p::PackageDatabase::RepositoryIterator, const p::Repository>
                r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            cout << "Repository " << colour(cl_repository_name, r->name()) << ":" << endl;

            p::RepositoryInfo::ConstPointer ii(r->info(true));
            for (p::RepositoryInfo::SectionIterator i(ii->begin_sections()),
                    i_end(ii->end_sections()) ; i != i_end ; ++i)
            {
                cout << "    " << colour(cl_heading, (*i)->heading() + ":") << endl;
                for (p::RepositoryInfoSection::KeyValueIterator k((*i)->begin_kvs()),
                        k_end((*i)->end_kvs()) ; k != k_end ; ++k)
                    cout << "        " << std::setw(22) << std::left << (p::stringify(k->first) + ":")
                        << std::setw(0) << " " << k->second << endl;
                cout << endl;
            }
        }

    }
}

int
main(int argc, char *argv[])
{
    p::Context context("In main program:");

    try
    {
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
                    CommandLine::get_instance()->a_list_vulnerabilities.specified() +
                    CommandLine::get_instance()->a_contents.specified() +
                    CommandLine::get_instance()->a_owner.specified() +
                    CommandLine::get_instance()->a_has_version.specified() +
                    CommandLine::get_instance()->a_update_news.specified() +
                    CommandLine::get_instance()->a_environment_variable.specified() +
                    CommandLine::get_instance()->a_configuration_variable.specified() +
                    CommandLine::get_instance()->a_info.specified() +
                    CommandLine::get_instance()->a_best_version.specified()))
        {
            if ((1 == std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters())) &&
                    ("moo" == *CommandLine::get_instance()->begin_parameters()))
            {
                cout << endl;
                cout << " ______________________________" << endl;
                cout << "( Ugh. Another Portage user... )" << endl;
                cout << " ------------------------------ " << endl;
                cout << "    o" << endl;
                cout << "     o" << endl;
                cout << "    ^__^         /" << endl;
                cout << "    (" << colour(cl_bold_pink, "oo") << ")\\_______/  _________" << endl;
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

        if (CommandLine::get_instance()->a_list_vulnerabilities.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-vulnerabilities action takes no paramters");

            return do_list_vulnerabilities();
        }

        /* these actions do need DefaultConfig */

        try
        {
            std::string paludis_command(argv[0]);
            if (CommandLine::get_instance()->a_config_suffix.specified())
            {
                p::DefaultConfig::set_config_suffix(CommandLine::get_instance()->a_config_suffix.argument());
                paludis_command.append(" --config-suffix " +
                        CommandLine::get_instance()->a_config_suffix.argument());
            }
            paludis_command.append(" --log-level " + CommandLine::get_instance()->a_log_level.argument());
            p::DefaultConfig::get_instance()->set_paludis_command(paludis_command);
        }
        catch (const p::DefaultConfigError & e)
        {
            if (CommandLine::get_instance()->a_info.specified())
            {
                display_version();
                cout << endl;
                cout << "Cannot complete --info output due to configuration exception" << endl;
            }
            throw;
        }

        if (CommandLine::get_instance()->a_info.specified())
        {
            display_version();
            cout << endl;
            display_info();
            cout << endl;
            return EXIT_SUCCESS;
        }

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

        if (CommandLine::get_instance()->a_contents.specified())
        {
            if (CommandLine::get_instance()->empty())
                throw DoHelp("contents action requires at least one parameter");

            return do_contents();
        }

        if (CommandLine::get_instance()->a_owner.specified())
        {
            if (CommandLine::get_instance()->empty())
                throw DoHelp("owner action requires at least one parameter");

            return do_owner();
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

        if (CommandLine::get_instance()->a_environment_variable.specified())
        {
            if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("environment-variable action takes exactly two parameters (depatom var)");

            return do_environment_variable();
        }

        if (CommandLine::get_instance()->a_configuration_variable.specified())
        {
            if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("configuration-variable action takes exactly two parameters (depatom var)");

            return do_configuration_variable();
        }

        if (CommandLine::get_instance()->a_update_news.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("update-news action takes no parameters");

            return do_update_news();
        }

        throw p::InternalError(__PRETTY_FUNCTION__, "no action?");
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

