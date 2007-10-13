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
#include "install.hh"

#include <paludis/args/do_help.hh>
#include <src/output/colour.hh>

#include <paludis/environment_maker.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/sequence.hh>
#include <paludis/about.hh>
#include <paludis/repository_maker.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>

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
        cout << "importare, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
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
        CommandLine::get_instance()->run(argc, argv, "importare", "IMPORTARE_OPTIONS", "IMPORTARE_CMDLINE");
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

        /* need at most one action */
        if (1 < (
                    CommandLine::get_instance()->a_install.specified()
                    ))
            throw args::DoHelp("you should specify at most one action");

        std::string paludis_command("paludis"), env_spec;

        if (CommandLine::get_instance()->a_environment.specified())
        {
            env_spec = CommandLine::get_instance()->a_environment.argument();
            paludis_command.append(" --" + CommandLine::get_instance()->a_environment.long_name() + " " +
                    CommandLine::get_instance()->a_environment.argument());
        }

        paludis_command.append(" --" + CommandLine::get_instance()->a_log_level.long_name() + " " +
                CommandLine::get_instance()->a_log_level.argument());

        if (CommandLine::get_instance()->a_no_color.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_no_color.long_name());

        paludis_command.append(CommandLine::get_instance()->install_args.paludis_command_fragment());
        paludis_command.append(CommandLine::get_instance()->dl_args.paludis_command_fragment());

        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(env_spec));
        env->set_paludis_command(paludis_command);

        try
        {
            std::vector<std::string> params(
                    CommandLine::get_instance()->begin_parameters(),
                    CommandLine::get_instance()->end_parameters());

            if ((params.size() > 3) || (params.size() < 1))
                throw args::DoHelp("install action takes between one and three parameters (cat/pkg version slot)");

            QualifiedPackageName q(params[0]);
            VersionSpec v(params.size() >= 2 ? params[1] : "0");
            SlotName s(params.size() >= 3 ? params[2] : "0");

            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("location", stringify(
                        CommandLine::get_instance()->a_location.specified() ?
                        FSEntry(CommandLine::get_instance()->a_location.argument()) :
                        FSEntry::cwd()));
            keys->insert("format", "unpackaged");
            keys->insert("name", stringify(q));
            keys->insert("version", stringify(v));
            keys->insert("slot", stringify(s));
            tr1::shared_ptr<Repository> repo((*RepositoryMaker::get_instance()->find_maker("unpackaged"))(env.get(), keys));
            env->package_database()->add_repository(10, repo);
            tr1::shared_ptr<const PackageIDSequence> ids(repo->package_ids(q));
            if (1 != std::distance(ids->begin(), ids->end()))
                throw InternalError(PALUDIS_HERE, "ids is '" + join(indirect_iterator(ids->begin()), indirect_iterator(
                                ids->end()), " ") + "'");

            return do_install(env, *ids->begin());
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


