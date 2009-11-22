/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/args/do_help.hh>
#include "command_line.hh"
#include <paludis/about.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/package_database.hh>
#include <paludis/output_manager_from_environment.hh>
#include <cstdlib>
#include <tr1/functional>
#include <iostream>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

int
main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("ACCERSO_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "accerso", "ACCERSO_OPTIONS", "ACCERSO_CMDLINE");

        set_use_colour(
                (! CommandLine::get_instance()->a_no_colour.specified()) &&
                (! CommandLine::get_instance()->a_report_file.specified()));
        set_force_colour(CommandLine::get_instance()->a_force_colour.specified());

        if (CommandLine::get_instance()->a_help.specified())
            throw args::DoHelp();

        if (CommandLine::get_instance()->a_log_level.specified())
            Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());
        else
            Log::get_instance()->set_log_level(ll_qa);

        if (1 < (
                    CommandLine::get_instance()->a_fetch.specified() +
                    CommandLine::get_instance()->a_version.specified()
                ))
            throw args::DoHelp("you should specify exactly one action");

        if (! CommandLine::get_instance()->a_write_cache_dir.specified())
            CommandLine::get_instance()->a_write_cache_dir.set_argument("/var/empty");

        if (! CommandLine::get_instance()->a_repository_directory.specified())
            CommandLine::get_instance()->a_repository_directory.set_argument(stringify(FSEntry::cwd()));

        if (CommandLine::get_instance()->a_version.specified())
        {
            cout << "accerso, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
                << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
            if (! std::string(PALUDIS_GIT_HEAD).empty())
                cout << " git " << PALUDIS_GIT_HEAD;
            cout << endl << endl;
            cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
            cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
            cout << "License, version 2." << endl;

            return EXIT_SUCCESS;
        }
        else
        {
            std::tr1::shared_ptr<FSEntrySequence> extra_repository_dirs(new FSEntrySequence);
            for (args::StringSequenceArg::ConstIterator d(CommandLine::get_instance()->a_extra_repository_dir.begin_args()),
                    d_end(CommandLine::get_instance()->a_extra_repository_dir.end_args()) ;
                    d != d_end ; ++d)
                extra_repository_dirs->push_back(*d);

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("distdir", CommandLine::get_instance()->a_download_directory.argument());
            NoConfigEnvironment env(make_named_values<no_config_environment::Params>(
                    value_for<n::accept_unstable>(true),
                    value_for<n::disable_metadata_cache>(false),
                    value_for<n::extra_accept_keywords>(""),
                    value_for<n::extra_params>(keys),
                    value_for<n::extra_repository_dirs>(extra_repository_dirs),
                    value_for<n::master_repository_name>(CommandLine::get_instance()->a_master_repository_name.argument()),
                    value_for<n::profiles_if_not_auto>(""),
                    value_for<n::repository_dir>(CommandLine::get_instance()->a_repository_directory.argument()),
                    value_for<n::repository_type>(no_config_environment::ncer_ebuild),
                    value_for<n::write_cache>(CommandLine::get_instance()->a_write_cache_dir.argument())
                    ));

            std::tr1::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(
                        generator::InRepository(env.main_repository()->name()))]);
            std::multimap<std::tr1::shared_ptr<const PackageID>, std::string, PackageIDComparator> results(
                    PackageIDComparator(env.package_database().get()));
            unsigned success(0), total(0);

            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                Context i_context("When fetching ID '" + stringify(**i) + "':");

                cout << "Processing " << colour(cl_package_name, stringify(**i)) << "..." << endl;
                ++total;

                OutputManagerFromEnvironment output_manager_holder(&env, *i, oe_exclusive);
                FetchAction a(make_named_values<FetchActionOptions>(
                            value_for<n::errors>(make_shared_ptr(new Sequence<FetchActionFailure>)),
                            value_for<n::exclude_unmirrorable>(true),
                            value_for<n::fetch_parts>(FetchParts() + fp_regulars + fp_unneeded),
                            value_for<n::ignore_unfetched>(false),
                            value_for<n::make_output_manager>(std::tr1::ref(output_manager_holder)),
                            value_for<n::safe_resume>(true)
                            ));

                try
                {
                    if ((*i)->supports_action(SupportsActionTest<FetchAction>()))
                    {
                        (*i)->perform_action(a);
                        ++success;
                        if (output_manager_holder.output_manager_if_constructed())
                            output_manager_holder.output_manager_if_constructed()->succeeded();
                    }
                    else
                        results.insert(std::make_pair(*i, "Does not support fetching"));
                }
                catch (const ActionFailedError & e)
                {
                    if (a.options.errors()->empty())
                    {
                        results.insert(std::make_pair(*i, e.message()));
                    }

                    for (Sequence<FetchActionFailure>::ConstIterator f(a.options.errors()->begin()),
                            f_end(a.options.errors()->end()) ; f != f_end ; ++f)
                    {
                        std::string r;
                        if ((*f).requires_manual_fetching())
                            r = "manual";

                        if ((*f).failed_automatic_fetching())
                        {
                            if (! r.empty())
                                r.append(", ");
                            r.append("could not fetch");
                        }

                        if (! (*f).failed_integrity_checks().empty())
                        {
                            if (! r.empty())
                                r.append(", ");
                            r.append((*f).failed_integrity_checks());
                        }

                        results.insert(std::make_pair(*i, (*f).target_file() + ": " + r));
                    }
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    results.insert(std::make_pair(*i, "Uncaught exception '" + e.message() + "' (" + e.what() + ")"));
                }
            }

            std::cout << std::endl;

            std::tr1::shared_ptr<SafeOFStream> outf;
            if (CommandLine::get_instance()->a_report_file.specified())
                outf.reset(new SafeOFStream(FSEntry(CommandLine::get_instance()->a_report_file.argument())));

            std::ostream & out(outf ? *outf : cout);

            out << colour(cl_heading, "Accerso results for ") << colour(cl_repository_name, env.main_repository()->name())
                << colour(cl_heading, " on " + pretty_print_time(time(0)) + ":") << endl << endl
                << total << " IDs, " << success << " successes, " << (total - success) << " failures" << endl << endl;

            int exit_status(0);
            std::tr1::shared_ptr<const PackageID> old_id;
            for (std::multimap<std::tr1::shared_ptr<const PackageID>, std::string, std::tr1::reference_wrapper<const PackageIDComparator> >::const_iterator
                    r(results.begin()), r_end(results.end()) ; r != r_end ; ++r)
            {
                exit_status |= 1;
                if ((! old_id) || (*old_id != *r->first))
                {
                    out << colour(cl_package_name, stringify(*r->first)) << ":" << endl;
                    old_id = r->first;
                }
                out << "  " << r->second << endl;
            }
            out << endl;

            return exit_status;
        }
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


