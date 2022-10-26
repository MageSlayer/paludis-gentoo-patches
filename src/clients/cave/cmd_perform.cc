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

#include "cmd_perform.hh"
#include "resolve_cmdline.hh"
#include "exceptions.hh"
#include "format_user_config.hh"
#include "parse_spec_with_nice_error.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/hook.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <paludis/ipc_output_manager.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/stringify.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_perform-fmt.hh"

    struct PerformCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave perform";
        }

        std::string app_synopsis() const override
        {
            return "Perform an action upon a package.";
        }

        std::string app_description() const override
        {
            return "Perform an action upon a package. Not suitable for direct use, although it "
                "may be useful in some more complex scripts.";
        }

        args::ArgsGroup g_general_options;
        args::SwitchArg a_if_supported;
        args::SwitchArg a_hooks;
        args::StringArg a_x_of_y;
        args::SwitchArg a_no_terminal_titles;
        args::SwitchArg a_managed_output;
        args::EnumArg a_output_exclusivity;

        args::ArgsGroup g_fetch_action_options;
        args::SwitchArg a_exclude_unmirrorable;
        args::SwitchArg a_fetch_unneeded;
        args::SwitchArg a_ignore_unfetched;
        args::SwitchArg a_ignore_manual_fetch_errors;
        args::SwitchArg a_regulars_only;

        args::ArgsGroup g_install_action_options;
        args::StringArg a_destination;
        args::StringSetArg a_replacing;
        args::StringSetArg a_skip_phase;
        args::StringSetArg a_abort_at_phase;
        args::StringSetArg a_skip_until_phase;

        args::ArgsGroup g_uninstall_action_options;
        args::StringArg a_config_protect;

        ResolveCommandLineImportOptions import_options;

        PerformCommandLine() :
            g_general_options(main_options_section(), "General Options",
                    "General options for all actions"),
            a_if_supported(&g_general_options, "if-supported", '\0',
                    "If the action is not supported, exit silently with success rather than erroring.", true),
            a_hooks(&g_general_options, "hooks", '\0',
                    "Also execute the appropriate hooks for the action.", true),
            a_x_of_y(&g_general_options, "x-of-y", '\0',
                    "Specify the value of the X_OF_Y variable that is passed to hooks."),
            a_no_terminal_titles(&g_general_options, "no-terminal-titles", '\0',
                    "Do not change terminal titles", false),
            a_managed_output(&g_general_options, "managed-output", '\0',
                    "Specify that our output is being managed by another process. Used by "
                    "'cave execute-resolution'; not for end user use.", false),
            a_output_exclusivity(&g_general_options, "output-exclusivity", '\0',
                    "Specify the exclusivity of our output. Should not be changed unless "
                    "--managed-output is also specified",
                    args::EnumArg::EnumArgOptions
                    ("exclusive",       "Exclusive output")
                    ("with-others",     "With others")
                    ("background",      "Backgrounded"),
                    "exclusive"),

            g_fetch_action_options(main_options_section(), "Fetch Action Options",
                    "Options for if the action is 'fetch' or 'pretend-fetch'"),
            a_exclude_unmirrorable(&g_fetch_action_options, "exclude-unmirrorable", '\0',
                    "Do not include unmirrorable components", true),
            a_fetch_unneeded(&g_fetch_action_options, "fetch-unneeded", '\0',
                    "Also fetch components that aren't needed (e.g. because they are conditional upon "
                    "disabled options", true),
            a_ignore_unfetched(&g_fetch_action_options, "ignore-unfetched", '\0',
                    "Do not fetch any component that has not already been downloaded (but do verify "
                    "components that have already been downloaded", true),
            a_ignore_manual_fetch_errors(&g_fetch_action_options, "ignore-manual-fetch-errors", '\0',
                    "Ignore any errors that say that manual fetching is required for a component", true),
            a_regulars_only(&g_fetch_action_options, "regulars-only", '\0',
                    "Only fetch regular components. If this option is not specified, the job cannot safely "
                    "be backgrounded or run in parallel with installs", true),

            g_install_action_options(main_options_section(), "Install Action Options",
                    "Options for if the action is 'install' or (for --destination and --replacing) 'pretend'"),
            a_destination(&g_install_action_options, "destination", '\0',
                    "The name of the repository to which the install should take place"),
            a_replacing(&g_install_action_options, "replacing", '\0',
                    "A spec that uniquely identifies an ID to be replaced as part of the install. May be "
                    "specified multiple times."),
            a_skip_phase(&g_install_action_options, "skip-phase", '\0',
                    "Skip the named phases"),
            a_abort_at_phase(&g_install_action_options, "abort-at-phase", '\0',
                    "Abort when a named phase is encounted"),
            a_skip_until_phase(&g_install_action_options, "skip-until-phase", '\0',
                    "Skip every phase until a named phase is encounted"),

            g_uninstall_action_options(main_options_section(), "Uninstall Action Options",
                    "Options for if the action is 'uninstall'"),
            a_config_protect(&g_uninstall_action_options, "config-protect", '\0',
                    "Specify additional items to include in the config protection list"),

            import_options(this)
        {
            add_usage_line("config spec");
            add_usage_line("fetch | pretend-fetch [ --exclude-unmirrorable ] [ --fetch-unneeded ]"
                    " [ --ignore-unfetched ] spec");
            add_usage_line("info spec");
            add_usage_line("install --destination repo [ --replacing spec ... ] spec");
            add_usage_line("pretend --destination repo [ --replacing spec ... ] spec");
            add_usage_line("pretend-fetch spec");
            add_usage_line("uninstall [ --config-protect values ] spec");
        }
    };

    OutputExclusivity get_output_exclusivity(const PerformCommandLine & cmdline)
    {
        if (cmdline.a_output_exclusivity.argument() == "exclusive")
            return oe_exclusive;
        if (cmdline.a_output_exclusivity.argument() == "with-others")
            return oe_with_others;
        if (cmdline.a_output_exclusivity.argument() == "background")
            return oe_background;
        throw args::DoHelp("Don't understand argument '"
                + cmdline.a_output_exclusivity.argument() + "' to '--"
                + cmdline.a_output_exclusivity.long_name() + "'");
    }

    struct OutputManagerFromIPCOrEnvironment
    {
        std::shared_ptr<OutputManagerFromIPC> manager_if_ipc;
        std::shared_ptr<OutputManagerFromEnvironment> manager_if_env;

        OutputManagerFromIPCOrEnvironment(
                const Environment * const e,
                const PerformCommandLine & cmdline,
                const std::shared_ptr<const PackageID> & id)
        {
            if (cmdline.a_managed_output.specified())
                manager_if_ipc = std::make_shared<OutputManagerFromIPC>(e, id, get_output_exclusivity(cmdline),
                            ClientOutputFeatures() + cof_summary_at_end);
            else
                manager_if_env = std::make_shared<OutputManagerFromEnvironment>(e, id, get_output_exclusivity(cmdline),
                            ClientOutputFeatures() + cof_summary_at_end);
        }

        const std::shared_ptr<OutputManager> operator() (const Action & a)
        {
            if (manager_if_env)
                return (*manager_if_env)(a);
            else
                return (*manager_if_ipc)(a);
        }

        const std::shared_ptr<OutputManager> output_manager_if_constructed()
        {
            if (manager_if_env)
                return manager_if_env->output_manager_if_constructed();
            else
                return manager_if_ipc->output_manager_if_constructed();
        }

        void construct_standard_if_unconstructed()
        {
            if (manager_if_env)
                manager_if_env->construct_standard_if_unconstructed();
            else
                manager_if_ipc->construct_standard_if_unconstructed();
        }
    };

    void execute(
            const std::shared_ptr<Environment> & env,
            const PerformCommandLine & cmdline,
            const std::shared_ptr<const PackageID> & id,
            const std::string & action_name,
            Action & action,
            OutputManagerFromIPCOrEnvironment & output_manager_holder)
    {
        if (cmdline.a_x_of_y.specified() && ! cmdline.a_no_terminal_titles.specified())
            cout << fuc(fs_x_of_y_title(), fv<'x'>(cmdline.a_x_of_y.argument()),
                    fv<'i'>(stringify(*id)), fv<'a'>(action_name), fv<'c'>("")) << std::flush;

        if (cmdline.a_hooks.specified())
            if (0 != env->perform_hook(Hook(action_name + "_pre")
                        ("TARGET", stringify(*id))
                        ("X_OF_Y", cmdline.a_x_of_y.argument()),
                        output_manager_holder.output_manager_if_constructed()).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

        try
        {
            id->perform_action(action);
        }
        catch (const ActionFailedError & e)
        {
            if (cmdline.a_hooks.specified())
            {
                HookResult PALUDIS_ATTRIBUTE((unused)) dummy(env->perform_hook(Hook(action_name + "_fail")
                            ("TARGET", stringify(*id))
                            ("MESSAGE", e.message())
                            ("X_OF_Y", cmdline.a_x_of_y.argument()),
                            output_manager_holder.output_manager_if_constructed()));
            }

            throw;
        }

        if (cmdline.a_hooks.specified())
            if (0 != env->perform_hook(Hook(action_name + "_post")
                        ("TARGET", stringify(*id))
                        ("X_OF_Y", cmdline.a_x_of_y.argument()),
                        output_manager_holder.output_manager_if_constructed()).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

        if (cmdline.a_x_of_y.specified() && ! cmdline.a_no_terminal_titles.specified())
            cout << fuc(fs_x_of_y_title(), fv<'x'>(cmdline.a_x_of_y.argument()),
                    fv<'i'>(stringify(*id)), fv<'a'>(action_name), fv<'c'>("Completed ")) << std::flush;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }

    void perform_uninstall(
            const std::shared_ptr<Environment> & env,
            const PerformCommandLine & cmdline,
            const std::shared_ptr<const PackageID> & id,
            const std::string & action_name,
            const UninstallActionOptions & options,
            OutputManagerFromIPCOrEnvironment & output_manager_holder)
    {
        UninstallAction uninstall_action(options);
        execute(env, cmdline, id, "clean", uninstall_action, output_manager_holder);

        if (cmdline.a_x_of_y.specified() && ! cmdline.a_no_terminal_titles.specified())
            cout << fuc(fs_x_of_y_title(), fv<'x'>(cmdline.a_x_of_y.argument()),
                    fv<'i'>(stringify(*id)), fv<'a'>(action_name), fv<'c'>("")) << std::flush;
    }

    struct WantInstallPhase
    {
        const PerformCommandLine & cmdline;
        OutputManagerFromIPCOrEnvironment & output_manager_holder;
        bool done_any;

        WantInstallPhase(const PerformCommandLine & c, OutputManagerFromIPCOrEnvironment & o) :
            cmdline(c),
            output_manager_holder(o),
            done_any(false)
        {
        }

        WantPhase operator() (const std::string & phase)
        {
            output_manager_holder.construct_standard_if_unconstructed();
            std::shared_ptr<OutputManager> output_manager(output_manager_holder.output_manager_if_constructed());

            if (cmdline.a_abort_at_phase.end_args() != std::find(
                        cmdline.a_abort_at_phase.begin_args(), cmdline.a_abort_at_phase.end_args(), phase))
            {
                output_manager->stdout_stream() << "+++ Aborting at phase '" + phase + "' as instructed" << endl;
                return wp_abort;
            }

            if (cmdline.a_skip_until_phase.end_args() != cmdline.a_skip_until_phase.begin_args())
                if (! done_any)
                    if (cmdline.a_skip_until_phase.end_args() == std::find(
                                cmdline.a_skip_until_phase.begin_args(), cmdline.a_skip_until_phase.end_args(), phase))
                    {
                        output_manager->stdout_stream() << "+++ Skipping phase '" << phase <<
                            "' as instructed since it is before a start phase" << endl;
                        return wp_skip;
                    }

            /* make --skip-until-phase foo --skip-phase foo work */
            done_any = true;

            if (cmdline.a_skip_phase.end_args() != std::find(
                        cmdline.a_skip_phase.begin_args(), cmdline.a_skip_phase.end_args(), phase))
            {
                output_manager->stdout_stream() << "+++ Skipping phase '" + phase + "' as instructed" << endl;
                return wp_skip;
            }

            if (cmdline.a_skip_phase.specified() || cmdline.a_skip_until_phase.specified() ||
                    cmdline.a_abort_at_phase.specified())
                output_manager->stdout_stream() << "+++ Executing phase '" + phase + "' as instructed" << endl;

            return wp_yes;
        }
    };

    struct OurPretendFetchAction :
        PretendFetchAction
    {
        std::set<FSPath, FSPathComparator> already_downloaded;
        unsigned long size;
        bool overflow;

        OurPretendFetchAction(const FetchActionOptions & o) :
            PretendFetchAction(o),
            size(0),
            overflow(false)
        {
        }

        void will_fetch(const FSPath & destination, const unsigned long size_in_bytes) override
        {
            if (already_downloaded.end() != already_downloaded.find(destination))
                return;
            already_downloaded.insert(destination);

            unsigned long new_size(size + size_in_bytes);
            if (new_size < size)
                overflow = true;
            else
                size = new_size;
        }
    };

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

int
PerformCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PerformCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PERFORM_OPTIONS", "CAVE_PERFORM_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (2 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("perform takes exactly two parameters");

    cmdline.import_options.apply(env);

    std::string action(*cmdline.begin_parameters());

    const auto spec_str(*next(cmdline.begin_parameters()));
    const auto spec(parse_spec_with_nice_error(spec_str, env.get(), { }, filter::All()));
    const auto ids((*env)[selection::AllVersionsUnsorted(generator::Matches(spec, nullptr, { }))]);
    if (ids->empty())
        nothing_matching_error(env.get(), spec_str, filter::All());
    else if (1 != std::distance(ids->begin(), ids->end()))
        throw BeMoreSpecific(spec, ids);
    const std::shared_ptr<const PackageID> id(*ids->begin());

    FetchParts parts;
    parts += fp_regulars;
    if (! cmdline.a_regulars_only.specified())
        parts += fp_extras;
    if (cmdline.a_fetch_unneeded.specified())
        parts += fp_unneeded;

    const std::shared_ptr<PackageIDSequence> replacing(std::make_shared<PackageIDSequence>());
    for (const auto & replacing_spec : cmdline.a_replacing.args())
    {
        PackageDepSpec rspec(parse_spec_with_nice_error(replacing_spec, env.get(), { }, filter::All()));
        const std::shared_ptr<const PackageIDSequence> rids((*env)[selection::AllVersionsUnsorted(generator::Matches(rspec, nullptr, { }))]);
        if (rids->empty())
            nothing_matching_error(env.get(), replacing_spec, filter::All());
        else if (1 != std::distance(rids->begin(), rids->end()))
            throw BeMoreSpecific(rspec, rids);
        else
            replacing->push_back(*rids->begin());
    }

    if (action == "config")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<ConfigAction>()))
            return EXIT_SUCCESS;

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        ConfigActionOptions options(make_named_values<ConfigActionOptions>(
                    n::make_output_manager() = std::ref(output_manager_holder)
                    ));
        ConfigAction config_action(options);
        execute(env, cmdline, id, action, config_action, output_manager_holder);
    }
    else if (action == "fetch")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<FetchAction>()))
            return EXIT_SUCCESS;

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        WantInstallPhase want_phase(cmdline, output_manager_holder);
        std::shared_ptr<Sequence<FetchActionFailure> > failures(std::make_shared<Sequence<FetchActionFailure>>());
        FetchActionOptions options(make_named_values<FetchActionOptions>(
                    n::errors() = failures,
                    n::exclude_unmirrorable() = cmdline.a_exclude_unmirrorable.specified(),
                    n::fetch_parts() = parts,
                    n::ignore_not_in_manifest() = false,
                    n::ignore_unfetched() = cmdline.a_ignore_unfetched.specified(),
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::safe_resume() = true,
                    n::want_phase() = want_phase
                    ));
        FetchAction fetch_action(options);

        try
        {
            execute(env, cmdline, id, action, fetch_action, output_manager_holder);
        }
        catch (const ActionFailedError &)
        {
            if (cmdline.a_ignore_manual_fetch_errors.specified())
            {
                for (const auto & failure : *failures)
                    if (! failure.requires_manual_fetching())
                        throw;
            }
            else
                throw;
        }

    }
    else if (action == "pretend-fetch")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<PretendFetchAction>()))
            return EXIT_SUCCESS;

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        FetchActionOptions options(make_named_values<FetchActionOptions>(
                    n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                    n::exclude_unmirrorable() = cmdline.a_exclude_unmirrorable.specified(),
                    n::fetch_parts() = parts,
                    n::ignore_not_in_manifest() = false,
                    n::ignore_unfetched() = cmdline.a_ignore_unfetched.specified(),
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::safe_resume() = true,
                    n::want_phase() = &want_all_phases
                    ));
        OurPretendFetchAction pretend_fetch_action(options);
        execute(env, cmdline, id, action, pretend_fetch_action, output_manager_holder);

        if (! output_manager_holder.output_manager_if_constructed())
            output_manager_holder(pretend_fetch_action);

        if (pretend_fetch_action.overflow)
            output_manager_holder.output_manager_if_constructed()->stdout_stream()
                << "Total download size: more than " << pretend_fetch_action.size << " bytes" << endl;
        else
            output_manager_holder.output_manager_if_constructed()->stdout_stream()
                << "Total download size: " << pretend_fetch_action.size << " bytes" << endl;
    }
    else if (action == "info")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<InfoAction>()))
            return EXIT_SUCCESS;

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        InfoActionOptions options(make_named_values<InfoActionOptions>(
                    n::make_output_manager() = std::ref(output_manager_holder)
                    ));
        InfoAction info_action(options);
        execute(env, cmdline, id, action, info_action, output_manager_holder);
    }
    else if (action == "install")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<InstallAction>()))
            return EXIT_SUCCESS;

        if (! cmdline.a_destination.specified())
            throw args::DoHelp("--destination must be specified for an install");

        const std::shared_ptr<Repository> destination(env->fetch_repository(
                    RepositoryName(cmdline.a_destination.argument())));

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        WantInstallPhase want_phase(cmdline, output_manager_holder);
        InstallActionOptions options(make_named_values<InstallActionOptions>(
                    n::destination() = destination,
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::perform_uninstall() = std::bind(&perform_uninstall,
                            env, std::cref(cmdline), std::placeholders::_1,
                            action, std::placeholders::_2, std::ref(output_manager_holder)
                            ),
                    n::replacing() = replacing,
                    n::want_phase() = want_phase
                    ));
        InstallAction install_action(options);
        execute(env, cmdline, id, action, install_action, output_manager_holder);
    }
    else if (action == "pretend")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<PretendAction>()))
            return EXIT_SUCCESS;

        if (! cmdline.a_destination.specified())
            throw args::DoHelp("--destination must be specified for a pretend");

        const std::shared_ptr<Repository> destination(env->fetch_repository(
                    RepositoryName(cmdline.a_destination.argument())));

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        PretendActionOptions options(make_named_values<PretendActionOptions>(
                    n::destination() = destination,
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::replacing() = replacing
                    ));
        PretendAction pretend_action(options);
        execute(env, cmdline, id, action, pretend_action, output_manager_holder);
        if (pretend_action.failed())
            return EXIT_FAILURE;
    }
    else if (action == "uninstall")
    {
        if (cmdline.a_if_supported.specified() && ! id->supports_action(SupportsActionTest<UninstallAction>()))
            return EXIT_SUCCESS;

        OutputManagerFromIPCOrEnvironment output_manager_holder(env.get(), cmdline, id);
        WantInstallPhase want_phase(cmdline, output_manager_holder);
        UninstallActionOptions options(make_named_values<UninstallActionOptions>(
                    n::config_protect() = cmdline.a_config_protect.argument(),
                    n::if_for_install_id() = nullptr,
                    n::ignore_for_unmerge() = &ignore_nothing,
                    n::is_overwrite() = false,
                    n::make_output_manager() = std::ref(output_manager_holder),
                    n::override_contents() = nullptr,
                    n::want_phase() = want_phase
                    ));
        UninstallAction uninstall_action(options);
        execute(env, cmdline, id, action, uninstall_action, output_manager_holder);
    }
    else
        throw args::DoHelp("action '" + action + "' unrecognised");

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PerformCommand::make_doc_cmdline()
{
    return std::make_shared<PerformCommandLine>();
}

CommandImportance
PerformCommand::importance() const
{
    return ci_internal;
}

