/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include "do_config.hh"
#include "install.hh"
#include "list.hh"
#include "info.hh"
#include "owner.hh"
#include "query.hh"
#include "report.hh"
#include "sync.hh"
#include "uninstall.hh"

#include <src/common_args/do_help.hh>
#include <src/output/colour.hh>

#include <paludis/paludis.hh>
#include <paludis/environments/environment_maker.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/util.hh>
#include <paludis/util/log.hh>

#include <libebt/libebt.hh>
#include <libwrapiter/libwrapiter.hh>

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iterator>

#include <time.h>

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
        cout << endl;
    }
}

int
main(int argc, char *argv[])
{
    Context context("In program " + join(argv, argv + argc, " ") + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "paludis", "PALUDIS_OPTIONS", "PALUDIS_CMDLINE");
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
            CommandLine::get_instance()->dl_reinstall.set_specified(true);
        }

        if (CommandLine::get_instance()->a_show_install_reasons.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "--show-install-reasons is deprecated, use --show-reasons");
            CommandLine::get_instance()->a_show_reasons.set_argument(
                    CommandLine::get_instance()->a_show_install_reasons.argument());
            CommandLine::get_instance()->a_show_reasons.set_specified(true);
        }

        if (CommandLine::get_instance()->a_add_to_world_atom.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "--add-to-world-atom is deprecated, use --add-to-world-spec");
            CommandLine::get_instance()->a_add_to_world_spec.set_argument(
                    CommandLine::get_instance()->a_add_to_world_atom.argument());
            CommandLine::get_instance()->a_add_to_world_spec.set_specified(true);
        }

        if (CommandLine::get_instance()->a_safe_resume.specified())
        {
            Log::get_instance()->message(ll_warning, lc_no_context) <<
                    "Safe resume support is now enabled by default; there is no need to pass --safe-resume";
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
                    CommandLine::get_instance()->a_config.specified() +
                    CommandLine::get_instance()->a_has_version.specified() +
                    CommandLine::get_instance()->a_update_news.specified() +
                    CommandLine::get_instance()->a_regenerate_installed_cache.specified() +
                    CommandLine::get_instance()->a_regenerate_installable_cache.specified() +
                    CommandLine::get_instance()->a_environment_variable.specified() +
                    CommandLine::get_instance()->a_configuration_variable.specified() +
                    CommandLine::get_instance()->a_info.specified() +
                    CommandLine::get_instance()->a_best_version.specified() +
                    CommandLine::get_instance()->a_match.specified()
                    ))
        {
            if ((1 == std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters())) &&
                    (47503 == (0xffff & paludis::CRCHash<std::string>()
                               (*CommandLine::get_instance()->begin_parameters()))))
            {
                time_t t(time(0));
                const struct tm * tt(localtime(&t));
                if (tt->tm_mday == 1 && tt->tm_mon == 3)
                {
                    static const char chk[] = {
                        0x0a, 0x0a, 0x7c, 0x7c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                        0x20, 0x20, 0x7c, 0x7c, 0x20, 0x20, 0x20, 0x7c, 0x7c, 0x20, 0x20, 0x20, 0x20,
                        0x20, 0x7c, 0x7c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x7c,
                        0x20, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c,
                        0x20, 0x5c, 0x20, 0x20, 0x7c, 0x20, 0x77, 0x2d, 0x2d, 0x2d, 0x2d, 0x7c, 0x7c,
                        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x5f, 0x5f, 0x5f, 0x5f,
                        0x5f, 0x5c, 0x20, 0x5f, 0x7c, 0x5f, 0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x28, 0x3d,
                        0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x29, 0x5f, 0x5f, 0x28,
                        0x20, 0x20, 0x20, 0x20, 0x0a, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
                        0x5f, 0x20, 0x20, 0x2f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5c, 0x29,
                        0x6f, 0x6f, 0x28, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x2f, 0x20, 0x20, 0x20, 0x20,
                        0x20, 0x20, 0x20, 0x20, 0x20, 0x5e, 0x5f, 0x5f, 0x5e, 0x20, 0x20, 0x20, 0x20,
                        0x0a, 0x6f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x6f, 0x20, 0x20, 0x20, 0x20,
                        0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
                        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
                        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x20, 0x0a, 0x29, 0x20, 0x2e, 0x2e, 0x2e, 0x72,
                        0x65, 0x73, 0x75, 0x20, 0x65, 0x67, 0x61, 0x74, 0x72, 0x6f, 0x50, 0x20, 0x72,
                        0x65, 0x68, 0x74, 0x6f, 0x6e, 0x41, 0x20, 0x2e, 0x68, 0x67, 0x55, 0x20, 0x28,
                        0x0a, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
                        0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
                        0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x20, 0x0a
                    };
                    cout << endl;
                    std::reverse_copy(&chk[0], &chk[sizeof(chk) / sizeof(char)], std::ostreambuf_iterator<char>(std::cout));
                    cout << endl;
                    return EXIT_SUCCESS;
                }
                else
                    throw args::DoHelp("don't be silly");
            }
            else
                throw args::DoHelp("you should specify exactly one action");
        }

        /* these actions don't need Environment or paludis_command. */

        if (CommandLine::get_instance()->a_list_repository_formats.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw args::DoHelp("list-repository-formats action takes no parameters");

            return do_list_repository_formats();
        }

        if (CommandLine::get_instance()->a_list_dep_tag_categories.specified())
        {
            if (! CommandLine::get_instance()->empty())
                throw args::DoHelp("list-dep-tag-categories action takes no parameters");

            return do_list_dep_tag_categories();
        }

        /* these actions do need Environment or paludis_command */

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

        if (CommandLine::get_instance()->a_no_config_protection.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_no_config_protection.long_name());

        if (CommandLine::get_instance()->a_preserve_world.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_preserve_world.long_name());

        if (CommandLine::get_instance()->a_debug_build.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_debug_build.long_name() + " "
                    + CommandLine::get_instance()->a_debug_build.argument());

        if (CommandLine::get_instance()->a_no_safe_resume.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_no_safe_resume.long_name());

        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(env_spec));
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

            if (CommandLine::get_instance()->a_update_news.specified())
            {
                Log::get_instance()->message(ll_warning, lc_no_context,
                        "Calling --update-news is no longer useful or necessary");
                return EXIT_SUCCESS;
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
            if (env->package_database()->has_repository_named(RepositoryName("x-" + stringify(e.name()))))
                cerr << "Perhaps you meant 'x-" << e.name() << "'?" << endl;
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

