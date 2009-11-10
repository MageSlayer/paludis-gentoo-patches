/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/util/make_named_values.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/destination.hh>
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

#include <set>
#include <iterator>
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
    struct ExecuteResolutionCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_general_options;
        args::SwitchArg a_pretend;
        args::SwitchArg a_set;

        ResolveCommandLineExecutionOptions execution_options;
        ResolveCommandLineProgramOptions program_options;

        ExecuteResolutionCommandLine() :
            g_general_options(main_options_section(), "General Options", "General options."),
            a_pretend(&g_general_options, "pretend", '\0', "Only carry out the pretend action", false),
            a_set(&g_general_options, "set", '\0', "Our target is a set rather than package specs", false),
            execution_options(this),
            program_options(this)
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

    int do_pretend(
            const std::tr1::shared_ptr<Environment> &,
            const ExecuteResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const int x, const int y)
    {
        Context context("When pretending for '" + stringify(*decision.origin_id()) + "':");

        if (x > 1)
            std::cout << std::string(stringify(x - 1).length() + stringify(y).length() + 4, '\010');
        std::cout << x << " of " << y << std::flush;

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" pretend --hooks --if-supported ");
        command.append(stringify(decision.origin_id()->uniquely_identifying_spec()));
        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

        paludis::Command cmd(command);
        return run_command(cmd);
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

    int do_fetch(
            const std::tr1::shared_ptr<Environment> &,
            const ExecuteResolutionCommandLine & cmdline,
            const ChangesToMakeDecision & decision,
            const int x, const int y)
    {
        const std::tr1::shared_ptr<const PackageID> id(decision.origin_id());
        Context context("When fetching for '" + stringify(*id) + "':");

        starting_action("fetch", decision, x, y);

        std::string command(cmdline.program_options.a_perform_program.argument());
        if (command.empty())
            command = "$CAVE perform";

        command.append(" fetch --hooks --if-supported ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --x-of-y '" + stringify(x) + " of " + stringify(y) + "'");

        paludis::Command cmd(command);
        int retcode(run_command(cmd));

        done_action("fetch", decision, 0 == retcode);
        return retcode;
    }

    int do_install(
            const std::tr1::shared_ptr<Environment> &,
            const ExecuteResolutionCommandLine & cmdline,
            const std::tr1::shared_ptr<const Resolution> & resolution,
            const ChangesToMakeDecision & decision,
            const std::tr1::shared_ptr<const Destination> & destination,
            const int x, const int y)
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

        command.append(" install --hooks ");
        command.append(stringify(id->uniquely_identifying_spec()));
        command.append(" --destination " + stringify(destination->repository()));
        for (PackageIDSequence::ConstIterator i(destination->replacing()->begin()),
                i_end(destination->replacing()->end()) ;
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

        paludis::Command cmd(command);
        int retcode(run_command(cmd));

        done_action(action_string, decision, 0 == retcode);
        return retcode;
    }

    int execute_resolution(
            const std::tr1::shared_ptr<Environment> & env,
            const ResolverLists & lists,
            const ExecuteResolutionCommandLine & cmdline)
    {
        Context context("When executing chosen resolution:");

        int retcode(0), x(0), y(std::distance(lists.ordered()->begin(), lists.ordered()->end()));

        if (0 != env->perform_hook(Hook("install_task_execute_pre")
                    ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
            throw ActionAbortedError("Aborted by hook");

        try
        {
            if (0 != env->perform_hook(Hook("pretend_all_pre")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                        ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

            std::cout << "Executing pretend actions: " << std::flush;

            for (Resolutions::ConstIterator c(lists.ordered()->begin()), c_end(lists.ordered()->end()) ;
                    c != c_end ; ++c)
            {
                const ChangesToMakeDecision * const decision(simple_visitor_cast<const ChangesToMakeDecision>(
                            *(*c)->decision()));
                if (! decision)
                    throw InternalError(PALUDIS_HERE, "huh? not ChangesToMakeDecision");
                retcode |= do_pretend(env, cmdline, *decision, ++x, y);
            }

            std::cout << std::endl;

            if (0 != env->perform_hook(Hook("pretend_all_post")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                        ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

            if (0 != retcode || cmdline.a_pretend.specified())
                return retcode;

            x = 0;

            if (0 != env->perform_hook(Hook("install_all_pre")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

            for (Resolutions::ConstIterator c(lists.ordered()->begin()), c_end(lists.ordered()->end()) ;
                    c != c_end ; ++c)
            {
                ++x;

                const ChangesToMakeDecision * const decision(simple_visitor_cast<const ChangesToMakeDecision>(
                            *(*c)->decision()));
                if (! decision)
                    throw InternalError(PALUDIS_HERE, "huh? not ChangesToMakeDecision");

                retcode = do_fetch(env, cmdline, *decision, x, y);
                if (0 != retcode)
                    return retcode;

                retcode = do_install(env, cmdline, *c, *decision, decision->destination(), x, y);
                if (0 != retcode)
                    return retcode;
            }

            if (0 != env->perform_hook(Hook("install_all_post")
                        ("TARGETS", join(cmdline.begin_parameters(), cmdline.end_parameters(), " "))
                    ).max_exit_status())
                throw ActionAbortedError("Aborted by hook");

            if (! cmdline.execution_options.a_preserve_world.specified())
            {
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
                                || *a == "everything" || *a == "insecurity")
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
            }
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


