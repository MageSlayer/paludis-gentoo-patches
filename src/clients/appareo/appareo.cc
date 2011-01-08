/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Kim Højgaard-Hansen
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_stat.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/util/set.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/package_database.hh>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

typedef std::multimap<std::shared_ptr<const PackageID>, std::string, PackageIDComparator> IDMap;

namespace
{
    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    FSPath get_location_and_add_filters()
    {
        Context context("When determining tree location:");

        if (CommandLine::get_instance()->a_repository_directory.specified())
            return FSPath(CommandLine::get_instance()->a_repository_directory.argument());

        if ((FSPath::cwd() / "profiles").stat().is_directory())
            return FSPath::cwd();

        if ((FSPath::cwd().dirname() / "profiles").stat().is_directory())
        {
            CommandLine::get_instance()->a_category.add_argument(FSPath::cwd().basename());
            CommandLine::get_instance()->a_category.set_specified(true);
            return FSPath::cwd().dirname();
        }

        if ((FSPath::cwd().dirname().dirname() / "profiles").stat().is_directory())
        {
            CommandLine::get_instance()->a_package.add_argument(FSPath::cwd().basename());
            CommandLine::get_instance()->a_package.set_specified(true);
            CommandLine::get_instance()->a_category.add_argument(FSPath::cwd().dirname().basename());
            CommandLine::get_instance()->a_category.set_specified(true);
            return FSPath::cwd().dirname().dirname();
        }

        throw ConfigurationError("Cannot find tree location (try specifying --repository-dir)");
     }

