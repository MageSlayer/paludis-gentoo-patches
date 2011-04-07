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

#include "cmd_execute_resolution.hh"
#include "resolve_cmdline.hh"
#include "cmd_perform.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "colours.hh"
#include "resume_data.hh"
#include "format_user_config.hh"

#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/executor.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/process.hh>

#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_state.hh>
#include <paludis/resolver/job_requirements.hh>

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
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/elike_blocker.hh>
#include <paludis/repository.hh>
#include <paludis/package_dep_spec_requirement.hh>

#include <set>
#include <iterator>
#include <iostream>
#include <list>
#include <cstdlib>
#include <algorithm>
#include <unordered_map>

using namespace paludis;
using namespace cave;
using namespace paludis::resolver;

using std::cout;
using std::endl;

namespace
{
#include "cmd_execute_resolution-fmt.hh"

    struct ExecuteResolutionCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_general_options;
        args::SwitchArg a_pretend;
        args::SwitchArg a_set;
        args::StringSetArg a_world_specs;
        args::StringSetArg a_removed_if_dependent_names;

        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineProgramOptions program_options;
        ResolveCommandLineImportOptions import_options;

        ExecuteResolutionCommandLine() :
            g_general_options(main_options_section(), "General Options", "General options."),
            a_pretend(&g_general_options, "pretend", '\0', "Only carry out the pretend action", false),
            a_set(&g_general_options, "set", '\0', "Our target is a set rather than package specs", false),
            a_world_specs(&g_general_options, "world-specs", '\0', "Use the specified spec or set name for updating world"),
            a_removed_if_dependent_names(&g_general_options, "removed-if-dependent-names", '\0',
                    "If nothing is left with the specified name, also remove it from world"),
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
            return "Executes a dependency resolution created using 'cave resolve'.";
        }

        virtual std::string app_description() const
        {
            return "Execute a dependency resolution created using 'cave resolve'. Mostly for "
                "internal use; most users will not use this command directly.";
        }
    };

    std::string lock_pipe_command(
            Mutex & mutex,
            ProcessPipeCommandFunction f,
            const std::string & s)
    {
        Lock lock(mutex);
        return f(s);
    }

    std::string stringify_id_or_spec(
            const std::shared_ptr<Environment> & env,
            const PackageDepSpec & spec)
    {
        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::Matches(spec, make_null_shared_ptr(), { }))]);
        if (ids->empty())
            return stringify(spec);
        else
            return stringify(**ids->begin());
    }

    struct NotASuccess
    {
        bool operator() (const std::shared_ptr<const ExecuteJob> & job) const
        {
            return (! job->state()) || ! visitor_cast<const JobSucceededState>(*job->state());
        }
    };

    void write_resume_file(
            const std::shared_ptr<Environment> &,
            const std::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline,
            const bool erase)
    {
        if (! cmdline.execution_options.a_resume_file.specified())
            return;

        FSPath resume_file(cmdline.execution_options.a_resume_file.argument());
        bool success(lists->execute_job_list()->end() == std::find_if(lists->execute_job_list()->begin(),
                    lists->execute_job_list()->end(), NotASuccess()));
        if (success)
        {
            if (erase)
            {
                cout << fuc(fs_erasing_resume_file(), fv<'f'>(stringify(resume_file)));
                resume_file.unlink();
            }
        }
        else
        {
            std::shared_ptr<Sequence<std::string> > targets(std::make_shared<Sequence<std::string>>());
            std::copy(cmdline.begin_parameters(), cmdline.end_parameters(), targets->back_inserter());

            std::shared_ptr<Sequence<std::string> > world_specs(std::make_shared<Sequence<std::string>>());
            std::copy(cmdline.a_world_specs.begin_args(), cmdline.a_world_specs.end_args(), world_specs->back_inserter());

            std::shared_ptr<Sequence<std::string> > removed_if_dependent_names(std::make_shared<Sequence<std::string>>());
            for (auto r(cmdline.a_removed_if_dependent_names.begin_args()), r_end(cmdline.a_removed_if_dependent_names.end_args()) ;
                    r != r_end ; ++r)
                world_specs->push_back("!" + *r);

            ResumeData resume_data(make_named_values<ResumeData>(
                        n::job_lists() = lists,
                        n::preserve_world() = cmdline.execution_options.a_preserve_world.specified(),
                        n::removed_if_dependent_names() = removed_if_dependent_names,
                        n::target_set() = cmdline.a_set.specified(),
                        n::targets() = targets,
                        n::world_specs() = world_specs
                        ));

            cout << fuc(fs_writing_resume_file(), fv<'f'>(stringify(resume_file)));
            SafeOFStream stream(resume_file, -1, true);
            Serialiser ser(stream);
            resume_data.serialise(ser);
        }
    }

    std::string make_x_of_y(const int x, const int y, const int f, const int s)
    {
        std::string result(stringify(x) + " of " + stringify(y));
        if (f != 0)
            result.append(", " + stringify(f) + " failed");
        if (s != 0)
            result.append(", " + stringify(s) + " skipped");
        return result;
    }

    bool do_pretend(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const PackageDepSpec & origin_id_spec,
            const RepositoryName & destination_repository_name,
            const int x, const int y, const int f, const int s,
            std::string & string_to_backspace,
            std::shared_ptr<OutputManager> & output_manager_goes_here)
    {
        Context context("When pretending for '" + stringify(origin_id_spec) + "':");

        if (! string_to_backspace.empty())
            cout << std::string(string_to_backspace.length(), '\010');
        string_to_backspace = make_x_of_y(x, y, f, s);
        cout << string_to_backspace << std::flush;

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        args->push_back("pretend");
        args->push_back("--hooks");
        args->push_back("--if-supported");
        args->push_back("--x-of-y");
        args->push_back(make_x_of_y(x, y, f, s));
        args->push_back(stringify(origin_id_spec));
        args->push_back("--destination");
        args->push_back(stringify(destination_repository_name));

        int retcode;

        if (cmdline.program_options.a_perform_program.specified())
        {
            args->push_back("--managed-output");

            if (cmdline.import_options.a_unpackaged_repository_params.specified())
            {
                for (args::StringSetArg::ConstIterator p(cmdline.import_options.a_unpackaged_repository_params.begin_args()),
                        p_end(cmdline.import_options.a_unpackaged_repository_params.end_args()) ;
                        p != p_end ; ++p)
                {
                    args->push_back("--" + cmdline.import_options.a_unpackaged_repository_params.long_name());
                    args->push_back(*p);
                }
            }

            std::string command(cmdline.program_options.a_perform_program.argument());
            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            IPCInputManager input_manager(env.get(), std::function<void (const std::shared_ptr<OutputManager> &)>());

            Process process(ProcessCommand({ "sh", "-c", command }));
            process.pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler());

            retcode = process.run().wait();
            const std::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
            if (output_manager)
            {
                output_manager->nothing_more_to_come();
                output_manager_goes_here = output_manager;
            }
        }
        else
        {
            PerformCommand command;
            retcode = command.run(env, args);
        }

        return 0 == retcode;
    }

    std::string maybe_replacing(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & id_specs,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & maybe_replacing_specs)
    {
        std::string r;

        if (maybe_replacing_specs)
            for (auto i(maybe_replacing_specs->begin()), i_end(maybe_replacing_specs->end()) ;
                    i != i_end ; ++i)
            {
                if (r.empty())
                    r = " replacing ";
                else
                    r.append(", ");

                const auto replacing_ids((*env)[selection::BestVersionOnly(generator::Matches(*i, make_null_shared_ptr(), { }))]);
                if (replacing_ids->empty())
                    r.append(stringify(*i));
                else if (id_specs->empty() || id_specs->begin()->package_name_requirement()->name() != (*replacing_ids->begin())->name())
                    r.append(stringify(**replacing_ids->begin()));
                else
                    r.append((*replacing_ids->begin())->canonical_form(idcf_no_name));
            }

        return r;
    }

    std::string maybe_ids(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & id_specs)
    {
        std::string r;

        if (id_specs)
            for (auto i(id_specs->begin()), i_end(id_specs->end()) ;
                    i != i_end ; ++i)
            {
                if (! r.empty())
                    r.append(", ");

                const auto ids((*env)[selection::BestVersionOnly(generator::Matches(*i, make_null_shared_ptr(), { }))]);
                if (ids->empty())
                    r.append(stringify(*i));
                else
                    r.append(stringify(**ids->begin()));
            }

        return r;
    }

    void starting_action(
            const std::shared_ptr<Environment> & env,
            const std::string & action,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & id_specs,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & maybe_replacing_specs,
            const int x, const int y, const int f, const int s)
    {
        cout << fuc(fs_starting_action(), fv<'x'>(make_x_of_y(x, y, f, s)),
                fv<'a'>(action), fv<'i'>(join(id_specs->begin(), id_specs->end(), ", ",
                        std::bind(&stringify_id_or_spec, env, std::placeholders::_1))),
                fv<'r'>(maybe_replacing(env, id_specs, maybe_replacing_specs)));
    }

    void done_action(
            const std::shared_ptr<Environment> & env,
            const std::string & action,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & ids,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & maybe_replacing_specs,
            const bool success)
    {
        cout << endl;
        if (success)
            cout << fuc(fs_done_action(),
                    fv<'a'>(action), fv<'i'>(maybe_ids(env, ids)),
                    fv<'r'>(maybe_replacing(env, ids, maybe_replacing_specs)));
        else
            cout << fuc(fs_failed_action(),
                    fv<'a'>(action), fv<'i'>(maybe_ids(env, ids)),
                    fv<'r'>(maybe_replacing(env, ids, maybe_replacing_specs)));
    }

    void set_output_manager(
            Mutex & mutex,
            JobActiveState & active_state,
            const std::shared_ptr<OutputManager> & o)
    {
        Lock lock(mutex);
        active_state.set_output_manager(o);
    }

    bool do_fetch(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs,
            const PackageDepSpec & id_spec,
            const int x, const int y, const int f, const int s, bool normal_only,
            Mutex & job_mutex,
            JobActiveState & active_state,
            Mutex & executor_mutex)
    {
        Context context("When fetching for '" + stringify(id_spec) + "':");

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" fetch --hooks --if-supported --managed-output ");
        if (0 != n_fetch_jobs)
            command.append("--output-exclusivity with-others --no-terminal-titles ");
        command.append(stringify(id_spec));
        command.append(" --x-of-y '" + make_x_of_y(x, y, f, s) + "'");

        if (normal_only)
            command.append(" --regulars-only --ignore-manual-fetch-errors");

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

        IPCInputManager input_manager(env.get(), std::bind(&set_output_manager, std::ref(job_mutex),
                    std::ref(active_state), std::placeholders::_1));

        Process process(ProcessCommand({ "sh", "-c", command }));
        process.pipe_command_handler("PALUDIS_IPC", std::bind(lock_pipe_command,
                    std::ref(executor_mutex), input_manager.pipe_command_handler(), std::placeholders::_1));

        int retcode(process.run().wait());
        return 0 == retcode;
    }

    bool do_install(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs,
            const PackageDepSpec & id_spec,
            const RepositoryName & destination_repository_name,
            const std::shared_ptr<const Sequence<PackageDepSpec> > & replacing_specs,
            const std::string & destination_string,
            const int x, const int y,
            const int f, const int s,
            Mutex & job_mutex,
            JobActiveState & active_state,
            Mutex & executor_mutex)
    {
        Context context("When " + destination_string + " for '" + stringify(id_spec) + "':");

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" install --hooks --managed-output ");
        if (0 != n_fetch_jobs)
            command.append("--output-exclusivity with-others ");
        command.append(stringify(id_spec));
        command.append(" --destination " + stringify(destination_repository_name));
        for (Sequence<PackageDepSpec>::ConstIterator i(replacing_specs->begin()),
                i_end(replacing_specs->end()) ;
                i != i_end ; ++i)
            command.append(" --replacing " + stringify(*i));

        command.append(" --x-of-y '" + make_x_of_y(x, y, f, s) + "'");

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

        IPCInputManager input_manager(env.get(), std::bind(&set_output_manager, std::ref(job_mutex),
                    std::ref(active_state), std::placeholders::_1));
        Process process(ProcessCommand({ "sh", "-c", command }));
        process.pipe_command_handler("PALUDIS_IPC", std::bind(lock_pipe_command,
                    std::ref(executor_mutex), input_manager.pipe_command_handler(), std::placeholders::_1));

        int retcode(process.run().wait());
        const std::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        return 0 == retcode;
    }

    bool do_uninstall(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs,
            const PackageDepSpec & id_spec,
            const int x, const int y,
            const int f, const int s,
            Mutex & job_mutex,
            JobActiveState & active_state,
            Mutex & executor_mutex)
    {
        Context context("When removing '" + stringify(id_spec) + "':");

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" uninstall --hooks --managed-output ");
        if (0 != n_fetch_jobs)
            command.append("--output-exclusivity with-others ");
        command.append(stringify(id_spec));

        command.append(" --x-of-y '" + make_x_of_y(x, y, f, s) + "'");

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

        IPCInputManager input_manager(env.get(), std::bind(&set_output_manager, std::ref(job_mutex),
                    std::ref(active_state), std::placeholders::_1));

        Process process(ProcessCommand({ "sh", "-c", command }));
        process.pipe_command_handler("PALUDIS_IPC", std::bind(lock_pipe_command,
                    std::ref(executor_mutex), input_manager.pipe_command_handler(), std::placeholders::_1));

        int retcode(process.run().wait());
        const std::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        if (output_manager)
            output_manager->succeeded();

        return 0 == retcode;
    }

    void update_world(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const bool removes)
    {
        std::string command(cmdline.program_options.a_update_world_program.argument());
        if (command.empty())
            command = "$CAVE update-world --verbose";

        if (removes)
            command.append(" --remove");

        bool any(false);
        if (cmdline.a_set.specified())
        {
            if (removes)
                return;

            command.append(" --set");
            for (args::StringSetArg::ConstIterator a(cmdline.a_world_specs.begin_args()),
                    a_end(cmdline.a_world_specs.end_args()) ;
                    a != a_end ; ++a)
            {
                if (*a == "world" || *a == "system" || *a == "security"
                        || *a == "everything" || *a == "insecurity"
                        || *a == "installed-packages" || *a == "installed-slots"
                        || *a == "nothing")
                    cout << fuc(fs_special_set_world(), fv<'a'>(*a));
                else
                {
                    any = true;
                    command.append(" " + *a);
                }
            }
        }
        else
        {
            for (args::StringSetArg::ConstIterator a(cmdline.a_world_specs.begin_args()),
                    a_end(cmdline.a_world_specs.end_args()) ;
                    a != a_end ; ++a)
            {
                auto p(split_elike_blocker(*a));
                switch (std::get<0>(p))
                {
                    case ebk_no_block:
                        if (removes)
                            continue;
                        break;

                    case ebk_single_bang:
                    case ebk_bang_question:
                    case ebk_double_bang:
                        if (! removes)
                            continue;
                        break;

                    case last_ebk:
                        throw InternalError(PALUDIS_HERE, "unhandled ebk");
                }

                PackageDepSpec spec(parse_user_package_dep_spec(std::get<2>(p), env.get(), { updso_no_disambiguation }));
                if (package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                                n::has_any_slot_requirement() = false,
                                n::has_category_name_part() = false,
                                n::has_choice_requirements() = false,
                                n::has_exact_slot_requirement() = false,
                                n::has_from_repository() = false,
                                n::has_in_repository() = false,
                                n::has_installable_to_path() = false,
                                n::has_installable_to_repository() = false,
                                n::has_installed_at_path() = false,
                                n::has_key_requirements() = false,
                                n::has_package() = true,
                                n::has_package_name_part() = false,
                                n::has_tag() = indeterminate,
                                n::has_version_requirements() = false
                                )))
                {
                    any = true;
                    command.append(" " + stringify(spec));
                }
                else
                {
                    if (removes)
                        cout << fuc(fs_not_removing_world(), fv<'a'>(stringify(spec)));
                    else
                        cout << fuc(fs_not_adding_world(), fv<'a'>(stringify(spec)));
                }
            }
        }

        if (any)
        {
            Process process(ProcessCommand({ "sh", "-c", command }));
            if (0 != process.run().wait())
                throw ActionAbortedError("Updating world failed");
        }
    }

    void execute_update_world(
            const std::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline)
    {
        if (cmdline.execution_options.a_preserve_world.specified() || cmdline.execution_options.a_fetch.specified())
            return;

        cout << fuc(fs_updating_world());

        update_world(env, cmdline, true);
        update_world(env, cmdline, false);

        bool any(false);
        std::string command(cmdline.program_options.a_update_world_program.argument());
        if (command.empty())
            command = "$CAVE update-world --verbose --remove --if-nothing-left ";

        for (args::StringSetArg::ConstIterator a(cmdline.a_removed_if_dependent_names.begin_args()),
                a_end(cmdline.a_removed_if_dependent_names.end_args()) ;
                a != a_end ; ++a)
        {
            any = true;
            command.append(" " + stringify(*a));
        }

        if (any)
        {
            Process process(ProcessCommand({ "sh", "-c", command }));
            if (0 != process.run().wait())
                throw ActionAbortedError("Updating world failed");
        }
    }

    int execute_pretends(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        bool failed(false);
        int x(0), y(lists->pretend_job_list()->length()), f(0), s(0);
        std::string string_to_backspace("");

        if (0 != env->perform_hook(Hook("pretend_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " ")),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        for (JobList<PretendJob>::ConstIterator c(lists->pretend_job_list()->begin()),
                c_end(lists->pretend_job_list()->end()) ;
                c != c_end ; ++c)
        {
            if (++x == 1)
                cout << "Executing pretend actions: " << std::flush;

            std::shared_ptr<OutputManager> output_manager_goes_here;
            if (! do_pretend(env, cmdline, (*c)->origin_id_spec(), (*c)->destination_repository_name(),
                    x, y, f, s, string_to_backspace, output_manager_goes_here))
            {
                failed = true;
                ++f;
            }
        }

        if (! string_to_backspace.empty())
            cout << endl;

        if (0 != env->perform_hook(Hook("pretend_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " ")),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return failed ? 1 : 0;
    }

    struct ExecuteCounts
    {
        Mutex mutex;
        int x_fetches, y_fetches, f_fetches, s_fetches, x_installs, y_installs, f_installs, s_installs;

        ExecuteCounts() :
            x_fetches(0),
            y_fetches(0),
            f_fetches(0),
            s_fetches(0),
            x_installs(0),
            y_installs(0),
            f_installs(0),
            s_installs(0)
        {
        }

        void visit(const FetchJob &)
        {
            Lock lock(mutex);
            ++y_fetches;
        }

        void visit(const InstallJob &)
        {
            Lock lock(mutex);
            ++y_installs;
        }

        void visit(const UninstallJob &)
        {
            Lock lock(mutex);
            ++y_installs;
        }
    };

    enum ExecuteOneVisitorPart
    {
        x1_pre,
        x1_main,
        x1_post
    };

    std::shared_ptr<Sequence<PackageDepSpec> > ensequence(const PackageDepSpec & s)
    {
        auto result(std::make_shared<Sequence<PackageDepSpec> >());
        result->push_back(s);
        return result;
    }

    struct ExecuteOneVisitor
    {
        const std::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        const int n_fetch_jobs;
        ExecuteCounts & counts;
        Mutex & job_mutex;
        Mutex & executor_mutex;
        const ExecuteOneVisitorPart part;
        int retcode;

        ExecuteOneVisitor(
                const std::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                const int n,
                ExecuteCounts & k,
                Mutex & m,
                Mutex & x,
                ExecuteOneVisitorPart p,
                int r) :
            env(e),
            cmdline(c),
            n_fetch_jobs(n),
            counts(k),
            job_mutex(m),
            executor_mutex(x),
            part(p),
            retcode(r)
        {
        }

        int visit(InstallJob & install_item)
        {
            std::string destination_string, action_string;
            switch (install_item.destination_type())
            {
                case dt_install_to_slash:
                    destination_string = "installing to /";
                    action_string = "install to /";
                    break;

                case dt_install_to_chroot:
                    destination_string = "installing to chroot";
                    action_string = "install to chroot";
                    break;

                case dt_create_binary:
                    destination_string = "creating binary";
                    action_string = "create binary in ::" + stringify(install_item.destination_repository_name());
                    break;

                case last_dt:
                    break;
            }

            if (destination_string.empty())
                throw InternalError(PALUDIS_HERE, "unhandled dt");

            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_installs;
                        starting_action(env, action_string, ensequence(install_item.origin_id_spec()),
                                install_item.replacing_specs(), counts.x_installs, counts.y_installs,
                                counts.f_installs, counts.s_installs);
                    }
                    break;

                case x1_main:
                    {
                        const std::shared_ptr<JobActiveState> active_state(std::make_shared<JobActiveState>());
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state);
                        }

                        if (! do_fetch(env, cmdline, n_fetch_jobs, install_item.origin_id_spec(), counts.x_installs, counts.y_installs,
                                    counts.f_installs, counts.s_installs, false,
                                    job_mutex, *active_state, executor_mutex))
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state->failed());
                            ++counts.f_installs;
                            return 1;
                        }

                        if (! do_install(env, cmdline, n_fetch_jobs, install_item.origin_id_spec(), install_item.destination_repository_name(),
                                    install_item.replacing_specs(), destination_string,
                                    counts.x_installs, counts.y_installs, counts.f_installs, counts.s_installs, job_mutex, *active_state, executor_mutex))
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state->failed());
                            ++counts.f_installs;
                            return 1;
                        }

                        Lock lock(job_mutex);
                        install_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action(env, action_string, ensequence(install_item.origin_id_spec()), install_item.replacing_specs(), 0 == retcode);
                    env->fetch_repository(install_item.destination_repository_name())->invalidate();
                    break;
            }

            return retcode;
        }

        int visit(UninstallJob & uninstall_item)
        {
            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_installs;
                        starting_action(env, "remove", uninstall_item.ids_to_remove_specs(), make_null_shared_ptr(), counts.x_installs, counts.y_installs,
                                counts.f_installs, counts.s_installs);
                    }
                    break;

                case x1_main:
                    {
                        const std::shared_ptr<JobActiveState> active_state(std::make_shared<JobActiveState>());
                        {
                            Lock lock(job_mutex);
                            uninstall_item.set_state(active_state);
                        }

                        for (Sequence<PackageDepSpec>::ConstIterator i(uninstall_item.ids_to_remove_specs()->begin()),
                                i_end(uninstall_item.ids_to_remove_specs()->end()) ;
                                i != i_end ; ++i)
                            if (! do_uninstall(env, cmdline, n_fetch_jobs, *i, counts.x_installs, counts.y_installs,
                                        counts.f_installs, counts.s_installs, job_mutex, *active_state, executor_mutex))
                            {
                                Lock lock(job_mutex);
                                uninstall_item.set_state(active_state->failed());
                                ++counts.f_installs;
                                return 1;
                            }

                        Lock lock(job_mutex);
                        uninstall_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action(env, "remove", uninstall_item.ids_to_remove_specs(), make_null_shared_ptr(), 0 == retcode);
                    break;
            }

            return retcode;
        }

        int visit(FetchJob & fetch_item)
        {
            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_fetches;
                        starting_action(env, "fetch", ensequence(fetch_item.origin_id_spec()), make_null_shared_ptr(), counts.x_fetches, counts.y_fetches,
                                counts.f_fetches, counts.s_fetches);
                    }
                    break;

                case x1_main:
                    {
                        const std::shared_ptr<JobActiveState> active_state(std::make_shared<JobActiveState>());
                        {
                            Lock lock(job_mutex);
                            fetch_item.set_state(active_state);
                        }

                        if (! do_fetch(env, cmdline, n_fetch_jobs, fetch_item.origin_id_spec(), counts.x_fetches, counts.y_fetches,
                                    counts.f_fetches, counts.s_fetches, true, job_mutex, *active_state, executor_mutex))
                        {
                            Lock lock(job_mutex);
                            fetch_item.set_state(active_state->failed());
                            ++counts.f_fetches;
                            return 1;
                        }

                        Lock lock(job_mutex);
                        fetch_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action(env, "fetch", ensequence(fetch_item.origin_id_spec()), make_null_shared_ptr(), 0 == retcode);
                    break;
            }

            return retcode;
        }
    };

    struct ContinueAfterState
    {
        bool visit(const JobPendingState &) const
        {
            /* it's still pending because it's a circular dep that we ended up ignoring */
            return true;
        }

        bool visit(const JobActiveState &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "still active? how did that happen?");
        }

        bool visit(const JobSucceededState &) const
        {
            return true;
        }

        bool visit(const JobFailedState &) const
        {
            return false;
        }

        bool visit(const JobSkippedState &) const
        {
            return false;
        }
    };

    struct ExistingStateVisitor
    {
        bool done;
        bool failed;
        bool skipped;

        ExistingStateVisitor() :
            done(false),
            failed(false),
            skipped(false)
        {
        }

        void visit(const JobPendingState &)
        {
        }

        void visit(const JobActiveState &)
        {
        }

        void visit(const JobFailedState &)
        {
            done = true;
            failed = true;
        }

        void visit(const JobSkippedState &)
        {
            done = true;
            skipped = true;
        }

        void visit(const JobSucceededState &)
        {
            done = true;
        }
    };

    struct AlreadyDoneVisitor
    {
        const std::shared_ptr<Environment> env;
        ExecuteCounts & counts;
        int * const fetch_sf;
        int * const install_sf;
        int x, y, f, s;
        std::string text;

        AlreadyDoneVisitor(
                const std::shared_ptr<Environment> & e,
                ExecuteCounts & c,
                int * const sf,
                int * const si) :
            env(e),
            counts(c),
            fetch_sf(sf),
            install_sf(si),
            x(0),
            y(0),
            f(0),
            s(0)
        {
        }

        void visit(const InstallJob & j)
        {
            text = "install " + stringify_id_or_spec(env, j.origin_id_spec());
            x = ++counts.x_installs;
            y = counts.y_installs;
            if (install_sf)
                ++*install_sf;
        }

        void visit(const FetchJob & j)
        {
            text = "fetch " + stringify_id_or_spec(env, j.origin_id_spec());
            x = ++counts.x_fetches;
            y = counts.y_fetches;
            if (fetch_sf)
                ++*fetch_sf;
        }

        void visit(const UninstallJob & j)
        {
            text = "uninstall " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ", ",
                    std::bind(&stringify_id_or_spec, env, std::placeholders::_1));
            x = ++counts.x_installs;
            y = counts.y_installs;
            if (install_sf)
                ++*install_sf;
        }
    };

    void already_done_action(
            const std::shared_ptr<Environment> & env,
            const std::string & state,
            const std::shared_ptr<const ExecuteJob> & job,
            ExecuteCounts & counts,
            int * const fetch_sf,
            int * const install_sf)
    {
        AlreadyDoneVisitor v(env, counts, fetch_sf, install_sf);
        job->accept(v);
        cout << fuc(fs_already_action(), fv<'x'>(make_x_of_y(v.x, v.y, v.f, v.s)), fv<'s'>(state), fv<'t'>(v.text));
    }

    struct MakeJobID
    {
        const std::shared_ptr<Environment> env;

        MakeJobID(const std::shared_ptr<Environment> & e) :
            env(e)
        {
        }

        std::string visit(const UninstallJob & j) const
        {
            return "uninstalling " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ", ",
                    std::bind(stringify_id_or_spec, env, std::placeholders::_1));
        }

        std::string visit(const InstallJob & j) const
        {
            return "installing " + stringify_id_or_spec(env, j.origin_id_spec()) + " to ::" + stringify(j.destination_repository_name())
                + maybe_replacing(env, ensequence(j.origin_id_spec()), j.replacing_specs());
        }

        std::string visit(const FetchJob & j) const
        {
            return "fetch " + stringify_id_or_spec(env, j.origin_id_spec());
        }
    };

    struct GetOutputManager
    {
        const std::shared_ptr<OutputManager> visit(const JobActiveState & s) const
        {
            return s.output_manager();
        }

        const std::shared_ptr<OutputManager> visit(const JobFailedState & s) const
        {
            return s.output_manager();
        }

        const std::shared_ptr<OutputManager> visit(const JobSucceededState & s) const
        {
            return s.output_manager();
        }

        const std::shared_ptr<OutputManager> visit(const JobSkippedState &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<OutputManager> visit(const JobPendingState &) const
        {
            return make_null_shared_ptr();
        }
    };

    struct CanStartState
    {
        bool visit(const JobSkippedState &) const
        {
            return true;
        }

        bool visit(const JobPendingState &) const
        {
            return false;
        }

        bool visit(const JobActiveState &) const
        {
            return false;
        }

        bool visit(const JobSucceededState &) const
        {
            return true;
        }

        bool visit(const JobFailedState &) const
        {
            return true;
        }
    };

    struct ExecuteJobExecutive :
        Executive
    {
        const std::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        Executor & executor;
        const int n_fetch_jobs;
        const std::shared_ptr<ExecuteJob> job;
        const std::shared_ptr<JobLists> lists;
        JobRequirementIf require_if;
        Mutex & global_retcode_mutex;
        int & global_retcode;
        int local_retcode;
        ExecuteCounts & counts;
        std::string & old_heading;

        Timestamp last_flushed, last_output;

        Mutex job_mutex;

        bool want, already_done;

        ExecuteJobExecutive(
                const std::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                Executor & x,
                const int n,
                const std::shared_ptr<ExecuteJob> & j,
                const std::shared_ptr<JobLists> & l,
                JobRequirementIf r,
                Mutex & m,
                int & rc,
                ExecuteCounts & k,
                std::string & h) :
            env(e),
            cmdline(c),
            executor(x),
            n_fetch_jobs(n),
            job(j),
            lists(l),
            require_if(r),
            global_retcode_mutex(m),
            global_retcode(rc),
            local_retcode(0),
            counts(k),
            old_heading(h),
            last_flushed(Timestamp::now()),
            last_output(last_flushed),
            want(true),
            already_done(false)
        {
        }

        std::string queue_name() const
        {
            if (0 != n_fetch_jobs)
                return visitor_cast<const FetchJob>(*job) ? "fetch" : "execute";
            else
                return "execute";
        }

        std::string unique_id() const
        {
            return job->accept_returning<std::string>(MakeJobID(env));
        }

        bool can_run() const
        {
            for (JobRequirements::ConstIterator r(job->requirements()->begin()), r_end(job->requirements()->end()) ;
                    r != r_end ; ++r)
            {
                if (! r->required_if()[jri_fetching])
                    continue;

                const std::shared_ptr<const ExecuteJob> req(*lists->execute_job_list()->fetch(r->job_number()));
                if (! req->state()->accept_returning<bool>(CanStartState()))
                    return false;
            }

            return true;
        }

        void pre_execute_exclusive()
        {
            last_flushed = Timestamp::now();
            last_output = last_flushed;

            ExistingStateVisitor initial_state;

            if (job->state())
            {
                job->state()->accept(initial_state);

                if (initial_state.failed)
                {
                    local_retcode = 1;
                    want = false;
                    already_done_action(env, "failed", job, counts, &counts.f_fetches, &counts.f_installs);
                }
                else if (initial_state.skipped)
                {
                    local_retcode = 1;
                    want = false;
                    already_done_action(env, "skipped", job, counts, &counts.s_fetches, &counts.s_installs);
                }
                else if (initial_state.done)
                {
                    want = false;
                    already_done_action(env, "succeeded", job, counts, 0, 0);
                }
            }

            if (want && cmdline.execution_options.a_fetch.specified())
                want = visitor_cast<const FetchJob>(*job);

            int current_global_retcode;
            {
                Lock lock(global_retcode_mutex);
                current_global_retcode = global_retcode;
            }

            if ((0 != local_retcode || 0 != current_global_retcode) && want)
            {
                if (last_jri == require_if)
                    want = false;
                else
                {
                    for (JobRequirements::ConstIterator r(job->requirements()->begin()), r_end(job->requirements()->end()) ;
                            r != r_end && want ; ++r)
                    {
                        if (! r->required_if()[require_if])
                            continue;

                        const std::shared_ptr<const ExecuteJob> req(*lists->execute_job_list()->fetch(r->job_number()));
                        want = want && req->state()->accept_returning<bool>(ContinueAfterState());
                    }
                }
            }

            already_done = initial_state.done;

            if (want)
            {
                ExecuteOneVisitor execute(env, cmdline, n_fetch_jobs, counts, job_mutex, executor.exclusivity_mutex(), x1_pre, local_retcode);
                int job_retcode(job->accept_returning<int>(execute));
                local_retcode |= job_retcode;
            }
        }

        void execute_threaded()
        {
            if (want)
            {
                ExecuteOneVisitor execute(env, cmdline, n_fetch_jobs, counts, job_mutex, executor.exclusivity_mutex(), x1_main, local_retcode);
                int job_retcode(job->accept_returning<int>(execute));
                local_retcode |= job_retcode;
            }
            else if (! already_done)
            {
                Lock lock(job_mutex);
                job->set_state(std::make_shared<JobSkippedState>());
            }
        }

        void display_active(const bool force)
        {
            if (n_fetch_jobs == 0)
                return;

            Lock lock(job_mutex);
            const std::shared_ptr<OutputManager> output_manager(
                    job->state()->accept_returning<std::shared_ptr<OutputManager> >(GetOutputManager()));

            if (output_manager)
            {
                std::string heading("Output from " + unique_id() + ":");

                if (output_manager->want_to_flush())
                {
                    bool really_flush(true);

                    if ((! force) && (heading != old_heading) && (! old_heading.empty()))
                    {
                        /* avoid hopping backwards and forwards between outputs too much */
                        if (Timestamp::now().seconds() - last_output.seconds() < 2)
                            really_flush = false;
                    }

                    if (really_flush)
                    {
                        if (heading != old_heading)
                        {
                            cout << fuc(fs_output_heading(), fv<'h'>(heading));
                            old_heading = heading;
                        }

                        output_manager->flush();
                        last_output = Timestamp::now();
                    }
                }
                else
                {
                    if ((force) || (executor.active() != 1))
                    {
                        cout << fuc(fs_output_heading(), fv<'h'>(heading));
                        old_heading = "";
                        cout << fuc(fs_no_output(), fv<'n'>(stringify(Timestamp::now().seconds() - last_output.seconds())));
                    }
                }

                last_flushed = Timestamp::now();
            }
        }

        void flush_threaded()
        {
            Lock lock(job_mutex);
            const std::shared_ptr<OutputManager> output_manager(
                    job->state()->accept_returning<std::shared_ptr<OutputManager> >(GetOutputManager()));

            if (output_manager && output_manager->want_to_flush())
                display_active(false);
            else
            {

                Timestamp now(Timestamp::now());
                if (now.seconds() - last_flushed.seconds() >= 10)
                    display_active(false);
            }
        }

        void post_execute_exclusive()
        {
            if (want)
            {
                ExecuteOneVisitor execute(env, cmdline, n_fetch_jobs, counts, job_mutex, executor.exclusivity_mutex(), x1_post, local_retcode);
                local_retcode |= job->accept_returning<int>(execute);

                Lock lock(job_mutex);
                const std::shared_ptr<OutputManager> output_manager(
                        job->state()->accept_returning<std::shared_ptr<OutputManager> >(GetOutputManager()));

                if (output_manager)
                {
                    if (output_manager->want_to_flush())
                        display_active(true);
                    output_manager->nothing_more_to_come();
                }

                old_heading = "";
            }

            {
                Lock lock(global_retcode_mutex);
                global_retcode |= local_retcode;
            }

            if (want)
                write_resume_file(env, lists, cmdline, false);
        }
    };

    int execute_executions(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs)
    {
        int retcode(0);
        Mutex retcode_mutex;
        ExecuteCounts counts;

        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
            (*c)->accept(counts);

        if (0 != env->perform_hook(Hook("install_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " ")),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        JobRequirementIf require_if(last_jri);
        if (cmdline.execution_options.a_continue_on_failure.argument() == "always")
            require_if = jri_require_always;
        else if (cmdline.execution_options.a_continue_on_failure.argument() == "if-satisfied")
            require_if = jri_require_for_satisfied;
        else if (cmdline.execution_options.a_continue_on_failure.argument() == "if-independent")
            require_if = jri_require_for_independent;
        else if (cmdline.execution_options.a_continue_on_failure.argument() == "never")
            require_if = last_jri;
        else
            throw args::DoHelp("Don't understand argument '"
                    + cmdline.execution_options.a_continue_on_failure.argument() + "' to '--"
                    + cmdline.execution_options.a_continue_on_failure.long_name() + "'");

        Executor executor(100);

        std::string old_heading;
        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
            executor.add(std::make_shared<ExecuteJobExecutive>(env, cmdline, executor, n_fetch_jobs, *c, lists, require_if, retcode_mutex,
                            retcode, counts, old_heading));

        executor.execute();

        if (0 != env->perform_hook(Hook("install_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " ")),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return retcode;
    }

    int execute_resolution_main(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs)
    {
        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
            if (! (*c)->state())
                (*c)->set_state(std::make_shared<JobPendingState>());

        int retcode(0);

        retcode |= execute_pretends(env, lists, cmdline);
        if (0 != retcode || cmdline.a_pretend.specified())
            return retcode;

        retcode |= execute_executions(env, lists, cmdline, n_fetch_jobs);

        if (0 != retcode)
            return retcode;

        execute_update_world(env, cmdline);

        return retcode;
    }

    struct SummaryNameVisitor
    {
        const std::shared_ptr<Environment> env;

        SummaryNameVisitor(const std::shared_ptr<Environment> & e) :
            env(e)
        {
        }

        std::string visit(const FetchJob & j) const
        {
            return "fetch " + stringify_id_or_spec(env, j.origin_id_spec());
        }

        std::string visit(const InstallJob & j) const
        {
            std::string r;
            if (! j.replacing_specs()->empty())
            {
                const auto origin_ids((*env)[selection::BestVersionOnly(generator::Matches(j.origin_id_spec(), make_null_shared_ptr(), { }))]);

                for (auto i(j.replacing_specs()->begin()), i_end(j.replacing_specs()->end()) ;
                        i != i_end ; ++i)
                {
                    if (r.empty())
                        r = " replacing ";
                    else
                        r.append(", ");

                    const auto ids((*env)[selection::BestVersionOnly(generator::Matches(*i, make_null_shared_ptr(), { }))]);
                    if (ids->empty())
                        r.append(stringify(*i));
                    else if (origin_ids->empty() || (*origin_ids->begin())->name() != (*ids->begin())->name())
                        r.append(stringify(**ids->begin()));
                    else
                        r.append((*ids->begin())->canonical_form(idcf_version));
                }
            }

            return "install " + stringify_id_or_spec(env, j.origin_id_spec()) + " to ::" + stringify(j.destination_repository_name()) + r;
        }

        std::string visit(const UninstallJob & j) const
        {
            return "uninstall " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ", ",
                    std::bind(&stringify_id_or_spec, env, std::placeholders::_1));
        }

    };

    struct SummaryDisplayer
    {
        const std::shared_ptr<Environment> env;
        const std::shared_ptr<const ExecuteJob> job;
        const bool something_failed;
        bool & done_heading;

        SummaryDisplayer(
                const std::shared_ptr<Environment> & e,
                const std::shared_ptr<const ExecuteJob> & j,
                const bool s,
                bool & b) :
            env(e),
            job(j),
            something_failed(s),
            done_heading(b)
        {
        }

        void need_heading() const
        {
            if (! done_heading)
            {
                done_heading = true;
                cout << fuc(fs_summary_heading());
            }
        }

        void visit(const JobActiveState &) const
        {
            need_heading();
            cout << fuc(fs_summary_job_pending(), fv<'s'>(job->accept_returning<std::string>(SummaryNameVisitor(env))));
        }

        void visit(const JobPendingState &) const
        {
            need_heading();
            cout << fuc(fs_summary_job_pending(), fv<'s'>(job->accept_returning<std::string>(SummaryNameVisitor(env))));
        }

        void visit(const JobSucceededState & s) const
        {
            if ((s.output_manager() && s.output_manager()->want_to_flush())
                    || (something_failed && ! visitor_cast<const FetchJob>(*job)))
            {
                need_heading();
                cout << fuc(fs_summary_job_succeeded(), fv<'s'>(job->accept_returning<std::string>(SummaryNameVisitor(env))));
            }
        }

        void visit(const JobFailedState &) const
        {
            need_heading();
            cout << fuc(fs_summary_job_failed(), fv<'s'>(job->accept_returning<std::string>(SummaryNameVisitor(env))));
        }

        void visit(const JobSkippedState &) const
        {
            if (! visitor_cast<const FetchJob>(*job))
            {
                need_heading();
                cout << fuc(fs_summary_job_skipped(), fv<'s'>(job->accept_returning<std::string>(SummaryNameVisitor(env))));
            }
        }
    };

    void display_summary(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<JobLists> & lists,
            const bool something_failed)
    {
        bool done_heading(false);

        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
        {
            (*c)->state()->accept(SummaryDisplayer(env, *c, something_failed, done_heading));
            (*c)->set_state(make_null_shared_ptr());
        }
    }

    int execute_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline,
            const int n_fetch_jobs)
    {
        Context context("When executing chosen resolution:");

        int retcode(0);
        if (0 != env->perform_hook(Hook("install_task_execute_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " ")),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        try
        {
            retcode = execute_resolution_main(env, lists, cmdline, n_fetch_jobs);
        }
        catch (...)
        {
            if ((! cmdline.a_pretend.specified()) || (0 == retcode))
                write_resume_file(env, lists, cmdline, true);

            if (! cmdline.a_pretend.specified())
                display_summary(env, lists, 0 != retcode);

            if (0 != env->perform_hook(Hook("install_task_execute_post")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                        ("PRETEND", stringify(cmdline.a_pretend.specified()))
                        ("SUCCESS", stringify(false)),
                        make_null_shared_ptr()).max_exit_status())
                throw ActionAbortedError("Aborted by hook");
            throw;
        }

        if ((! cmdline.a_pretend.specified()) || (0 == retcode))
            write_resume_file(env, lists, cmdline, true);

        if (! cmdline.a_pretend.specified())
            display_summary(env, lists, 0 != retcode);

        if (0 != env->perform_hook(Hook("install_task_execute_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ("PRETEND", stringify(cmdline.a_pretend.specified()))
                    ("SUCCESS", stringify(true)),
                    make_null_shared_ptr()).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return retcode;
    }
}

int
ExecuteResolutionCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args,
        const std::shared_ptr<JobLists> & maybe_lists
        )
{
    ExecuteResolutionCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_EXECUTE_RESOLUTION_OPTIONS", "CAVE_EXECUTE_RESOLUTION_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.import_options.apply(env);

    std::shared_ptr<JobLists> lists(maybe_lists);
    if (! lists)
    {
        if (getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "").empty())
            throw args::DoHelp("PALUDIS_SERIALISED_RESOLUTION_FD must be provided");

        int fd(destringify<int>(getenv_with_default("PALUDIS_SERIALISED_RESOLUTION_FD", "")));
        SafeIFStream deser_stream(fd);
        Deserialiser deserialiser(env.get(), deser_stream);
        Deserialisation deserialisation("JobLists", deserialiser);
        lists = JobLists::deserialise(deserialisation);
        close(fd);
    }

    int n_fetch_jobs;
    if (cmdline.execution_options.a_fetch_jobs.specified())
        n_fetch_jobs = cmdline.execution_options.a_fetch_jobs.argument();
    else if (cmdline.execution_options.a_fetch.specified())
        n_fetch_jobs = 0;
    else
        n_fetch_jobs = 1;

    return execute_resolution(env, lists, cmdline, n_fetch_jobs);
}

int
ExecuteResolutionCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, make_null_shared_ptr());
}

std::shared_ptr<args::ArgsHandler>
ExecuteResolutionCommand::make_doc_cmdline()
{
    return std::make_shared<ExecuteResolutionCommandLine>();
}

CommandImportance
ExecuteResolutionCommand::importance() const
{
    return ci_internal;
}

