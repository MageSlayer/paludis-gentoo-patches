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
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/thread_pool.hh>
#include <paludis/util/mutex.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <sys/sysinfo.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>

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
            auto PALUDIS_ATTRIBUTE((unused)) v(k.value());
        }
    };

    void worker(Mutex & mutex, PackageIDSequence::ConstIterator & i, const PackageIDSequence::ConstIterator & i_end, bool & fail)
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
        ThreadPool pool;

        int n_procs(get_nprocs());
        if (n_procs < 1)
            n_procs = 1;

        for (int n(0), n_end(n_procs) ; n != n_end ; ++n)
            pool.create_thread(std::bind(&worker, std::ref(mutex), std::ref(i), std::cref(i_end), std::ref(fail)));
    }

    return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
GenerateMetadataCommand::make_doc_cmdline()
{
    return std::make_shared<GenerateMetadataCommandLine>();
}