    bool fetch_ids(const std::shared_ptr<const PackageIDSequence> & ids, IDMap & results, unsigned & success, unsigned & total)
    {
        bool no_conflict_with_manifest(true);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            Context i_context("When fetching ID '" + stringify(**i) + "':");

            cout << "Processing " << colour(cl_package_name, stringify(**i)) << "..." << endl;
            ++total;

            const std::shared_ptr<Sequence<FetchActionFailure> > failures(std::make_shared<Sequence<FetchActionFailure>>());
            try
            {
                if ((*i)->supports_action(SupportsActionTest<FetchAction>()))
                {
                    FetchAction a(make_named_values<FetchActionOptions>(
                                n::errors() = failures,
                                n::exclude_unmirrorable() = false,
                                n::fetch_parts() = FetchParts() + fp_regulars + fp_extras + fp_unneeded,
                                n::ignore_not_in_manifest() = true,
                                n::ignore_unfetched() = false,
                                n::make_output_manager() = &make_standard_output_manager,
                                n::safe_resume() = true,
                                n::want_phase() = &want_all_phases
                                ));
                    (*i)->perform_action(a);
                    ++success;
                }
                else
                    results.insert(std::make_pair(*i, "Does not support fetching"));
            }
            catch (const ActionFailedError & e)
            {
                bool incremented_success(false);
                for (Sequence<FetchActionFailure>::ConstIterator f(failures->begin()),
                        f_end(failures->end()) ;
                        f != f_end ; ++f)
                {
                    std::string r;
                    if (f->requires_manual_fetching())
                        r = "manual";

                    if (f->failed_automatic_fetching())
                    {
                        if (! r.empty())
                            r.append(", ");
                        r.append("could not fetch");
                    }

                    if (! f->failed_integrity_checks().empty())
                    {
                        if (CommandLine::get_instance()->a_override.specified())
                        {
                            if (! incremented_success)
                                ++success;
                            incremented_success = true;
                        }
                        else
                        {
                            no_conflict_with_manifest = false;
                            if (! r.empty())
                                r.append(", ");
                            r.append(f->failed_integrity_checks());
                        }
                    }

                    if (! r.empty())
                        results.insert(std::make_pair(*i, f->target_file() + ": " + r));
                }

                if (failures->empty())
                    results.insert(std::make_pair(*i, "Unknown fetch error"));
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
        return no_conflict_with_manifest;
    }
}

int
main(int argc, char *argv[])
{
    std::string options(getenv_with_default("APPAREO_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "appareo", "APPAREO_OPTIONS", "APPAREO_CMDLINE");
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
                    CommandLine::get_instance()->a_manifest.specified() +
                    CommandLine::get_instance()->a_version.specified()
                ))
            throw args::DoHelp("you should specify exactly one action");

        if (! CommandLine::get_instance()->a_write_cache_dir.specified())
            CommandLine::get_instance()->a_write_cache_dir.set_argument("/var/empty");

        if (! CommandLine::get_instance()->a_repository_directory.specified())
            CommandLine::get_instance()->a_repository_directory.set_argument(stringify(FSPath::cwd()));

        if (CommandLine::get_instance()->a_version.specified())
        {
            cout << "appareo, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
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
            std::shared_ptr<FSPathSequence> extra_repository_dirs(std::make_shared<FSPathSequence>());
            for (args::StringSequenceArg::ConstIterator d(CommandLine::get_instance()->a_extra_repository_dir.begin_args()),
                        d_end(CommandLine::get_instance()->a_extra_repository_dir.end_args()) ;
                        d != d_end ; ++d)
                extra_repository_dirs->push_back(FSPath(*d));

            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("distdir", CommandLine::get_instance()->a_download_directory.argument());

            NoConfigEnvironment env(make_named_values<no_config_environment::Params>(
                    n::accept_unstable() = true,
                    n::disable_metadata_cache() = false,
                    n::extra_accept_keywords() = "",
                    n::extra_params() = keys,
                    n::extra_repository_dirs() = extra_repository_dirs,
                    n::master_repository_name() = CommandLine::get_instance()->a_master_repository_name.argument(),
                    n::profiles_if_not_auto() = "",
                    n::repository_dir() = get_location_and_add_filters(),
                    n::repository_type() = no_config_environment::ncer_ebuild,
                    n::write_cache() = CommandLine::get_instance()->a_write_cache_dir.argument()
                    ));

            IDMap results(PackageIDComparator(env.package_database().get()));

            unsigned success(0), total(0);

            std::shared_ptr<const CategoryNamePartSet> cat_names(env.main_repository()->category_names());
            for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                    c != c_end ; ++c)
            {
                if (CommandLine::get_instance()->a_category.specified())
                    if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                        continue;

                std::shared_ptr<const QualifiedPackageNameSet> pkg_names(env.main_repository()->package_names(*c));
                for (QualifiedPackageNameSet::ConstIterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                        p != p_end ; ++p)
                {
                    if (CommandLine::get_instance()->a_package.specified())
                        if (CommandLine::get_instance()->a_package.end_args() == std::find(
                                CommandLine::get_instance()->a_package.begin_args(),
                                CommandLine::get_instance()->a_package.end_args(),
                                stringify(p->package())))
                            continue;

                    const std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(
                            generator::Package(*p) & generator::InRepository(env.main_repository()->name()))]);

                    cout << "Making manifest for: " << colour(cl_package_name, stringify(*p)) << "..." << endl;
                    if (fetch_ids(ids, results, success, total) && env.main_repository()->manifest_interface())
                    {
                        try
                        {
                            env.main_repository()->manifest_interface()->make_manifest(*p);
                        }
                        catch (const MissingDistfileError &)
                        {
                            cerr << "Cannot make manifest for: " << colour(cl_package_name, stringify(*p)) << endl;
                        }
                    }
                    else
                        cerr << "Cannot make manifest for: " << colour(cl_package_name, stringify(*p)) << endl;
                }

            }

            cout << colour(cl_heading, "Appareo results for ") << colour(cl_repository_name, env.main_repository()->name())
                << ": " << total << " IDs, " << success << " successes, " << (total - success) << " failures" << endl << endl;

            int exit_status(0);
            std::shared_ptr<const PackageID> old_id;
            for (IDMap::const_iterator r(results.begin()), r_end(results.end()) ; r != r_end ; ++r)
            {
                exit_status |= 1;
                if ((! old_id) || (*old_id != *r->first))
                {
                    cout << colour(cl_package_name, stringify(*r->first)) << ":" << endl;
                    old_id = r->first;
                }
                cout << "  " << r->second << endl;
            }
            cout << endl;

            return exit_status;
        }
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

