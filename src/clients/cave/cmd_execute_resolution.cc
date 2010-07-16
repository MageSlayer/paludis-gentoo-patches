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
#include "cmd_perform.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "formats.hh"
#include "colour_formatter.hh"
#include "resume_data.hh"
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/simple_visitor-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/executor.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/timestamp.hh>
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
        args::StringSetArg a_world_specs;

        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineProgramOptions program_options;
        ResolveCommandLineImportOptions import_options;

        ExecuteResolutionCommandLine() :
            g_general_options(main_options_section(), "General Options", "General options."),
            a_pretend(&g_general_options, "pretend", '\0', "Only carry out the pretend action", false),
            a_set(&g_general_options, "set", '\0', "Our target is a set rather than package specs", false),
            a_world_specs(&g_general_options, "world-specs", '\0', "Use the specified spec or set name for updating world"),
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

    bool do_pretend(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const PackageDepSpec & origin_id_spec,
            const int x, const int y, const int prev_x,
            std::tr1::shared_ptr<OutputManager> & output_manager_goes_here)
    {
        Context context("When pretending for '" + stringify(origin_id_spec) + "':");

        const std::tr1::shared_ptr<const PackageID> origin_id(*(*env)[selection::RequireExactlyOne(
                    generator::Matches(origin_id_spec, MatchPackageOptions()))]->begin());

        if (0 != prev_x)
            cout << std::string(stringify(prev_x).length() + stringify(y).length() + 4, '\010');
        cout << x << " of " << y << std::flush;

        std::tr1::shared_ptr<Sequence<std::string> > args(new Sequence<std::string>);

        args->push_back("pretend");
        args->push_back("--hooks");
        args->push_back("--if-supported");
        args->push_back("--x-of-y");
        args->push_back(stringify(x) + " of " + stringify(y));
        args->push_back(stringify(origin_id->uniquely_identifying_spec()));

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

            IPCInputManager input_manager(env.get(), std::tr1::function<void (const std::tr1::shared_ptr<OutputManager> &)>());
            paludis::Command cmd(command);
            cmd
                .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
                ;

            retcode = run_command(cmd);
            const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
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

    void starting_action(
            const std::string & action,
            const std::tr1::shared_ptr<const PackageIDSequence> & ids,
            const int x, const int y)
    {
        cout << endl;
        cout << c::bold_blue() << x << " of " << y << ": Starting " << action << " for "
            << join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), ", ") << "..." << c::normal() << endl;
        cout << endl;
    }

    void done_action(
            const std::string & action,
            const std::tr1::shared_ptr<const PackageIDSequence> & ids,
            const bool success)
    {
        cout << endl;
        if (success)
            cout << c::bold_green() << "Done " << action << " for "
                << join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), ", ") << c::normal() << endl;
        else
            cout << c::bold_red() << "Failed " << action << " for "
                << join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), ", ") << c::normal() << endl;
        cout << endl;
    }

    void set_output_manager(
            Mutex & mutex,
            JobActiveState & active_state,
            const std::tr1::shared_ptr<OutputManager> & o)
    {
        Lock lock(mutex);
        active_state.set_output_manager(o);
    }

    bool do_fetch(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const PackageDepSpec & id_spec,
            const int x, const int y, bool normal_only,
            Mutex & job_mutex,
            JobActiveState & active_state)
    {
        Context context("When fetching for '" + stringify(id_spec) + "':");

        const std::tr1::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(
                    generator::Matches(id_spec, MatchPackageOptions()))]->begin());

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" fetch --hooks --if-supported --managed-output --output-exclusivity with-others ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

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

        IPCInputManager input_manager(env.get(), std::tr1::bind(&set_output_manager, std::tr1::ref(job_mutex),
                    std::tr1::ref(active_state), std::tr1::placeholders::_1));

        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        return 0 == retcode;
    }

    bool do_install(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const PackageDepSpec & id_spec,
            const RepositoryName & destination_repository_name,
            const std::tr1::shared_ptr<const Sequence<PackageDepSpec> > & replacing_specs,
            const std::string & destination_string,
            const int x, const int y,
            Mutex & job_mutex,
            JobActiveState & active_state)
    {
        const std::tr1::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(
                    generator::Matches(id_spec, MatchPackageOptions()))]->begin());

        Context context("When " + destination_string + " for '" + stringify(*id) + "':");

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" install --hooks --managed-output --output-exclusivity with-others ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --destination " + stringify(destination_repository_name));
        for (Sequence<PackageDepSpec>::ConstIterator i(replacing_specs->begin()),
                i_end(replacing_specs->end()) ;
                i != i_end ; ++i)
            command.append(" --replacing " + stringify(*i));

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

        IPCInputManager input_manager(env.get(), std::tr1::bind(&set_output_manager, std::tr1::ref(job_mutex),
                    std::tr1::ref(active_state), std::tr1::placeholders::_1));
        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        return 0 == retcode;
    }

    bool do_uninstall(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline,
            const PackageDepSpec & id_spec,
            const int x, const int y,
            Mutex & job_mutex,
            JobActiveState & active_state)
    {
        Context context("When removing '" + stringify(id_spec) + "':");

        const std::tr1::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(
                    generator::Matches(id_spec, MatchPackageOptions()))]->begin());

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" uninstall --hooks --managed-output --output-exclusivity with-others ");
        command.append(stringify(id->uniquely_identifying_spec()));

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

        IPCInputManager input_manager(env.get(), std::tr1::bind(&set_output_manager, std::tr1::ref(job_mutex),
                    std::tr1::ref(active_state), std::tr1::placeholders::_1));
        paludis::Command cmd(command);
        cmd
            .with_pipe_command_handler("PALUDIS_IPC", input_manager.pipe_command_handler())
            ;

        int retcode(run_command(cmd));
        const std::tr1::shared_ptr<OutputManager> output_manager(input_manager.underlying_output_manager_if_constructed());
        if (output_manager)
            output_manager->succeeded();

        return 0 == retcode;
    }

    void update_world(
            const std::tr1::shared_ptr<Environment> & env,
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
                        || *a == "installed-packages" || *a == "installed-slots")
                    cout << "* Special set '" << *a << "' does not belong in world" << endl;
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
                std::string aa(*a);
                if (aa.empty())
                    continue;

                if ('!' == aa.at(0))
                {
                    if (! removes)
                        continue;
                    aa.erase(0, 1);
                }
                else
                {
                    if (removes)
                        continue;
                }

                PackageDepSpec spec(parse_user_package_dep_spec(aa, env.get(), UserPackageDepSpecOptions() + updso_no_disambiguation));
                if (package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                                n::has_additional_requirements() = false,
                                n::has_category_name_part() = false,
                                n::has_from_repository() = false,
                                n::has_in_repository() = false,
                                n::has_installable_to_path() = false,
                                n::has_installable_to_repository() = false,
                                n::has_installed_at_path() = false,
                                n::has_package() = true,
                                n::has_package_name_part() = false,
                                n::has_slot_requirement() = false,
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
                        cout << "* Not removing '" << spec << "'" << endl;
                    else
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
    }

    void execute_update_world(
            const std::tr1::shared_ptr<Environment> & env,
            const ExecuteResolutionCommandLine & cmdline)
    {
        if (cmdline.execution_options.a_preserve_world.specified())
            return;

        cout << endl << c::bold_green() << "Updating world" << c::normal() << endl << endl;

        update_world(env, cmdline, true);
        update_world(env, cmdline, false);
    }

    int execute_pretends(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        bool failed(false);
        int x(0), y(lists->pretend_job_list()->length()), prev_x(0);

        if (0 != env->perform_hook(Hook("pretend_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        for (JobList<PretendJob>::ConstIterator c(lists->pretend_job_list()->begin()),
                c_end(lists->pretend_job_list()->end()) ;
                c != c_end ; ++c)
        {
            if (++x == 1)
                cout << "Executing pretend actions: " << std::flush;

            std::tr1::shared_ptr<OutputManager> output_manager_goes_here;
            failed = failed || ! do_pretend(env, cmdline, (*c)->origin_id_spec(), x, y, prev_x, output_manager_goes_here);
            prev_x = x;
        }

        if (0 != prev_x)
            cout << endl;

        if (0 != env->perform_hook(Hook("pretend_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return failed ? 1 : 0;
    }

    struct ExecuteCounts
    {
        Mutex mutex;
        int x_fetches, y_fetches, x_installs, y_installs;

        ExecuteCounts() :
            x_fetches(0),
            y_fetches(0),
            x_installs(0),
            y_installs(0)
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

    struct ExecuteOneVisitor
    {
        const std::tr1::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        ExecuteCounts & counts;
        Mutex & job_mutex;
        const ExecuteOneVisitorPart part;
        int retcode;

        ExecuteOneVisitor(
                const std::tr1::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                ExecuteCounts & k,
                Mutex & m,
                ExecuteOneVisitorPart p,
                int r) :
            env(e),
            cmdline(c),
            counts(k),
            job_mutex(m),
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

                case dt_create_binary:
                    destination_string = "creating binary";
                    action_string = "create binary";
                    break;

                case last_dt:
                    break;
            }

            if (destination_string.empty())
                throw InternalError(PALUDIS_HERE, "unhandled dt");

            const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::RequireExactlyOne(
                        generator::Matches(install_item.origin_id_spec(), MatchPackageOptions()))]);

            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_installs;
                        starting_action(action_string, ids, counts.x_installs, counts.y_installs);
                    }
                    break;

                case x1_main:
                    {
                        const std::tr1::shared_ptr<JobActiveState> active_state(new JobActiveState);
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state);
                        }

                        if (! do_fetch(env, cmdline, install_item.origin_id_spec(), counts.x_installs, counts.y_installs, false,
                                    job_mutex, *active_state))
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state->failed());
                            return 1;
                        }

                        if (! do_install(env, cmdline, install_item.origin_id_spec(), install_item.destination_repository_name(),
                                    install_item.replacing_specs(), destination_string,
                                    counts.x_installs, counts.y_installs, job_mutex, *active_state))
                        {
                            Lock lock(job_mutex);
                            install_item.set_state(active_state->failed());
                            return 1;
                        }

                        Lock lock(job_mutex);
                        install_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action(action_string, ids, 0 == retcode);
                    break;
            }

            return 0;
        }

        int visit(UninstallJob & uninstall_item)
        {
            const std::tr1::shared_ptr<PackageIDSequence> ids(new PackageIDSequence);
            for (Sequence<PackageDepSpec>::ConstIterator i(uninstall_item.ids_to_remove_specs()->begin()),
                    i_end(uninstall_item.ids_to_remove_specs()->end()) ;
                    i != i_end ; ++i)
            {
                const std::tr1::shared_ptr<const PackageID> id(*(*env)[selection::RequireExactlyOne(
                            generator::Matches(*i, MatchPackageOptions()))]->begin());
                ids->push_back(id);
            }

            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_installs;
                        starting_action("remove", ids, counts.x_installs, counts.y_installs);
                    }
                    break;

                case x1_main:
                    {
                        const std::tr1::shared_ptr<JobActiveState> active_state(new JobActiveState);
                        {
                            Lock lock(job_mutex);
                            uninstall_item.set_state(active_state);
                        }

                        for (Sequence<PackageDepSpec>::ConstIterator i(uninstall_item.ids_to_remove_specs()->begin()),
                                i_end(uninstall_item.ids_to_remove_specs()->end()) ;
                                i != i_end ; ++i)
                            if (! do_uninstall(env, cmdline, *i, counts.x_installs, counts.y_installs, job_mutex, *active_state))
                            {
                                Lock lock(job_mutex);
                                uninstall_item.set_state(active_state->failed());
                                return 1;
                            }

                        Lock lock(job_mutex);
                        uninstall_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action("remove", ids, 0 == retcode);
                    break;
            }

            return 0;
        }

        int visit(FetchJob & fetch_item)
        {
            const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::RequireExactlyOne(
                        generator::Matches(fetch_item.origin_id_spec(), MatchPackageOptions()))]);

            switch (part)
            {
                case x1_pre:
                    {
                        ++counts.x_fetches;
                        starting_action("fetch", ids, counts.x_fetches, counts.y_fetches);
                    }
                    break;

                case x1_main:
                    {
                        const std::tr1::shared_ptr<JobActiveState> active_state(new JobActiveState);
                        {
                            Lock lock(job_mutex);
                            fetch_item.set_state(active_state);
                        }

                        if (! do_fetch(env, cmdline, fetch_item.origin_id_spec(), counts.x_fetches, counts.y_fetches, true,
                                    job_mutex, *active_state))
                        {
                            Lock lock(job_mutex);
                            fetch_item.set_state(active_state->failed());
                            return 1;
                        }

                        Lock lock(job_mutex);
                        fetch_item.set_state(active_state->succeeded());
                    }
                    break;

                case x1_post:
                    done_action("fetch", ids, 0 == retcode);
                    break;
            }

            return 0;
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

    std::string stringify_id_or_spec(
            const std::tr1::shared_ptr<Environment> & env,
            const PackageDepSpec & spec)
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions()))]);
        if (ids->empty())
            return stringify(spec);
        else
            return stringify(**ids->begin());
    }

    struct AlreadyDoneVisitor
    {
        const std::tr1::shared_ptr<Environment> env;
        ExecuteCounts & counts;
        int x, y;
        std::string text;

        AlreadyDoneVisitor(
                const std::tr1::shared_ptr<Environment> & e,
                ExecuteCounts & c) :
            env(e),
            counts(c)
        {
        }

        void visit(const InstallJob & j)
        {
            text = "install " + stringify_id_or_spec(env, j.origin_id_spec());
            x = ++counts.x_installs;
            y = counts.y_installs;
        }

        void visit(const FetchJob & j)
        {
            text = "fetch " + stringify_id_or_spec(env, j.origin_id_spec());
            x = ++counts.x_fetches;
            y = counts.y_fetches;
        }

        void visit(const UninstallJob & j)
        {
            text = "uninstall " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ", ",
                    std::tr1::bind(&stringify_id_or_spec, env, std::tr1::placeholders::_1));
            x = ++counts.x_installs;
            y = counts.y_installs;
        }
    };

    void already_done_action(
            const std::tr1::shared_ptr<Environment> & env,
            const std::string & state,
            const std::tr1::shared_ptr<const ExecuteJob> & job,
            ExecuteCounts & counts)
    {
        AlreadyDoneVisitor v(env, counts);
        job->accept(v);
        cout << endl;
        cout << c::bold_green() << v.x << " of " << v.y << ":  Already " << state << " for "
            << v.text << "..." << c::normal() << endl;
        cout << endl;
    }

    struct MakeJobID
    {
        const std::tr1::shared_ptr<Environment> env;

        MakeJobID(const std::tr1::shared_ptr<Environment> & e) :
            env(e)
        {
        }

        std::string visit(const UninstallJob & j) const
        {
            return "Uninstall " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ",",
                    std::tr1::bind(stringify_id_or_spec, env, std::tr1::placeholders::_1));
        }

        std::string visit(const InstallJob & j) const
        {
            return "Install " + stringify_id_or_spec(env, j.origin_id_spec()) + " to " + stringify(j.destination_repository_name());
        }

        std::string visit(const FetchJob & j) const
        {
            return "Fetch " + stringify_id_or_spec(env, j.origin_id_spec());
        }
    };

    struct GetOutputManager
    {
        const std::tr1::shared_ptr<OutputManager> visit(const JobActiveState & s) const
        {
            return s.output_manager();
        }

        const std::tr1::shared_ptr<OutputManager> visit(const JobFailedState & s) const
        {
            return s.output_manager();
        }

        const std::tr1::shared_ptr<OutputManager> visit(const JobSucceededState & s) const
        {
            return s.output_manager();
        }

        const std::tr1::shared_ptr<OutputManager> visit(const JobSkippedState &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<OutputManager> visit(const JobPendingState &) const
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
        const std::tr1::shared_ptr<Environment> env;
        const ExecuteResolutionCommandLine & cmdline;
        const std::tr1::shared_ptr<ExecuteJob> job;
        const std::tr1::shared_ptr<const JobLists> lists;
        JobRequirementIf require_if;
        Mutex & retcode_mutex;
        int & retcode;
        ExecuteCounts & counts;
        std::string & old_heading;

        Timestamp last_flushed, last_output;

        Mutex job_mutex;

        bool want, already_done;

        ExecuteJobExecutive(
                const std::tr1::shared_ptr<Environment> & e,
                const ExecuteResolutionCommandLine & c,
                const std::tr1::shared_ptr<ExecuteJob> & j,
                const std::tr1::shared_ptr<const JobLists> & l,
                JobRequirementIf r,
                Mutex & m,
                int & rc,
                ExecuteCounts & k,
                std::string & h) :
            env(e),
            cmdline(c),
            job(j),
            lists(l),
            require_if(r),
            retcode_mutex(m),
            retcode(rc),
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
            return simple_visitor_cast<const FetchJob>(*job) ? "fetch" : "execute";
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
                if (! r->required_if()[jri_require_always])
                    continue;

                const std::tr1::shared_ptr<const ExecuteJob> req(*lists->execute_job_list()->fetch(r->job_number()));
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
                    {
                        Lock lock(retcode_mutex);
                        retcode = 1;
                    }
                    want = false;
                    already_done_action(env, "failed", job, counts);
                }
                else if (initial_state.skipped)
                {
                    {
                        Lock lock(retcode_mutex);
                        retcode = 1;
                    }
                    want = false;
                    already_done_action(env, "skipped", job, counts);
                }
                else if (initial_state.done)
                {
                    want = false;
                    already_done_action(env, "succeeded", job, counts);
                }
            }

            if ((0 != retcode) && want)
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

                        const std::tr1::shared_ptr<const ExecuteJob> req(*lists->execute_job_list()->fetch(r->job_number()));
                        want = want && req->state()->accept_returning<bool>(ContinueAfterState());
                    }
                }
            }

            already_done = initial_state.done;

            if (want)
            {
                ExecuteOneVisitor execute(env, cmdline, counts, job_mutex, x1_pre, retcode);
                retcode |= job->accept_returning<int>(execute);
            }
        }

        void execute_threaded()
        {
            if (want)
            {
                ExecuteOneVisitor execute(env, cmdline, counts, job_mutex, x1_main, retcode);
                retcode |= job->accept_returning<int>(execute);
            }
            else if (! already_done)
            {
                Lock lock(job_mutex);
                job->set_state(make_shared_ptr(new JobSkippedState));
            }
        }

        void display_active()
        {
            Lock lock(job_mutex);
            const std::tr1::shared_ptr<OutputManager> output_manager(
                    job->state()->accept_returning<std::tr1::shared_ptr<OutputManager> >(GetOutputManager()));

            if (output_manager)
            {
                std::string heading("[" + queue_name() + "] " + unique_id());

                if (output_manager->want_to_flush())
                {
                    if (heading != old_heading)
                    {
                        cout << endl << c::bold_blue() << heading << c::normal() << endl << endl;
                        old_heading = heading;
                    }

                    output_manager->flush();
                    last_output = Timestamp::now();
                }
                else
                {
                    cout << endl << c::bold_blue() << heading << c::normal() << endl << endl;
                    old_heading = "";
                    cout << "-> (no output for " << (Timestamp::now().seconds() - last_output.seconds()) << " seconds)" << endl;
                }

                last_flushed = Timestamp::now();
            }
        }

        void flush_threaded()
        {
            Lock lock(job_mutex);
            const std::tr1::shared_ptr<OutputManager> output_manager(
                    job->state()->accept_returning<std::tr1::shared_ptr<OutputManager> >(GetOutputManager()));

            if (output_manager && output_manager->want_to_flush())
                display_active();
            else
            {

                Timestamp now(Timestamp::now());
                if (now.seconds() - last_flushed.seconds() >= 10)
                    display_active();
            }
        }

        void post_execute_exclusive()
        {
            ExecuteOneVisitor execute(env, cmdline, counts, job_mutex, x1_post, retcode);
            retcode |= job->accept_returning<int>(execute);

            Lock lock(job_mutex);
            const std::tr1::shared_ptr<OutputManager> output_manager(
                    job->state()->accept_returning<std::tr1::shared_ptr<OutputManager> >(GetOutputManager()));

            if (output_manager)
            {
                if (output_manager->want_to_flush())
                    display_active();
                output_manager->nothing_more_to_come();
            }
        }
    };

    int execute_executions(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        int retcode(0);
        Mutex retcode_mutex;
        ExecuteCounts counts;

        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
            (*c)->accept(counts);

        if (0 != env->perform_hook(Hook("install_all_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
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
            executor.add(make_shared_ptr(new ExecuteJobExecutive(env, cmdline, *c, lists, require_if, retcode_mutex,
                            retcode, counts, old_heading)));

        executor.execute();

        if (0 != env->perform_hook(Hook("install_all_post")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        return retcode;
    }

    int execute_resolution_main(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        for (JobList<ExecuteJob>::ConstIterator c(lists->execute_job_list()->begin()),
                c_end(lists->execute_job_list()->end()) ;
                c != c_end ; ++c)
            if (! (*c)->state())
                (*c)->set_state(make_shared_ptr(new JobPendingState));

        int retcode(0);

        retcode |= execute_pretends(env, lists, cmdline);
        if (0 != retcode || cmdline.a_pretend.specified())
            return retcode;

        retcode |= execute_executions(env, lists, cmdline);

        if (0 != retcode)
            return retcode;

        execute_update_world(env, cmdline);

        return retcode;
    }

    struct SummaryNameVisitor
    {
        const std::tr1::shared_ptr<Environment> env;

        SummaryNameVisitor(const std::tr1::shared_ptr<Environment> & e) :
            env(e)
        {
        }

        std::string visit(const FetchJob & j) const
        {
            return "fetch " + stringify_id_or_spec(env, j.origin_id_spec());
        }

        std::string visit(const InstallJob & j) const
        {
            return "install " + stringify_id_or_spec(env, j.origin_id_spec()) + " to " + stringify(j.destination_repository_name());
        }

        std::string visit(const UninstallJob & j) const
        {
            return "uninstall " + join(j.ids_to_remove_specs()->begin(), j.ids_to_remove_specs()->end(), ", ",
                    std::tr1::bind(&stringify_id_or_spec, env, std::tr1::placeholders::_1));
        }

    };

    struct SummaryDisplayer
    {
        const std::tr1::shared_ptr<Environment> env;
        const std::tr1::shared_ptr<const ExecuteJob> job;
        const bool something_failed;
        bool & done_heading;

        SummaryDisplayer(
                const std::tr1::shared_ptr<Environment> & e,
                const std::tr1::shared_ptr<const ExecuteJob> & j,
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
                cout << endl << c::bold_blue() << "Summary:" << c::normal() << endl << endl;
            }
        }

        void visit(const JobActiveState &) const
        {
            need_heading();
            cout << c::bold_yellow() << "pending:   " << job->accept_returning<std::string>(
                    SummaryNameVisitor(env)) << c::normal() << endl;
        }

        void visit(const JobPendingState &) const
        {
            need_heading();
            cout << c::bold_yellow() << "pending:   " << job->accept_returning<std::string>(
                    SummaryNameVisitor(env)) << c::normal() << endl;
        }

        void visit(const JobSucceededState & s) const
        {
            if ((s.output_manager() && s.output_manager()->want_to_flush())
                    || (something_failed && ! simple_visitor_cast<const FetchJob>(*job)))
            {
                need_heading();
                cout << c::bold_green() << "succeeded: " << job->accept_returning<std::string>(
                        SummaryNameVisitor(env)) << c::normal() << endl;
            }
        }

        void visit(const JobFailedState &) const
        {
            need_heading();
            cout << c::bold_red() << "failed:    " << job->accept_returning<std::string>(
                    SummaryNameVisitor(env)) << c::normal() << endl;
        }

        void visit(const JobSkippedState &) const
        {
            if (! simple_visitor_cast<const FetchJob>(*job))
            {
                need_heading();
                cout << c::bold_yellow() << "skipped:   " << job->accept_returning<std::string>(
                        SummaryNameVisitor(env)) << c::normal() << endl;
            }
        }
    };

    void display_summary(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<JobLists> & lists,
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

    struct NotASuccess
    {
        bool operator() (const std::tr1::shared_ptr<const ExecuteJob> & job) const
        {
            return (! job->state()) || ! simple_visitor_cast<const JobSucceededState>(*job->state());
        }
    };

    void write_resume_file(
            const std::tr1::shared_ptr<Environment> &,
            const std::tr1::shared_ptr<JobLists> & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        if (! cmdline.execution_options.a_resume_file.specified())
            return;

        FSEntry resume_file(cmdline.execution_options.a_resume_file.argument());
        bool success(lists->execute_job_list()->end() == std::find_if(lists->execute_job_list()->begin(),
                    lists->execute_job_list()->end(), NotASuccess()));
        if (success)
        {
            cout << endl << "Erasing resume file " << resume_file << "..." << endl;
            resume_file.unlink();
        }
        else
        {
            std::tr1::shared_ptr<Sequence<std::string> > targets(new Sequence<std::string>);
            std::copy(cmdline.begin_parameters(), cmdline.end_parameters(), targets->back_inserter());

            std::tr1::shared_ptr<Sequence<std::string> > world_specs(new Sequence<std::string>);
            std::copy(cmdline.a_world_specs.begin_args(), cmdline.a_world_specs.end_args(), world_specs->back_inserter());

            ResumeData resume_data(make_named_values<ResumeData>(
                        n::job_lists() = lists,
                        n::preserve_world() = cmdline.execution_options.a_preserve_world.specified(),
                        n::target_set() = cmdline.a_set.specified(),
                        n::targets() = targets,
                        n::world_specs() = world_specs
                        ));

            cout << endl << "Writing resume information to " << resume_file << "..." << endl;
            SafeOFStream stream(resume_file);
            Serialiser ser(stream);
            resume_data.serialise(ser);
        }
    }

    int execute_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<JobLists> & lists,
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
            if ((! cmdline.a_pretend.specified()) || (0 == retcode))
                write_resume_file(env, lists, cmdline);

            if (! cmdline.a_pretend.specified())
                display_summary(env, lists, 0 != retcode);

            if (0 != env->perform_hook(Hook("install_task_execute_post")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                        ("PRETEND", stringify(cmdline.a_pretend.specified()))
                        ("SUCCESS", stringify(false))
                        ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");
            throw;
        }

        if ((! cmdline.a_pretend.specified()) || (0 == retcode))
            write_resume_file(env, lists, cmdline);

        if (! cmdline.a_pretend.specified())
            display_summary(env, lists, 0 != retcode);

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
        const std::tr1::shared_ptr<const Sequence<std::string > > & args,
        const std::tr1::shared_ptr<JobLists> & maybe_lists
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

    std::tr1::shared_ptr<JobLists> lists(maybe_lists);
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

    return execute_resolution(env, lists, cmdline);
}

int
ExecuteResolutionCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args)
{
    return run(env, args, make_null_shared_ptr());
}

std::tr1::shared_ptr<args::ArgsHandler>
ExecuteResolutionCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ExecuteResolutionCommandLine);
}

