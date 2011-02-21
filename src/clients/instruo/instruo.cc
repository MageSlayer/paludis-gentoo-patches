/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/util/system.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/thread_pool.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    typedef std::multimap<std::shared_ptr<const PackageID>, std::string, PackageIDComparator> Results;

    struct KeyValidator
    {
        void visit(const MetadataValueKey<std::string> & k)
        {
            const std::string & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            const SlotName & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataValueKey<long> & k)
        {
            long PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            bool PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            const std::shared_ptr<const PackageID> & PALUDIS_ATTRIBUTE((unused)) p(k.value());
        }

        void visit(const MetadataTimeKey & k)
        {
            Timestamp PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > & k)
        {
            const std::shared_ptr<const Contents> & PALUDIS_ATTRIBUTE((unused)) c(k.value());
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            const FSPath & PALUDIS_ATTRIBUTE((unused)) c(k.value());
        }

        void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> >  & k)
        {
            const std::shared_ptr<const RepositoryMaskInfo> & PALUDIS_ATTRIBUTE((unused)) i(k.value());
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            const std::shared_ptr<const PlainTextSpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            const std::shared_ptr<const RequiredUseSpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            const std::shared_ptr<const ProvideSpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            const std::shared_ptr<const FetchableURISpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            const std::shared_ptr<const SimpleURISpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            const std::shared_ptr<const LicenseSpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            const std::shared_ptr<const DependencySpecTree> & PALUDIS_ATTRIBUTE((unused)) t(k.value());
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            const std::shared_ptr<const PackageIDSequence> & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            const std::shared_ptr<const Set<std::string> > & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            const std::shared_ptr<const Map<std::string, std::string> > & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            const std::shared_ptr<const Sequence<std::string> > & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            const std::shared_ptr<const FSPathSequence> & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            const std::shared_ptr<const KeywordNameSet> & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            const std::shared_ptr<const Choices> & PALUDIS_ATTRIBUTE((unused)) s(k.value());
        }

        void visit(const MetadataSectionKey & k)
        {
            std::for_each(indirect_iterator(k.begin_metadata()),
                    indirect_iterator(k.end_metadata()), accept_visitor(*this));
        }
    };

    void worker(Mutex & mutex, const std::shared_ptr<PackageIDSequence> & ids, CategoryNamePart & old_cat,
            unsigned & total, unsigned & success, Results & results)
    {
        while (true)
        {
            std::shared_ptr<const PackageID> id;
            {
                Lock lock(mutex);
                if (ids->empty())
                    return;
                id = *ids->begin();
                ids->pop_front();

                if (id->name().category() != old_cat)
                {
                    cout << "Processing " << colour(cl_package_name, stringify(id->name().category())) << "..." << endl;
                    old_cat = id->name().category();
                }

                ++total;
            }

            Context i_context("When generating metadata for ID '" + stringify(*id) + "':");

            try
            {
                PackageID::MetadataConstIterator eapi_i(id->find_metadata("EAPI"));
                if (id->end_metadata() == eapi_i)
                {
                    Lock lock(mutex);
                    results.insert(std::make_pair(id, "No EAPI metadata key"));
                    continue;
                }

                if (! visitor_cast<const MetadataValueKey<std::string> >(**eapi_i))
                {
                    Lock lock(mutex);
                    results.insert(std::make_pair(id, "EAPI metadata key is not a string key"));
                    continue;
                }

                if (visitor_cast<const MetadataValueKey<std::string> >(**eapi_i)->value() == "UNKNOWN")
                {
                    Lock lock(mutex);
                    results.insert(std::make_pair(id, "EAPI is 'UNKNOWN'"));
                    continue;
                }

                bool metadata_errors(false);
                KeyValidator v;
                for (PackageID::MetadataConstIterator m(id->begin_metadata()), m_end(id->end_metadata()); m_end != m; ++m)
                {
                    try
                    {
                        (*m)->accept(v);
                    }
                    catch (const InternalError &)
                    {
                        throw;
                    }
                    catch (const Exception & e)
                    {
                        Lock lock(mutex);
                        results.insert(std::make_pair(id, "Error in metadata key '" + (*m)->raw_name() + "': '" + e.message() +
                                    "' (" + e.what() + ")"));
                        metadata_errors = true;
                    }
                }

                if (! metadata_errors)
                {
                    Lock lock(mutex);
                    ++success;
                }
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Lock lock(mutex);
                results.insert(std::make_pair(id, "Uncaught exception '" + e.message() + "' (" + e.what() + ")"));
            }
        }
    }
}

