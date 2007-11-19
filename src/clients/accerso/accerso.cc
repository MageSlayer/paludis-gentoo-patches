/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>
#include <iostream>
#include <fstream>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

int
main(int argc, char *argv[])
{
    Context context("In program " + join(argv, argv + argc, " ") + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "accerso", "ACCERSO_OPTIONS", "ACCERSO_CMDLINE");
        set_use_colour(
                (! CommandLine::get_instance()->a_no_color.specified()) &&
                (! CommandLine::get_instance()->a_report_file.specified()));

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

        if (! CommandLine::get_instance()->a_master_repository_dir.specified())
            CommandLine::get_instance()->a_master_repository_dir.set_argument("/var/empty");

        if (CommandLine::get_instance()->a_version.specified())
        {
            cout << "accerso, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
                << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
            if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
                cout << " svn " << PALUDIS_SUBVERSION_REVISION;
            cout << endl << endl;
            cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
            cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
            cout << "License, version 2." << endl;

            return EXIT_SUCCESS;
        }
        else
        {
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("distdir", CommandLine::get_instance()->a_download_directory.argument());
            NoConfigEnvironment env(no_config_environment::Params::create()
                    .repository_dir(CommandLine::get_instance()->a_repository_directory.argument())
                    .write_cache(CommandLine::get_instance()->a_write_cache_dir.argument())
                    .accept_unstable(true)
                    .repository_type(no_config_environment::ncer_ebuild)
                    .disable_metadata_cache(false)
                    .extra_params(keys)
                    .master_repository_dir(FSEntry(CommandLine::get_instance()->a_master_repository_dir.argument())));

            tr1::shared_ptr<const PackageIDSequence> ids(
                    env.package_database()->query(query::Repository(env.main_repository()->name()), qo_order_by_version));
            PackageIDComparator comparator(env.package_database().get());
            std::multimap<tr1::shared_ptr<const PackageID>, std::string, tr1::reference_wrapper<const PackageIDComparator> >
                results(tr1::cref(comparator));
            unsigned success(0), total(0);

            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                Context i_context("When fetching ID '" + stringify(**i) + "':");

                cout << "Processing " << colour(cl_package_name, stringify(**i)) << "..." << endl;
                ++total;

                try
                {
                    FetchAction a(FetchActionOptions::create()
                            .safe_resume(true)
                            .fetch_unneeded(true)
                            );
                    if ((*i)->supports_action(SupportsActionTest<FetchAction>()))
                    {
                        (*i)->perform_action(a);
                        ++success;
                    }
                    else
                        results.insert(std::make_pair(*i, "Does not support fetching"));
                }
                catch (const FetchActionError & e)
                {
                    for (Sequence<FetchActionFailure>::ConstIterator f(e.failures()->begin()), f_end(e.failures()->end()) ; f != f_end ; ++f)
                    {
                        std::string r;
                        if (f->requires_manual_fetching)
                            r = "manual";

                        if (f->failed_automatic_fetching)
                        {
                            if (! r.empty())
                                r.append(", ");
                            r.append("could not fetch");
                        }

                        if (! f->failed_integrity_checks.empty())
                        {
                            if (! r.empty())
                                r.append(", ");
                            r.append(f->failed_integrity_checks);
                        }

                        results.insert(std::make_pair(*i, f->target_file + ": " + r));
                    }
                }
                catch (const Exception & e)
                {
                    results.insert(std::make_pair(*i, "Uncaught exception '" + e.message() + "' (" + e.what() + ")"));
                }
            }

            std::cout << std::endl;

            tr1::shared_ptr<std::ofstream> outf;
            if (CommandLine::get_instance()->a_report_file.specified())
            {
                outf.reset(new std::ofstream(CommandLine::get_instance()->a_report_file.argument().c_str()));
                if (! *outf)
                {
                    std::cerr << "Cannot write to " << CommandLine::get_instance()->a_report_file.argument() << std::endl;
                    return EXIT_FAILURE;
                }
            }

            std::ostream & out(outf ? *outf : cout);

            char t[255];
            time_t tt(time(0));
            if (0 == strftime(t, 255, "%c", gmtime(&tt)))
                throw InternalError(PALUDIS_HERE, "strftime failed");

            out << colour(cl_heading, "Accerso results for ") << colour(cl_repository_name, env.main_repository()->name())
                << colour(cl_heading, " on " + stringify(t) + ":") << endl << endl
                << total << " IDs, " << success << " successes, " << (total - success) << " failures" << endl << endl;

            int exit_status(0);
            tr1::shared_ptr<const PackageID> old_id;
            for (std::multimap<tr1::shared_ptr<const PackageID>, std::string, tr1::reference_wrapper<const PackageIDComparator> >::const_iterator
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


