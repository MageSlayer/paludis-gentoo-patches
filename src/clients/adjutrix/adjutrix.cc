/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <output/colour.hh>
#include "command_line.hh"
#include "find_stable_candidates.hh"
#include "find_dropped_keywords.hh"
#include "find_insecure_packages.hh"
#include "find_reverse_deps.hh"
#include "find_unused_packages.hh"
#include "keywords_graph.hh"
#include "display_default_system_resolution.hh"
#include "what_needs_keywording.hh"
#include "downgrade_check.hh"

#include <paludis/about.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    FSEntry
    get_location_and_add_filters()
    {
        Context context("When determining tree location:");

        if (CommandLine::get_instance()->a_repository_directory.specified())
            return FSEntry(CommandLine::get_instance()->a_repository_directory.argument());

        if ((FSEntry::cwd() / "profiles").is_directory())
            return FSEntry::cwd();
        if ((FSEntry::cwd().dirname() / "profiles").is_directory())
        {
            CommandLine::get_instance()->a_category.add_argument(FSEntry::cwd().basename());
            CommandLine::get_instance()->a_category.set_specified(true);
            return FSEntry::cwd().dirname();
        }
        if ((FSEntry::cwd().dirname().dirname() / "profiles").is_directory())
        {
            CommandLine::get_instance()->a_package.add_argument(FSEntry::cwd().basename());
            CommandLine::get_instance()->a_package.set_specified(true);
            CommandLine::get_instance()->a_category.add_argument(FSEntry::cwd().dirname().basename());
            CommandLine::get_instance()->a_category.set_specified(true);
            return FSEntry::cwd().dirname().dirname();
        }

        throw ConfigurationError("Cannot find tree location (try specifying --repository-dir)");
    }

    void display_version()
    {
        cout << "adjutrix, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
        if (! std::string(PALUDIS_GIT_HEAD).empty())
            cout << " git " << PALUDIS_GIT_HEAD;
        cout << endl;
    }

    struct DoVersion
    {
    };
}

