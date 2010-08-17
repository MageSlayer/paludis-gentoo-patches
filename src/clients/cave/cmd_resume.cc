/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include "cmd_resume.hh"
#include "cmd_resolve_cmdline.hh"
#include "cmd_execute_resolution.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "colours.hh"
#include "colour_formatter.hh"
#include "resume_data.hh"
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/string_list_stream.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job_state.hh>
#include <paludis/environment.hh>
#include <paludis/serialise.hh>

#include <iostream>
#include <cstdlib>
#include <algorithm>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
    struct ResumeCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_retry_options;
        args::SwitchArg a_retry_failed;
        args::SwitchArg a_retry_skipped;
        args::SwitchArg a_skip_failed;

        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineProgramOptions program_options;

        ResumeCommandLine() :
            g_retry_options(main_options_section(), "Retry Options", "Retry options. By default, 'cave resume' will "
                    "treat packages that were already skipped or already failed as skipped or failed, respectively."),
            a_retry_failed(&g_retry_options, "retry-failed", 'r', "Retry any job that has already failed", true),
            a_retry_skipped(&g_retry_options, "retry-skipped", 'R', "Retry any job that has already been skipped. Note that "
                    "the job will just be skipped again unless circumstances have changed.", true),
            a_skip_failed(&g_retry_options, "skip-failed", 's', "Skip any job that has already failed", true),
            execution_options(this),
            program_options(this)
        {
            execution_options.a_preserve_world.remove();
            add_usage_line("--resume-file state [ --retry-failed ] [ --retry-skipped ]");
        }

        virtual std::string app_name() const
        {
            return "cave resume";
        }

        virtual std::string app_synopsis() const
        {
            return "Resume a failed resolution from 'cave resolve'";
        }

        virtual std::string app_description() const
        {
            return "Resumes a failed resultion from 'cave resolve'. To enable resumes, use "
                "'cave resolve --resume-file state --execute', and then if errors occur, use "
                "'cave resume --resume-file state [ --retry-failed ] [ --retry-skipped ]' to "
                "try again.";
        }
    };

    int perform_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<ResumeData> & data,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options)
    {
        Context context("When performing chosen resolution:");

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        for (args::ArgsSection::GroupsConstIterator g(program_options.begin()), g_end(program_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (args::ArgsSection::GroupsConstIterator g(execution_options.begin()), g_end(execution_options.end()) ;
                g != g_end ; ++g)
        {
            for (args::ArgsGroup::ConstIterator o(g->begin()), o_end(g->end()) ;
                    o != o_end ; ++o)
                if ((*o)->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f((*o)->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
        }

        for (Sequence<std::string>::ConstIterator p(data->targets()->begin()),
                p_end(data->targets()->end()) ;
                p != p_end ; ++p)
            args->push_back(*p);

        for (Sequence<std::string>::ConstIterator p(data->world_specs()->begin()),
                p_end(data->world_specs()->end()) ;
                p != p_end ; ++p)
        {
            args->push_back("--world-specs");
            args->push_back(*p);
        }

        if (data->preserve_world())
            args->push_back("--preserve-world");

        if (data->target_set())
            args->push_back("--set");

        if (program_options.a_execute_resolution_program.specified())
        {
            StringListStream ser_stream;
            Serialiser ser(ser_stream);
            data->job_lists()->serialise(ser);
            ser_stream.nothing_more_to_write();

            std::string command;
            if (program_options.a_execute_resolution_program.specified())
                command = program_options.a_execute_resolution_program.argument();
            else
                command = "$CAVE execute-resolution";

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            paludis::Command cmd(command);
            cmd
                .with_input_stream(&ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            become_command(cmd);
        }
        else
            return ExecuteResolutionCommand().run(env, args, data->job_lists());
    }

    struct StateVisitor
    {
        bool is_active;
        bool is_failed;
        bool is_skipped;

        StateVisitor() :
            is_active(false),
            is_failed(false),
            is_skipped(false)
        {
        }

        void visit(const JobPendingState &)
        {
        }

        void visit(const JobSkippedState &)
        {
            is_skipped = true;
        }

        void visit(const JobFailedState &)
        {
            is_failed = true;
        }

        void visit(const JobActiveState &)
        {
            is_active = true;
        }

        void visit(const JobSucceededState &)
        {
        }
    };

    void fix_lists(
            const std::shared_ptr<Environment> &,
            const ResumeCommandLine & cmdline,
            const std::shared_ptr<JobLists> & lists)
    {
        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
        {
            StateVisitor s;
            if ((*c)->state())
                (*c)->state()->accept(s);

            if (s.is_active)
                (*c)->set_state(std::make_shared<JobPendingState>());
            else if (cmdline.a_retry_failed.specified() && s.is_failed)
                (*c)->set_state(std::make_shared<JobPendingState>());
            else if (cmdline.a_skip_failed.specified() && s.is_failed)
                (*c)->set_state(std::make_shared<JobSkippedState>());
            else if (cmdline.a_retry_skipped.specified() && s.is_skipped)
                (*c)->set_state(std::make_shared<JobPendingState>());
        }
    }
}

bool
ResumeCommand::important() const
{
    return true;
}

int
ResumeCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ResumeCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_RESUME_OPTIONS", "CAVE_RESUME_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (! cmdline.execution_options.a_resume_file.specified())
        throw args::DoHelp("--" + cmdline.execution_options.a_resume_file.long_name() + " must be specified");

    std::shared_ptr<ResumeData> data;
    {
        FSEntry f(cmdline.execution_options.a_resume_file.argument());
        if (! f.exists())
            throw args::DoHelp("Resume file '" + stringify(f) + "' does not exist");
        SafeIFStream deser_stream(f);
        Deserialiser deserialiser(env.get(), deser_stream);
        Deserialisation deserialisation("ResumeData", deserialiser);
        data = ResumeData::deserialise(deserialisation);
    }

    fix_lists(env, cmdline, data->job_lists());
    return perform_resolution(env, data, cmdline.execution_options, cmdline.program_options);
}

std::shared_ptr<args::ArgsHandler>
ResumeCommand::make_doc_cmdline()
{
    return std::make_shared<ResumeCommandLine>();
}

