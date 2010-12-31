/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/environment_factory.hh>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/about.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/unformatted_pretty_printer.hh>

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
            << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
        if (! std::string(PALUDIS_GIT_HEAD).empty())
            cout << " git " << PALUDIS_GIT_HEAD;
        cout << endl;
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

int
main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("IMPORTARE_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    Log::get_instance()->message("importare.deprecated", ll_warning, lc_context)
        << "importare is deprecated. Use 'cave import' instead.";

    try
    {
        CommandLine::get_instance()->run(argc, argv, "importare", "IMPORTARE_OPTIONS", "IMPORTARE_CMDLINE");
        set_use_colour(! CommandLine::get_instance()->a_no_colour.specified());
        set_force_colour(CommandLine::get_instance()->a_force_colour.specified());

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

        if (CommandLine::get_instance()->a_force_color.specified())
            paludis_command.append(" --" + CommandLine::get_instance()->a_force_color.long_name());

        paludis_command.append(CommandLine::get_instance()->install_args.paludis_command_fragment());
        paludis_command.append(CommandLine::get_instance()->dl_args.paludis_command_fragment());

        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(env_spec));
        env->set_paludis_command(paludis_command);

        std::vector<std::string> params(
                CommandLine::get_instance()->begin_parameters(),
                CommandLine::get_instance()->end_parameters());

        if ((params.size() > 3) || (params.size() < 1))
            throw args::DoHelp("install action takes between one and three parameters (cat/pkg version slot)");

        QualifiedPackageName q(params[0]);
        VersionSpec v(params.size() >= 2 ? params[1] : "0", user_version_spec_options());
        SlotName s(params.size() >= 3 ? params[2] : "0");

        std::string build_dependencies, run_dependencies, description;

        if (CommandLine::get_instance()->a_preserve_metadata.specified())
        {
            std::shared_ptr<const PackageIDSequence> old_ids((*env)[selection::AllVersionsSorted(generator::Package(q))]);
            std::shared_ptr<const PackageID> old_id;
            for (PackageIDSequence::ConstIterator i(old_ids->begin()), i_end(old_ids->end()) ;
                    i != i_end ; ++i)
            {
                auto repo(env->package_database()->fetch_repository((*i)->repository_name()));
                if (! repo->format_key())
                    continue;
                if (repo->format_key()->value() != "installed_unpackaged")
                    continue;
                old_id = *i;
                break;
            }

            if (! old_id)
                throw args::DoHelp("--" + CommandLine::get_instance()->a_preserve_metadata.long_name() + " specified but "
                        "no old ID available");

            if (old_id->short_description_key())
                description = old_id->short_description_key()->value();
            if (old_id->build_dependencies_key())
                build_dependencies = old_id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { });
            if (old_id->run_dependencies_key())
                run_dependencies = old_id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { });
        }

        if (CommandLine::get_instance()->a_description.specified())
            description = CommandLine::get_instance()->a_description.argument();
        if (CommandLine::get_instance()->a_build_dependency.specified())
            build_dependencies = join(
                    CommandLine::get_instance()->a_build_dependency.begin_args(),
                    CommandLine::get_instance()->a_build_dependency.end_args(), ", ");
        if (CommandLine::get_instance()->a_run_dependency.specified())
            run_dependencies = join(
                    CommandLine::get_instance()->a_run_dependency.begin_args(),
                    CommandLine::get_instance()->a_run_dependency.end_args(), ", ");

        std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
        keys->insert("location", stringify(
                    CommandLine::get_instance()->a_location.specified() ?
                    FSPath(CommandLine::get_instance()->a_location.argument()) :
                    FSPath::cwd()));
        keys->insert("install_under", stringify(
                    CommandLine::get_instance()->a_install_under.specified() ?
                    FSPath(CommandLine::get_instance()->a_install_under.argument()) :
                    FSPath("/")));
        keys->insert("rewrite_ids_over_to_root", stringify(
                    CommandLine::get_instance()->a_rewrite_ids_over_to_root.specified() ?
                    CommandLine::get_instance()->a_rewrite_ids_over_to_root.argument() : -1));
        keys->insert("format", "unpackaged");
        keys->insert("name", stringify(q));
        keys->insert("version", stringify(v));
        keys->insert("slot", stringify(s));
        keys->insert("description", description);
        keys->insert("build_dependencies", build_dependencies);
        keys->insert("run_dependencies", run_dependencies);
        std::shared_ptr<Repository> repo(RepositoryFactory::get_instance()->create(env.get(),
                    std::bind(from_keys, keys, std::placeholders::_1)));
        env->package_database()->add_repository(10, repo);
        std::shared_ptr<const PackageIDSequence> ids(repo->package_ids(q));
        if (1 != std::distance(ids->begin(), ids->end()))
            throw InternalError(PALUDIS_HERE, "ids is '" + join(indirect_iterator(ids->begin()), indirect_iterator(
                            ids->end()), " ") + "'");

        return do_install(env, *ids->begin());
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