int
main(int argc, char *argv[])
{
    Context context("In program " + join(argv, argv + argc, " ") + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "adjutrix", "ADJUTRIX_OPTIONS", "ADJUTRIX_CMDLINE");
        set_use_colour(! CommandLine::get_instance()->a_no_colour.specified());
        set_force_colour(CommandLine::get_instance()->a_force_colour.specified());
        if (1 != isatty(1))
            CommandLine::get_instance()->a_no_suggestions.set_specified(true);

        if (CommandLine::get_instance()->a_help.specified())
            throw DoHelp();

        if (CommandLine::get_instance()->a_version.specified())
            throw DoVersion();

        if (CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());
        else
            Log::get_instance()->set_log_level(ll_qa);

        if (1 != (
                    CommandLine::get_instance()->a_find_stable_candidates.specified() +
                    CommandLine::get_instance()->a_find_dropped_keywords.specified() +
                    CommandLine::get_instance()->a_find_insecure_packages.specified() +
                    CommandLine::get_instance()->a_find_unused_packages.specified() +
                    CommandLine::get_instance()->a_keywords_graph.specified() +
                    CommandLine::get_instance()->a_reverse_deps.specified() +
                    CommandLine::get_instance()->a_display_default_system_resolution.specified() +
                    CommandLine::get_instance()->a_build_downgrade_check_list.specified() +
                    CommandLine::get_instance()->a_downgrade_check.specified() +
                    CommandLine::get_instance()->a_what_needs_keywording.specified()
                    ))
            throw DoHelp("you should specify exactly one action");

        if (! CommandLine::get_instance()->a_write_cache_dir.specified())
            CommandLine::get_instance()->a_write_cache_dir.set_argument("/var/empty");

        std::shared_ptr<FSEntrySequence> extra_repository_dirs(new FSEntrySequence);
        for (args::StringSequenceArg::ConstIterator d(CommandLine::get_instance()->a_extra_repository_dir.begin_args()),
                d_end(CommandLine::get_instance()->a_extra_repository_dir.end_args()) ;
                d != d_end ; ++d)
            extra_repository_dirs->push_back(*d);

        NoConfigEnvironment env(make_named_values<no_config_environment::Params>(
                    n::accept_unstable() = CommandLine::get_instance()->a_unstable.specified(),
                    n::disable_metadata_cache() = false,
                    n::extra_accept_keywords() = "",
                    n::extra_params() = std::shared_ptr<Map<std::string, std::string> >(),
                    n::extra_repository_dirs() = extra_repository_dirs,
                    n::master_repository_name() = CommandLine::get_instance()->a_master_repository_name.argument(),
                    n::profiles_if_not_auto() = CommandLine::get_instance()->a_profile.argument(),
                    n::repository_dir() = get_location_and_add_filters(),
                    n::repository_type() = (CommandLine::get_instance()->a_reverse_deps.specified()) ?
                        no_config_environment::ncer_auto : no_config_environment::ncer_ebuild,
                    n::write_cache() = CommandLine::get_instance()->a_write_cache_dir.argument()
                    ));

        if (CommandLine::get_instance()->a_find_stable_candidates.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("find-stable-candidates action takes exactly one parameter (the target arch)");

            do_find_stable_candidates(env);
            return EXIT_SUCCESS;
        }

        if (CommandLine::get_instance()->a_find_dropped_keywords.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("find-dropped-keywords action takes exactly one parameter (the target arch)");

            do_find_dropped_keywords(env);
            return EXIT_SUCCESS;
        }

        if (CommandLine::get_instance()->a_find_insecure_packages.specified())
        {
            if (CommandLine::get_instance()->begin_parameters() !=
                        CommandLine::get_instance()->end_parameters())
                throw DoHelp("find-dropped-keywords action takes no parameters");

            do_find_insecure_packages(env);
            return EXIT_SUCCESS;
        }

        if (CommandLine::get_instance()->a_find_unused_packages.specified())
        {
            if (CommandLine::get_instance()->begin_parameters() !=
                        CommandLine::get_instance()->end_parameters())
                throw DoHelp("find-unused-packages action takes no parameters");

            do_find_unused_packages(env);
            return EXIT_SUCCESS;
        }


        if (CommandLine::get_instance()->a_keywords_graph.specified())
        {
            if (CommandLine::get_instance()->begin_parameters() !=
                        CommandLine::get_instance()->end_parameters())
                throw DoHelp("keywords-graph action takes no parameters");

            do_keywords_graph(env);
            return EXIT_SUCCESS;
        }

        if (CommandLine::get_instance()->a_reverse_deps.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("reverse-deps action takes exactly one parameter (the target dep)");

            return do_find_reverse_deps(env);
        }

        if (CommandLine::get_instance()->a_display_default_system_resolution.specified())
        {
            if (CommandLine::get_instance()->begin_parameters() !=
                        CommandLine::get_instance()->end_parameters())
                throw DoHelp("display-default-system-resolution action takes no parameters");

            return do_display_default_system_resolution(env);
        }

        if (CommandLine::get_instance()->a_what_needs_keywording.specified())
        {
            if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("what-needs-keywording action takes exactly two parameters "
                        "(the target keyword and the target package)");

            return do_what_needs_keywording(env);
        }

        if (CommandLine::get_instance()->a_build_downgrade_check_list.specified())
        {
            if (1 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("build-downgrade-check-list takes exactly one parameter (the output directory)");

            return do_build_downgrade_check_list(env);
        }

        if (CommandLine::get_instance()->a_downgrade_check.specified())
        {
            if (2 != std::distance(CommandLine::get_instance()->begin_parameters(),
                        CommandLine::get_instance()->end_parameters()))
                throw DoHelp("build-downgrade-check-list takes exactly two parameters "
                        "(the before and after directories)");

            return do_downgrade_check(env);
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

