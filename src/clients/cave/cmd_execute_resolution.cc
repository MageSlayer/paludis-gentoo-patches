/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include "cmd_execute_resolution.hh"
#include "cmd_resolve_cmdline.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "formats.hh"
#include "colour_formatter.hh"
#include <paludis/args/do_help.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/simple_visitor-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/type_list.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/resolver_lists.hh>
#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/package_id.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/hook.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/serialise.hh>
#include <paludis/ipc_output_manager.hh>

#include <set>
#include <iterator>
#include <iostream>
#include <list>
#include <cstdlib>
#include <algorithm>
#include <tr1/unordered_map>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
    struct ExecuteResolutionCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_general_options;
        args::SwitchArg a_pretend;
        args::SwitchArg a_set;

        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineProgramOptions program_options;
        ResolveCommandLineImportOptions import_options;

        ExecuteResolutionCommandLine() :
            g_general_options(main_options_section(), "General Options", "General options."),
            a_pretend(&g_general_options, "pretend", '\0', "Only carry out the pretend action", false),
            a_set(&g_general_options, "set", '\0', "Our target is a set rather than package specs", false),
            execution_options(this),
            program_options(this),
            import_options(this)
        {
            add_environment_variable("PALUDIS_SERIALISED_RESOLUTION_FD",
                    "The file descriptor on which the serialised resolution can be found.");
        }

        virtual std::string app_name() const
        {
            return "cave execute-resolution";
        }

        virtual std::string app_synopsis() const
        {
            return "Executes a dependency resolution created using 'cave execute'.";
        }

        virtual std::string app_description() const
        {
            return "Execute a dependency resolution created using 'cave resolve'. Mostly for "
                "internal use; most users will not use this command directly.";
        }
    };

    struct JobPendingState;
    struct JobSucceededState;
    struct JobFailedState;
    struct JobSkippedState;

    class JobState :
        public virtual DeclareAbstractAcceptMethods<JobState, MakeTypeList<
            JobPendingState, JobSucceededState, JobFailedState, JobSkippedState>::Type >
    {
        private:
            const std::tr1::shared_ptr<const Job> _job;

        public:
            JobState(const std::tr1::shared_ptr<const Job> & j) :
                _job(j)
            {
            }

            virtual ~JobState() = 0;

            const std::tr1::shared_ptr<const Job> job() PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return _job;
            }

            virtual const std::string state() const = 0;
    };

    JobState::~JobState()
    {
    }

    class JobPendingState :
        public JobState,
        public ImplementAcceptMethods<JobState, JobPendingState>
    {
        public:
            JobPendingState(const std::tr1::shared_ptr<const Job> & j) :
                JobState(j)
            {
            }

            virtual const std::string state() const
            {
                return "pending";
            }
    };

    class JobSucceededState :
        public JobState,
        public ImplementAcceptMethods<JobState, JobSucceededState>
    {
        private:
            std::list<std::tr1::shared_ptr<OutputManager> > _output_managers;

        public:
            JobSucceededState(const std::tr1::shared_ptr<const Job> & j) :
                JobState(j)
            {
            }

            void add_output_manager(const std::tr1::shared_ptr<OutputManager> & o)
            {
                _output_managers.push_back(o);
            }

            virtual const std::string state() const
            {
                return "succeeded";
            }
    };

    class JobFailedState :
        public JobState,
        public ImplementAcceptMethods<JobState, JobFailedState>
    {
        private:
            std::list<std::tr1::shared_ptr<OutputManager> > _output_managers;

        public:
            JobFailedState(const std::tr1::shared_ptr<const Job> & j) :
                JobState(j)
            {
            }

            void add_output_manager(const std::tr1::shared_ptr<OutputManager> & o)
            {
                _output_managers.push_back(o);
            }

            virtual const std::string state() const
            {
                return "failed";
            }
    };

    class JobSkippedState :
        public JobState,
        public ImplementAcceptMethods<JobState, JobSkippedState>
    {
        private:
            const std::tr1::shared_ptr<OutputManager> _output_manager;
            const JobID _because;

        public:
            JobSkippedState(
                    const std::tr1::shared_ptr<const Job> & j,
                    const JobID & b) :
                JobState(j),
                _because(b)
            {
            }

            const JobID because()
            {
                return _because;
            }

            virtual const std::string state() const
            {
                return "skipped";
            }
    };

    bool do_pretend(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const int x, const int y,
            std::tr1::shared_ptr<OutputManager> & output_manager_goes_here)
    {
        Context context("When pretending for '" + stringify(*decision.origin_id()) + "':");

        if (x > 1)
            std::cout << std::string(stringify(x - 1).length() + stringify(y).length() + 4, '\010');
        std::cout << x << " of " << y << std::flush;

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" pretend --hooks --if-supported --managed-output ");
        command.append(stringify(decision.origin_id()->uniquely_identifying_spec()));
        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

        if (cmdline.import_options.a_unpackaged_repository_params.specified())
        {
            for (args::StringSetArg::ConstIterator p(cmdline.import_options.a_unpackaged_repository_params.begin_args()),
                    p_end(cmdline.import_options.a_unpackaged_repository_params.end_args()) ;
                    p != p_end ; ++p)
                command.append(" --" + cmdline.import_options.a_unpackaged_repository_params.long_name() + " '" + *p + "'");
        }

        IPCInputManager input_manager(env.get(), oe_exclusive);
        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        if (output_manager)
        {
            output_manager->nothing_more_to_come();
            output_manager_goes_here = output_manager;
        }

        return 0 == retcode;
    }

    void starting_action(
            const std::string & action,
            const ChangesToMakeDecision & decision,
            const int x, const int y)
    {
        cout << endl;
        cout << c::bold_blue() << x << " of " << y << ": Starting " << action << " for "
            << *decision.origin_id() << "..." << c::normal() << endl;
        cout << endl;
    }

    void done_action(
            const std::string & action,
            const ChangesToMakeDecision & decision,
            const bool success)
    {
        cout << endl;
        if (success)
            cout << c::bold_green() << "Done " << action << " for "
                << *decision.origin_id() << c::normal() << endl;
        else
            cout << c::bold_red() << "Failed " << action << " for "
                << *decision.origin_id() << c::normal() << endl;
        cout << endl;
    }

    bool do_fetch(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const int x, const int y, bool normal_only,
            std::tr1::shared_ptr<OutputManager> & output_manager_goes_here)
    {
        const std::tr1::shared_ptr<const PackageID> id(decision.origin_id());
        Context context("When fetching for '" + stringify(*id) + "':");

        starting_action("fetch (" + std::string(normal_only ? "regular parts" : "extra parts") + ")", decision, x, y);

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" fetch --hooks --if-supported --managed-output ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

        if (normal_only)
            command.append(" --regulars-only");

        if (cmdline.import_options.a_unpackaged_repository_params.specified())
        {
            for (args::StringSetArg::ConstIterator p(cmdline.import_options.a_unpackaged_repository_params.begin_args()),
                    p_end(cmdline.import_options.a_unpackaged_repository_params.end_args()) ;
                    p != p_end ; ++p)
                command.append(" --" + cmdline.import_options.a_unpackaged_repository_params.long_name() + " '" + *p + "'");
        }

        IPCInputManager input_manager(env.get(), oe_exclusive);
        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        if (output_manager)
        {
            output_manager->nothing_more_to_come();
            output_manager_goes_here = output_manager;
        }

        done_action("fetch (" + std::string(normal_only ? "regular parts" : "extra parts") + ")", decision, 0 == retcode);
        return 0 == retcode;
    }

    bool do_install(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const ChangesToMakeDecision & decision,
            const int x, const int y,
            std::tr1::shared_ptr<OutputManager> & output_manager_goes_here)
    {
        std::string destination_string, action_string;
        switch (resolution->resolvent().destination_type())
        {
            case dt_install_to_slash:
                destination_string = "installing to /";
                action_string = "install to /";
                break;

            case dt_create_binary:
                destination_string = "creating binary";
                action_string = "create binary";
                break;

            case last_dt:
                break;
        }

        if (destination_string.empty())
            throw InternalError(PALUDIS_HERE, "unhandled dt");

        const std::tr1::shared_ptr<const PackageID> id(decision.origin_id());
        Context context("When " + destination_string + " for '" + stringify(*id) + "':");

        starting_action(action_string, decision, x, y);

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" install --hooks --managed-output ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --destination " + stringify(decision.destination()->repository()));
        for (PackageIDSequence::ConstIterator i(decision.destination()->replacing()->begin()),
                i_end(decision.destination()->replacing()->end()) ;
                i != i_end ; ++i)
            command.append(" --replacing " + stringify((*i)->uniquely_identifying_spec()));

        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

        if (cmdline.execution_options.a_skip_phase.specified() || cmdline.execution_options.a_abort_at_phase.specified()
                || cmdline.execution_options.a_skip_until_phase.specified())
        {
            bool apply(false);

            if (cmdline.execution_options.a_change_phases_for.argument() == "all")
                apply = true;
            else if (cmdline.execution_options.a_change_phases_for.argument() == "first")
                apply = (x == 1);
            else if (cmdline.execution_options.a_change_phases_for.argument() == "last")
                apply = (x == y);
            else
                throw args::DoHelp("Don't understand argument '"
                        + cmdline.execution_options.a_change_phases_for.argument() + "' to '--"
                        + cmdline.execution_options.a_change_phases_for.long_name() + "'");

            if (apply)
            {
                if (cmdline.execution_options.a_skip_phase.specified())
                    command.append(" " + cmdline.execution_options.a_skip_phase.forwardable_string());
                if (cmdline.execution_options.a_abort_at_phase.specified())
                    command.append(" " + cmdline.execution_options.a_abort_at_phase.forwardable_string());
                if (cmdline.execution_options.a_skip_until_phase.specified())
                    command.append(" " + cmdline.execution_options.a_skip_until_phase.forwardable_string());
            }
        }

        if (cmdline.import_options.a_unpackaged_repository_params.specified())
        {
            for (args::StringSetArg::ConstIterator p(cmdline.import_options.a_unpackaged_repository_params.begin_args()),
                    p_end(cmdline.import_options.a_unpackaged_repository_params.end_args()) ;
                    p != p_end ; ++p)
                command.append(" --" + cmdline.import_options.a_unpackaged_repository_params.long_name() + " '" + *p + "'");
        }

        IPCInputManager input_manager(env.get(), oe_exclusive);
        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        if (output_manager)
        {
            output_manager->nothing_more_to_come();
            output_manager_goes_here = output_manager;
        }

        done_action(action_string, decision, 0 == retcode);
        return 0 == retcode;
    }

    struct JobCounts
    {
        int x_fetches, y_fetches, x_installs, y_installs;

        JobCounts() :
            x_fetches(0),
            y_fetches(0),
            x_installs(0),
            y_installs(0)
        {
        }

        void visit(const SimpleInstallJob &)
        {
            ++y_installs;
        }

        void visit(const FetchJob &)
        {
            ++y_fetches;
        }

        void visit(const UsableJob &)
        {
        }

        void visit(const UsableGroupJob &)
        {
        }

        void visit(const ErrorJob &)
        {
        }
    };

    typedef std::tr1::unordered_map<JobID, std::tr1::shared_ptr<JobState>, Hash<JobID> > JobStateMap;
    typedef std::list<std::tr1::shared_ptr<JobPendingState> > PendingJobsList;

    struct DoOneTakenVisitor
    {
        const std::tr1::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        JobCounts & counts;
        std::tr1::shared_ptr<JobState> & state;
        JobStateMap & job_state_map;

        DoOneTakenVisitor(
                const std::tr1::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                JobCounts & k,
                std::tr1::shared_ptr<JobState> & s,
                JobStateMap & m) :
            env(e),
            cmdline(c),
            counts(k),
            state(s),
            job_state_map(m)
        {
        }

        void visit(const SimpleInstallJob & job)
        {
            std::tr1::shared_ptr<OutputManager> fetch_output_manager_goes_here, install_output_manager_goes_here;

            ++counts.x_installs;

            /* not all of the fetch is done in the background */
            if (! do_fetch(env, cmdline, *job.decision(), counts.x_installs, counts.y_installs,
                        false, fetch_output_manager_goes_here))
            {
                std::tr1::shared_ptr<JobFailedState> failed_state(new JobFailedState(state->job()));
                if (fetch_output_manager_goes_here)
                    failed_state->add_output_manager(fetch_output_manager_goes_here);
                state = failed_state;
                return;
            }

            if (! do_install(env, cmdline, job.resolution(),
                        *job.decision(), counts.x_installs, counts.y_installs, install_output_manager_goes_here))
            {
                std::tr1::shared_ptr<JobFailedState> failed_state(new JobFailedState(state->job()));
                if (fetch_output_manager_goes_here)
                    failed_state->add_output_manager(fetch_output_manager_goes_here);
                if (install_output_manager_goes_here)
                    failed_state->add_output_manager(install_output_manager_goes_here);
                state = failed_state;
            }
            else
            {
                std::tr1::shared_ptr<JobSucceededState> succeeded_state(new JobSucceededState(state->job()));
                if (fetch_output_manager_goes_here)
                    succeeded_state->add_output_manager(fetch_output_manager_goes_here);
                if (install_output_manager_goes_here)
                    succeeded_state->add_output_manager(install_output_manager_goes_here);
                state = succeeded_state;
            }
        }

        void visit(const FetchJob & job)
        {
            std::tr1::shared_ptr<OutputManager> output_manager_goes_here;

            ++counts.x_fetches;
            if (! do_fetch(env, cmdline, *job.decision(), counts.x_fetches, counts.y_fetches,
                        true, output_manager_goes_here))
            {
                std::tr1::shared_ptr<JobFailedState> failed_state(new JobFailedState(state->job()));
                if (output_manager_goes_here)
                    failed_state->add_output_manager(output_manager_goes_here);
                state = failed_state;
            }
            else
            {
                std::tr1::shared_ptr<JobSucceededState> succeeded_state(new JobSucceededState(state->job()));
                if (output_manager_goes_here)
                    succeeded_state->add_output_manager(output_manager_goes_here);
                state = succeeded_state;
            }
        }

        void visit(const ErrorJob &)
        {
            state.reset(new JobFailedState(state->job()));
        }

        void visit(const UsableJob &)
        {
            state.reset(new JobSucceededState(state->job()));
        }

        void visit(const UsableGroupJob & job)
        {
            for (JobIDSequence::ConstIterator i(job.job_ids()->begin()), i_end(job.job_ids()->end()) ;
                    i != i_end ; ++i)
            {
                JobStateMap::iterator s(job_state_map.find(*i));
                if (s == job_state_map.end())
                    throw InternalError(PALUDIS_HERE, "missing state");

                DoOneTakenVisitor v(env, cmdline, counts, s->second, job_state_map);
                s->second->job()->accept(v);
            }

            state.reset(new JobSucceededState(state->job()));
        }
    };

    int execute_update_world(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists &,
            const ExecuteResolutionCommandLine & cmdline)
    {
        if (cmdline.execution_options.a_preserve_world.specified())
            return 0;

        cout << endl << c::bold_green() << "Updating world" << c::normal() << endl << endl;

        std::string command(cmdline.program_options.a_update_world_program.argument());
        if (command.empty())
            command = "$CAVE update-world";

        bool any(false);
        if (cmdline.a_set.specified())
        {
            command.append(" --set");
            for (args::ArgsHandler::ParametersConstIterator a(cmdline.begin_parameters()),
                    a_end(cmdline.end_parameters()) ;
                    a != a_end ; ++a)
            {
                if (*a == "world" || *a == "system" || *a == "security"
                        || *a == "everything" || *a == "insecurity"
                        || *a == "installed-packages" || *a == "installed-slots")
                    cout << "* Special set '" << *a << "' does not belong in world" << endl;
                else
                {
                    any = true;
                    cout << "* Adding '" << *a << "'" << endl;
                    command.append(" " + *a);
                }
            }
        }
        else
        {
            for (args::ArgsHandler::ParametersConstIterator a(cmdline.begin_parameters()),
                    a_end(cmdline.end_parameters()) ;
                    a != a_end ; ++a)
            {
                PackageDepSpec spec(parse_user_package_dep_spec(*a, env.get(), UserPackageDepSpecOptions()));
                if (package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                                value_for<n::has_additional_requirements>(false),
                                value_for<n::has_category_name_part>(false),
                                value_for<n::has_from_repository>(false),
                                value_for<n::has_in_repository>(false),
                                value_for<n::has_installable_to_path>(false),
                                value_for<n::has_installable_to_repository>(false),
                                value_for<n::has_installed_at_path>(false),
                                value_for<n::has_package>(true),
                                value_for<n::has_package_name_part>(false),
                                value_for<n::has_slot_requirement>(false),
                                value_for<n::has_tag>(indeterminate),
                                value_for<n::has_version_requirements>(false)
                                )))
                {
                    any = true;
                    cout << "* Adding '" << spec << "'" << endl;
                    command.append(" " + stringify(spec));
                }
                else
                {
                    cout << "* Not adding '" << spec << "'" << endl;
                }
            }
        }

        if (any)
        {
            paludis::Command cmd(command);
            if (0 != run_command(cmd))
                throw ActionAbortedError("Updating world failed");
        }

        return 0;
    }

    struct Populator
    {
        const std::tr1::shared_ptr<const Job> job;
        const ResolverLists & lists;
        JobStateMap & job_state_map;
        PendingJobsList * const pending;

        Populator(
                const std::tr1::shared_ptr<const Job> & j,
                const ResolverLists & l,
                JobStateMap & s,
                PendingJobsList * const p) :
            job(j),
            lists(l),
            job_state_map(s),
            pending(p)
        {
        }

        void common() const
        {
            const std::tr1::shared_ptr<JobPendingState> state(new JobPendingState(job));
            if (! job_state_map.insert(std::make_pair(job->id(), state)).second)
                throw InternalError(PALUDIS_HERE, "duplicate id");

            if (pending)
                pending->push_back(state);
        }

        void visit(const SimpleInstallJob &) const
        {
            common();
        }

        void visit(const UsableJob &) const
        {
            common();
        }

        void visit(const FetchJob &) const
        {
            common();
        }

        void visit(const ErrorJob &) const
        {
            common();
        }

        void visit(const UsableGroupJob & j) const
        {
            common();

            for (JobIDSequence::ConstIterator i(j.job_ids()->begin()), i_end(j.job_ids()->end()) ;
                    i != i_end ; ++i)
            {
                const std::tr1::shared_ptr<const Job> k(lists.jobs()->fetch(*i));
                k->accept(Populator(k, lists, job_state_map, 0));
            }
        }
    };

    int execute_taken(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        int retcode(0);

        JobCounts counts;
        JobStateMap job_state_map;
        PendingJobsList pending;

        for (JobIDSequence::ConstIterator c(lists.taken_job_ids()->begin()),
                c_end(lists.taken_job_ids()->end()) ;
                c != c_end ; ++c)
        {
            const std::tr1::shared_ptr<const Job> job(lists.jobs()->fetch(*c));
            job->accept(counts);
            job->accept(Populator(job, lists, job_state_map, &pending));
        }

        if (0 != env->perform_hook(Hook("install_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        for (PendingJobsList::iterator p(pending.begin()), p_next(p), p_end(pending.end()) ;
                p != p_end ; p = p_next)
        {
            ++p_next;

            for (ArrowSequence::ConstIterator a((*p)->job()->arrows()->begin()), a_end((*p)->job()->arrows()->end()) ;
                    a != a_end ; ++a)
            {
                const std::tr1::shared_ptr<const Job> after_job(lists.jobs()->fetch(a->comes_after()));
                JobStateMap::const_iterator s(job_state_map.find(after_job->id()));
                if (s == job_state_map.end())
                    throw InternalError(PALUDIS_HERE, "missing arrow: " + stringify(after_job->id().string_id()));

                if ((*p)->job()->used_existing_packages_when_ordering()->end() != std::find(
                            (*p)->job()->used_existing_packages_when_ordering()->begin(),
                            (*p)->job()->used_existing_packages_when_ordering()->end(),
                            a->comes_after()))
                {
                    if (! simple_visitor_cast<const JobPendingState>(*s->second))
                        throw InternalError(PALUDIS_HERE, "not pending: " + stringify((*p)->job()->id().string_id()) + " -> "
                                + stringify(a->comes_after().string_id()) + " " + s->second->state());
                }
                else
                {
                    if (! simple_visitor_cast<const JobSucceededState>(*s->second))
                        throw InternalError(PALUDIS_HERE, "didn't succeed: " + stringify((*p)->job()->id().string_id()) + " -> "
                                + stringify(a->comes_after().string_id()) + " " + s->second->state());
                }
            }

            JobStateMap::iterator s(job_state_map.find((*p)->job()->id()));
            if (s == job_state_map.end())
                throw InternalError(PALUDIS_HERE, "missing state");
            pending.erase(p);

            DoOneTakenVisitor v(env, cmdline, counts, s->second, job_state_map);
            s->second->job()->accept(v);

            if (! simple_visitor_cast<const JobSucceededState>(*s->second))
            {
                retcode |= 1;
                break;
            }
        }

        if (0 != env->perform_hook(Hook("install_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return retcode;
    }

    struct DoOnePretendVisitor
    {
        const std::tr1::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        JobCounts & counts;

        DoOnePretendVisitor(
                const std::tr1::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                JobCounts & k) :
            env(e),
            cmdline(c),
            counts(k)
        {
        }

        bool visit(const SimpleInstallJob & c) const
        {
            std::tr1::shared_ptr<OutputManager> output_manager_goes_here;
            return do_pretend(env, cmdline, *c.decision(), ++counts.x_installs, counts.y_installs,
                    output_manager_goes_here);
        }

        bool visit(const ErrorJob &) const
        {
            return true;
        }

        bool visit(const FetchJob &) const
        {
            return true;
        }

        bool visit(const UsableJob &) const
        {
            return true;
        }

        bool visit(const UsableGroupJob &) const
        {
            return true;
        }
    };

    int execute_pretends(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        bool failed(false);
        JobCounts counts;

        for (JobIDSequence::ConstIterator c(lists.taken_job_ids()->begin()),
                c_end(lists.taken_job_ids()->end()) ;
                c != c_end ; ++c)
            lists.jobs()->fetch(*c)->accept(counts);

        if (0 != env->perform_hook(Hook("pretend_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        std::cout << "Executing pretend actions: " << std::flush;

        for (JobIDSequence::ConstIterator c(lists.taken_job_ids()->begin()),
                c_end(lists.taken_job_ids()->end()) ;
                c != c_end ; ++c)
            failed = failed || ! lists.jobs()->fetch(*c)->accept_returning<bool>(DoOnePretendVisitor(env, cmdline, counts));

        cout << endl;

        if (0 != env->perform_hook(Hook("pretend_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return failed ? 1 : 0;
    }

    int execute_resolution_main(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        int retcode(0);

        retcode |= execute_pretends(env, lists, cmdline);
        if (0 != retcode || cmdline.a_pretend.specified())
            return retcode;

        retcode |= execute_taken(env, lists, cmdline);

        if (0 != retcode)
            return retcode;

        retcode |= execute_update_world(env, lists, cmdline);
        return retcode;
    }

    int execute_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        Context context("When executing chosen resolution:");

        int retcode(0);
        if (0 != env->perform_hook(Hook("install_task_execute_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        try
        {
            retcode = execute_resolution_main(env, lists, cmdline);
        }
        catch (...)
        {
            if (0 != env->perform_hook(Hook("install_task_execute_post")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                        ("PRETEND", stringify(cmdline.a_pretend.specified()))
                        ("SUCCESS", stringify(false))
                        ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");
            throw;
        }

        if (0 != env->perform_hook(Hook("install_task_execute_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ("PRETEND", stringify(cmdline.a_pretend.specified()))
                    ("SUCCESS", stringify(true))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return retcode;
    }
}

bool
ExecuteResolutionCommand::important() const
{
    return false;
}

int
ExecuteResolutionCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ExecuteResolutionCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_EXECUTE_RESOLUTION_OPTIONS", "CAVE_EXECUTE_RESOLUTION_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
        throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

    cmdline.import_options.apply(env);

    int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
    SafeIFStream deser_stream(fd);
    const std::string deser_str((std::istreambuf_iterator<char>(deser_stream)), std::istreambuf_iterator<char>());
    Deserialiser deserialiser(env.get(), deser_str);
    Deserialisation deserialisation("ResolverLists", deserialiser);
    ResolverLists lists(ResolverLists::deserialise(deserialisation));

    return execute_resolution(env, lists, cmdline);
}

std::tr1::shared_ptr<args::ArgsHandler>
ExecuteResolutionCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ExecuteResolutionCommandLine);
}

