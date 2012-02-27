/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include "cmd_generate_metadata.hh"
#include "format_string.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/thread_pool.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/notifier_callback.hh>
#include <sys/sysinfo.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <unistd.h>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct GenerateMetadataCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave generate-metadata";
        }

        virtual std::string app_synopsis() const
        {
            return "Pregenerate metadata for a set of IDs.";
        }

        virtual std::string app_description() const
        {
            return "Pregenerates metadata for a set of IDs.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_matching;

        GenerateMetadataCommandLine() :
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once."),
            a_matching(&g_filters, "matching", 'm', "Consider only IDs matching this spec. Note that certain specs "
                    "may force metadata generation anyway, e.g. to see whether a slot matches.",
                    args::StringSetArg::StringSetArgOptions())
        {
            add_usage_line("[ --matching spec ]");
        }
    };

    struct MetadataVisitor
    {
        void visit(const MetadataSectionKey & k) const
        {
            std::for_each(indirect_iterator(k.begin_metadata()), indirect_iterator(k.end_metadata()), accept_visitor(*this));
        }

        template <typename T_>
        void visit(const T_ & k) const
        {
            auto PALUDIS_ATTRIBUTE((unused)) v(k.parse_value());
        }
    };

    struct DoneOne
    {
    };

    struct DisplayCallback
    {
        mutable Mutex mutex;
        mutable std::map<std::string, int> metadata;
        mutable int steps;
        int total;
        mutable std::string stage;
        mutable unsigned width;

        bool output;

        DisplayCallback() :
            steps(0),
            total(-1),
            stage("Generating"),
            width(stage.length() + 2),
            output(::isatty(1))
        {
            if (output)
                cout << stage << ": " << std::flush;
        }

        ~DisplayCallback()
        {
            if (output)
                cout << endl << endl;
        }

        void update() const
        {
            if (! output)
                return;

            std::string s;
            s.append(stringify(steps));
            if (-1 != total)
                s.append("/" + stringify(total));

            if (! metadata.empty())
            {
                std::multimap<int, std::string> biggest;
                for (std::map<std::string, int>::const_iterator i(metadata.begin()), i_end(metadata.end()) ;
                        i != i_end ; ++i)
                    biggest.insert(std::make_pair(i->second, i->first));

                int t(0), n(0);
                std::string ss;
                for (std::multimap<int, std::string>::const_reverse_iterator i(biggest.rbegin()), i_end(biggest.rend()) ;
                        i != i_end ; ++i)
                {
                    ++n;

                    if (n == 4)
                        ss.append(", ...");

                    if (n < 4)
                    {
                        if (! ss.empty())
                            ss.append(", ");

                        ss.append(stringify(i->first) + " " + i->second);
                    }

                    t += i->first;
                }

                if (! s.empty())
                    s.append(", ");
                s.append(stringify(t) + " metadata (" + ss + ") ");
            }

            s = stage + ": " + s;
            std::cout << std::string(width, '\010') << s;

            if (width > s.length())
                std::cout
                    << std::string(width - s.length(), ' ')
                    << std::string(width - s.length(), '\010');

            width = s.length();
            std::cout << std::flush;
        }

        void operator() (const NotifierCallbackEvent & event) const
        {
            event.accept(*this);
        }

        void operator() (const DoneOne &) const
        {
            if (! output)
                return;

            Lock lock(mutex);
            ++steps;
            update();
        }

        void visit(const NotifierCallbackGeneratingMetadataEvent & e) const
        {
            if (! output)
                return;

            Lock lock(mutex);
            ++metadata.insert(std::make_pair(stringify(e.repository()), 0)).first->second;
            update();
        }

        void visit(const NotifierCallbackResolverStepEvent &) const
        {
        }

        void visit(const NotifierCallbackResolverStageEvent &) const
        {
        }

        void visit(const NotifierCallbackLinkageStepEvent &) const
        {
        }
    };

    void worker(Mutex & mutex, PackageIDSequence::ConstIterator & i, const PackageIDSequence::ConstIterator & i_end, bool & fail,
            DisplayCallback & display_callback)
    {
        while (true)
        {
            std::shared_ptr<const PackageID> id;
            {
                Lock lock(mutex);
                if (i != i_end)
                    id = *i++;
            }

            if (! id)
                return;

            for (PackageID::MetadataConstIterator m(id->begin_metadata()), m_end(id->end_metadata()); m_end != m; ++m)
                try
                {
                    MetadataVisitor v;
                    (*m)->accept(v);
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & e)
                {
                    Lock lock(mutex);
                    std::cerr << "When processing '" << **i << "' got exception '" << e.message() << "' (" << e.what() << ")" << std::endl;
                    fail = true;
                    break;
                }

                display_callback(DoneOne());
        }
    }
}

int
GenerateMetadataCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    GenerateMetadataCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_GENERATE_METADATA_OPTIONS", "CAVE_GENERATE_METADATA_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("generate-metadata takes no parameters");

    Generator g((generator::All()));
    if (cmdline.a_matching.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_matching.begin_args()),
                m_end(cmdline.a_matching.end_args()) ;
                m != m_end ; ++m)
        {
            PackageDepSpec s(parse_user_package_dep_spec(*m, env.get(), { updso_allow_wildcards }));
            g = g & generator::Matches(s, make_null_shared_ptr(), { });
        }
    }

    const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(g)]);
    bool fail(false);
    Mutex mutex;

    PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end());
    {
        DisplayCallback callback;
        callback.total = std::distance(ids->begin(), ids->end());
        ScopedNotifierCallback display_callback_holder(env.get(), NotifierCallbackFunction(std::cref(callback)));
        ThreadPool pool;

        int n_procs(get_nprocs());
        if (n_procs < 1)
            n_procs = 1;

        for (int n(0), n_end(n_procs) ; n != n_end ; ++n)
            pool.create_thread(std::bind(&worker, std::ref(mutex), std::ref(i), std::cref(i_end), std::ref(fail), std::ref(callback)));
    }

    return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
GenerateMetadataCommand::make_doc_cmdline()
{
    return std::make_shared<GenerateMetadataCommandLine>();
}

CommandImportance
GenerateMetadataCommand::importance() const
{
    return ci_supplemental;
}