int
main(int argc, char *argv[])
{
    std::string options(paludis::getenv_with_default("INSTRUO_OPTIONS", ""));
    if (! options.empty())
        options = "(" + options + ") ";
    options += join(argv + 1, argv + argc, " ");

    Context context(std::string("In program ") + argv[0] + " " + options + ":");

    try
    {
        CommandLine::get_instance()->run(argc, argv, "instruo", "INSTRUO_OPTIONS", "INSTRUO_CMDLINE");
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
                    CommandLine::get_instance()->a_generate_cache.specified() +
                    CommandLine::get_instance()->a_version.specified()
                ))
            throw args::DoHelp("you should specify exactly one action");

        if (! CommandLine::get_instance()->a_repository_directory.specified())
            CommandLine::get_instance()->a_repository_directory.set_argument(stringify(FSPath::cwd()));

        if (CommandLine::get_instance()->a_version.specified())
        {
            cout << "instruo, part of " << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
                << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO << PALUDIS_VERSION_SUFFIX;
            if (! std::string(PALUDIS_GIT_HEAD).empty())
                cout << " git " << PALUDIS_GIT_HEAD;
            cout << endl << endl;
            cout << "Paludis comes with ABSOLUTELY NO WARRANTY. Paludis is free software, and you" << endl;
            cout << "are welcome to redistribute it under the terms of the GNU General Public" << endl;
            cout << "License, version 2." << endl;

            return EXIT_SUCCESS;
        }

        if ((
                    CommandLine::get_instance()->a_repository_directory.specified() +
                    CommandLine::get_instance()->a_output_directory.specified()
            ) < 1)
            throw args::DoHelp("at least one of '--" + CommandLine::get_instance()->a_repository_directory.long_name() + "' or '--"
                    + CommandLine::get_instance()->a_output_directory.long_name() + "' must be specified");

        if (! CommandLine::get_instance()->a_output_directory.specified())
            CommandLine::get_instance()->a_output_directory.set_argument(stringify(FSPath::cwd()));

        std::shared_ptr<FSPathSequence> extra_repository_dirs(std::make_shared<FSPathSequence>());
        for (args::StringSequenceArg::ConstIterator d(CommandLine::get_instance()->a_extra_repository_dir.begin_args()),
                d_end(CommandLine::get_instance()->a_extra_repository_dir.end_args()) ;
                d != d_end ; ++d)
            extra_repository_dirs->push_back(FSPath(*d));

        std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
        keys->insert("append_repository_name_to_write_cache", "false");
        NoConfigEnvironment env(make_named_values<no_config_environment::Params>(
                    n::accept_unstable() = true,
                    n::disable_metadata_cache() = true,
                    n::extra_accept_keywords() = "",
                    n::extra_params() = keys,
                    n::extra_repository_dirs() = extra_repository_dirs,
                    n::master_repository_name() = CommandLine::get_instance()->a_master_repository_name.argument(),
                    n::profiles_if_not_auto() = "",
                    n::repository_dir() = CommandLine::get_instance()->a_repository_directory.argument(),
                    n::repository_type() = no_config_environment::ncer_ebuild,
                    n::write_cache() = CommandLine::get_instance()->a_output_directory.argument()
                ));

        std::shared_ptr<PackageIDSequence> ids(env[selection::AllVersionsSorted(
                    generator::InRepository(env.main_repository()->name()))]);
        Results results(env.package_database().get());
        unsigned success(0), total(0);
        CategoryNamePart old_cat("OLDCAT");
        Mutex mutex;

        {
            ThreadPool pool;
            for (int n(0), n_end(destringify<int>(getenv_with_default("INSTRUO_THREADS", "5"))) ; n < n_end ; ++n)
                pool.create_thread(std::bind(&worker, std::ref(mutex), std::ref(ids), std::ref(old_cat),
                            std::ref(total), std::ref(success), std::ref(results)));
        }

        std::cout << std::endl;

        std::shared_ptr<SafeOFStream> outf;
        if (CommandLine::get_instance()->a_report_file.specified())
            outf = std::make_shared<SafeOFStream>(FSPath(CommandLine::get_instance()->a_report_file.argument()), -1, true);

        std::ostream & out(outf ? *outf : cout);

        out << colour(cl_heading, "Instruo results for ") << colour(cl_repository_name, env.main_repository()->name())
            << colour(cl_heading, " on " + pretty_print_time(time(0)) + ":") << endl << endl
            << total << " IDs, " << success << " successes, " << (total - success) << " failures" << endl << endl;

        int exit_status(0);
        std::shared_ptr<const PackageID> old_id;
        for (Results::const_iterator r(results.begin()), r_end(results.end()) ; r != r_end ; ++r)
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

