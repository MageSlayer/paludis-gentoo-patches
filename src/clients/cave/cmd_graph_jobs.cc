/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_graph_jobs.hh"
#include "resolve_cmdline.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include <paludis/args/do_help.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/join.hh>
#include <paludis/util/process.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_requirements.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
    struct GraphJobsCommandLine :
        CaveCommandCommandLine
    {
        ResolveCommandLineGraphJobsOptions graph_jobs_options;
        ResolveCommandLineImportOptions import_options;
        ResolveCommandLineProgramOptions program_options;

        GraphJobsCommandLine() :
            graph_jobs_options(this),
            import_options(this),
            program_options(this)
        {
            add_environment_variable("PALUDIS_SERIALISED_RESOLUTION_FD",
                    "The file descriptor on which the serialised resolution can be found.");
        }

        std::string app_name() const override
        {
            return "cave graph-jobs";
        }

        std::string app_synopsis() const override
        {
            return "Creates a Graphviz graph for jobs in a resolution created using 'cave resolve'.";
        }

        std::string app_description() const override
        {
            return "Creates a Graphviz graph for jobs in a resolution created using 'cave resolve'. Mostly for "
                "internal use; most users will not use this command directly.";
        }
    };

    std::string short_spec(const PackageDepSpec & p, bool full)
    {
        if (full || ! p.package_ptr())
            return stringify(p);

        std::string result(stringify(p.package_ptr()->package()));
        if (p.slot_requirement_ptr())
            result = result + stringify(*p.slot_requirement_ptr());
        if (p.in_repository_ptr())
            result = result + "::" + stringify(*p.in_repository_ptr());

        return result;
    }

    struct ShowOneJobAttrs
    {
        std::ostream & output_stream;
        bool full;

        void visit(const FetchJob & job) const
        {
            output_stream << "label=\"fetch " << short_spec(job.origin_id_spec(), full) << "\", ";
            output_stream << "shape=ellipse, ";
            output_stream << "fillcolor=cadetblue, ";
            output_stream << "style=filled";
        }

        void visit(const InstallJob & job) const
        {
            output_stream << "label=\"" << short_spec(job.origin_id_spec(), full) << " -> " << job.destination_repository_name() << "\"";
            switch (job.destination_type())
            {
                case dt_install_to_slash:
                    output_stream << "shape=box, ";
                    output_stream << "fillcolor=royalblue, ";
                    break;

                case dt_create_binary:
                    output_stream << "shape=octagon, ";
                    output_stream << "fillcolor=aquamarine3, ";
                    break;

                case dt_install_to_chroot:
                    output_stream << "shape=septagon, ";
                    output_stream << "fillcolor=steelblue, ";
                    break;

                case last_dt:
                    break;
            }

            output_stream << "style=filled";
        }

        void visit(const UninstallJob & job) const
        {
            output_stream << "label=\"uninstall " << join(job.ids_to_remove_specs()->begin(), job.ids_to_remove_specs()->end(), ", ",
                    std::bind(&short_spec, std::placeholders::_1, full)) << "\"";
            output_stream << "shape=hexagon, ";
            output_stream << "fillcolor=slateblue, ";
            output_stream << "style=filled";
        }
    };

    void graph_jobs(
            const std::shared_ptr<Environment> &,
            const GraphJobsCommandLine & cmdline,
            const std::shared_ptr<const JobList<ExecuteJob> > & execute_job_list,
            std::ostream & output_stream)
    {
        output_stream << "digraph Jobs {" << endl;
        output_stream << "    graph [ splines=true ]" << endl;
        output_stream << "    node [ fontsize=8, fontname=sans, height=0, width=0 ]" << endl;

        for (auto j(execute_job_list->begin()), j_end(execute_job_list->end()) ;
                j != j_end ; ++j)
        {
            output_stream << "    job" << execute_job_list->number(j) << " [ ";
            (*j)->accept(ShowOneJobAttrs{output_stream, cmdline.graph_jobs_options.a_graph_jobs_full_names.specified()});
            output_stream << " ]" << endl;

            for (const auto & requirement : *(*j)->requirements())
            {
                if (! cmdline.graph_jobs_options.a_graph_jobs_all_arrows.specified())
                    if (! (requirement.required_if() - jri_require_for_independent).any())
                        continue;

                output_stream << "    job" << execute_job_list->number(j) << " -> job" << requirement.job_number() << " [ ";
                if (requirement.required_if()[jri_require_always])
                    output_stream << " color=crimson";
                else if (requirement.required_if()[jri_require_for_satisfied])
                    output_stream << " color=indianred";
                else if (requirement.required_if()[jri_require_for_independent])
                    output_stream << " color=lightpink";

                if (! requirement.required_if()[jri_fetching])
                    output_stream << " style=dotted";

                output_stream << " ]" << endl;
            }

            output_stream << endl;
        }

        output_stream << "}" << endl;
    }

    int create_graph(
            const std::shared_ptr<Environment> &,
            const GraphJobsCommandLine & cmdline,
            const FSPath & src,
            const FSPath & dst)
    {
        Process process(ProcessCommand({ cmdline.program_options.a_graph_program.argument(),
                    "-T", cmdline.graph_jobs_options.a_graph_jobs_format.argument(),
                    stringify(src), "-o", stringify(dst)
                    }));

        return process.run().wait();
    }
}

int
GraphJobsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args,
        const std::shared_ptr<const Resolved> & maybe_resolved
        )
{
    GraphJobsCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_GRAPH_JOBS_OPTIONS", "CAVE_GRAPH_JOBS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.import_options.apply(env);

    std::shared_ptr<const Resolved> resolved(maybe_resolved);
    if (! resolved)
    {
        if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
            throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

        int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
        SafeIFStream deser_stream(fd);
        Deserialiser deserialiser(env.get(), deser_stream);
        Deserialisation deserialisation("Resolved", deserialiser);
        resolved = make_shared_copy(Resolved::deserialise(deserialisation));
        close(fd);
    }

    std::shared_ptr<SafeOFStream> stream_if_file;
    if (! cmdline.graph_jobs_options.a_graph_jobs_basename.argument().empty())
        stream_if_file = std::make_shared<SafeOFStream>(FSPath(cmdline.graph_jobs_options.a_graph_jobs_basename.argument() + ".graphviz"), -1, true);

    int retcode(0);

    graph_jobs(env, cmdline, resolved->job_lists()->execute_job_list(), stream_if_file ? *stream_if_file : cout);

    if (stream_if_file && ! cmdline.graph_jobs_options.a_graph_jobs_format.argument().empty())
        retcode = create_graph(env, cmdline,
                FSPath(cmdline.graph_jobs_options.a_graph_jobs_basename.argument() + ".graphviz"),
                FSPath(cmdline.graph_jobs_options.a_graph_jobs_basename.argument() + "." + cmdline.graph_jobs_options.a_graph_jobs_format.argument()));

    return retcode;
}

int
GraphJobsCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, nullptr);
}

std::shared_ptr<args::ArgsHandler>
GraphJobsCommand::make_doc_cmdline()
{
    return std::make_shared<GraphJobsCommandLine>();
}

CommandImportance
GraphJobsCommand::importance() const
{
    return ci_internal;
}

