/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "do_contents.hh"
#include "install.hh"
#include "list.hh"
#include "news.hh"
#include "owner.hh"
#include "query.hh"
#include "report.hh"
#include "sync.hh"
#include "uninstall.hh"
#include "config.h"

#include <paludis/paludis.hh>
#include <paludis/util/util.hh>
#include <paludis/util/log.hh>
#include <paludis/environment/default/default_environment.hh>
#include <paludis/environment/default/default_config.hh>

#include <libebt/libebt.hh>
#include <libwrapiter/libwrapiter.hh>

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

/** \file
 * Main paludis program.
 */

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

struct DoVersion
{
};

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
        Environment * const env(DefaultEnvironment::get_instance());

        for (IndirectIterator<PackageDatabase::RepositoryIterator, const Repository>
                r(env->package_database()->begin_repositories()), r_end(env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
        {
            cout << "Repository " << colour(cl_repository_name, r->name()) << ":" << endl;

            RepositoryInfo::ConstPointer ii(r->info(true));
            for (RepositoryInfo::SectionIterator i(ii->begin_sections()),
                    i_end(ii->end_sections()) ; i != i_end ; ++i)
            {
                cout << "    " << colour(cl_heading, (*i)->heading() + ":") << endl;
                for (RepositoryInfoSection::KeyValueIterator k((*i)->begin_kvs()),
                        k_end((*i)->end_kvs()) ; k != k_end ; ++k)
                    cout << "        " << std::setw(22) << std::left << (stringify(k->first) + ":")
                        << std::setw(0) << " " << k->second << endl;
                cout << endl;
            }
        }

    }
}

int
main(int argc, char *argv[])
{
    Context context("In program " + join(argv, argv + argc, " ") + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "PALUDIS_OPTIONS");
        set_use_colour(! CommandLine::get_instance()->a_no_color.specified());

        if (CommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (! CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(ll_qa);
        else if (CommandLine::get_instance()->a_log_level.argument() == "debug")
            Log::get_instance()->set_log_level(ll_debug);
        else if (CommandLine::get_instance()->a_log_level.argument() == "qa")
            Log::get_instance()->set_log_level(ll_qa);
        else if (CommandLine::get_instance()->a_log_level.argument() == "warning")
            Log::get_instance()->set_log_level(ll_warning);
        else if (CommandLine::get_instance()->a_log_level.argument() == "silent")
            Log::get_instance()->set_log_level(ll_silent);
        else
            throw DoHelp("bad value for --log-level");

        Log::get_instance()->set_program_name(argv[0]);

        /* deprecated args */
        if (CommandLine::get_instance()->a_dl_no_unnecessary_upgrades.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "--dl-no-unnecessary-upgrades / -U is deprecated");
            CommandLine::get_instance()->dl_upgrade.set_argument("as-needed");
            CommandLine::get_instance()->dl_upgrade.set_specified(true);
        }
        if (CommandLine::get_instance()->a_dl_drop_all.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "--dl-drop-all / -0 is deprecated");
            CommandLine::get_instance()->dl_installed_deps_pre.set_argument("discard");
            CommandLine::get_instance()->dl_installed_deps_pre.set_specified(true);
            CommandLine::get_instance()->dl_installed_deps_post.set_argument("discard");
            CommandLine::get_instance()->dl_installed_deps_post.set_specified(true);
            CommandLine::get_instance()->dl_installed_deps_runtime.set_argument("discard");
            CommandLine::get_instance()->dl_installed_deps_runtime.set_specified(true);
            CommandLine::get_instance()->dl_uninstalled_deps_pre.set_argument("discard");
            CommandLine::get_instance()->dl_uninstalled_deps_pre.set_specified(true);
            CommandLine::get_instance()->dl_uninstalled_deps_post.set_argument("discard");
            CommandLine::get_instance()->dl_uninstalled_deps_post.set_specified(true);
            CommandLine::get_instance()->dl_uninstalled_deps_runtime.set_argument("discard");
            CommandLine::get_instance()->dl_uninstalled_deps_runtime.set_specified(true);
        }
        if (CommandLine::get_instance()->a_dl_ignore_installed.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "--dl-ignore-installed / -e is deprecated");
            CommandLine::get_instance()->dl_reinstall.set_argument("always");
            CommandLine::get_instance()->dl_reinstall.set_specified("always");
        }

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
                    CommandLine::get_instance()->a_list_dep_tag_categories.specified() +
                    CommandLine::get_instance()->a_contents.specified() +
                    CommandLine::get_instance()->a_owner.specified() +
                    CommandLine::get_instance()->a_has_version.specified() +
                    CommandLine::get_instance()->a_update_news.specified() +
                    CommandLine::get_instance()->a_regenerate_installed_cache.specified() +
                    CommandLine::get_instance()->a_regenerate_installable_cache.specified() +
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

        /* these actions do need DefaultConfig */

        try
        {
            std::string paludis_command(argv[0]);
            if (CommandLine::get_instance()->a_config_suffix.specified())
            {
                DefaultConfig::set_config_suffix(CommandLine::get_instance()->a_config_suffix.argument());
                paludis_command.append(" --config-suffix " +
                        CommandLine::get_instance()->a_config_suffix.argument());
            }
            paludis_command.append(" --log-level " + CommandLine::get_instance()->a_log_level.argument());
            DefaultConfig::get_instance()->set_paludis_command(paludis_command);
        }
        catch (const DefaultConfigError &)
        {
            if (CommandLine::get_instance()->a_info.specified())
            {
                display_version();
                cout << endl;
                cout << "Cannot complete --info output due to configuration exception" << endl;
            }
            throw;
        }

        if (CommandLine::get_instance()->a_resume_command_template.specified())
        {
            // The template should contain at least XXXXXX
            std::string resume_template = CommandLine::get_instance()->a_resume_command_template.argument();
            if (resume_template.find("XXXXXX", 0) == std::string::npos )
                throw DoHelp("resume-command-template must contain at least XXXXXX");
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

        if (CommandLine::get_instance()->a_uninstall_unused.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("uninstall-unused action takes no parameters");

            return do_uninstall_unused();
        }

        if (CommandLine::get_instance()->a_sync.specified())
        {
            return do_sync();
        }

        if (CommandLine::get_instance()->a_report.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("report action takes no parameters");
            return do_report();
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

        if (CommandLine::get_instance()->a_list_sets.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("list-sets action takes no parameters");

            return do_list_sets();
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
                throw DoHelp("configuration-variable action takes exactly two parameters (repository var)");

            return do_configuration_variable();
        }

        if (CommandLine::get_instance()->a_update_news.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("update-news action takes no parameters");

            return do_update_news();
        }

        if (CommandLine::get_instance()->a_regenerate_installed_cache.specified() ||
                CommandLine::get_instance()->a_regenerate_installable_cache.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw DoHelp("regenerate cache actions takes no parameters");

            return do_regenerate_cache(CommandLine::get_instance()->a_regenerate_installed_cache.specified());
        }

        throw InternalError(__PRETTY_FUNCTION__, "no action?");
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

