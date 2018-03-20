/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014 Ciaran McCreesh
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

#include "resolve_common.hh"
#include "cmd_resolve_display_callback.hh"
#include "cmd_resolve_dump.hh"
#include "cmd_display_resolution.hh"
#include "cmd_execute_resolution.hh"
#include "cmd_graph_jobs.hh"
#include "exceptions.hh"
#include "command_command_line.hh"
#include "parse_spec_with_nice_error.hh"

#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/string_list_stream.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/process.hh>

#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/package_or_block_dep_spec.hh>
#include <paludis/resolver/decision_utils.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/resolver/collect_depped_upon.hh>

#include <paludis/resolver/allow_choice_changes_helper.hh>
#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/allowed_to_restart_helper.hh>
#include <paludis/resolver/always_via_binary_helper.hh>
#include <paludis/resolver/can_use_helper.hh>
#include <paludis/resolver/confirm_helper.hh>
#include <paludis/resolver/find_replacing_helper.hh>
#include <paludis/resolver/find_repository_for_helper.hh>
#include <paludis/resolver/get_constraints_for_dependent_helper.hh>
#include <paludis/resolver/get_constraints_for_purge_helper.hh>
#include <paludis/resolver/get_constraints_for_via_binary_helper.hh>
#include <paludis/resolver/get_destination_types_for_blocker_helper.hh>
#include <paludis/resolver/get_destination_types_for_error_helper.hh>
#include <paludis/resolver/get_initial_constraints_for_helper.hh>
#include <paludis/resolver/get_resolvents_for_helper.hh>
#include <paludis/resolver/get_use_existing_nothing_helper.hh>
#include <paludis/resolver/interest_in_spec_helper.hh>
#include <paludis/resolver/make_destination_filtered_generator_helper.hh>
#include <paludis/resolver/make_origin_filtered_generator_helper.hh>
#include <paludis/resolver/make_unmaskable_filter_helper.hh>
#include <paludis/resolver/order_early_helper.hh>
#include <paludis/resolver/remove_hidden_helper.hh>
#include <paludis/resolver/remove_if_dependent_helper.hh>
#include <paludis/resolver/prefer_or_avoid_helper.hh>
#include <paludis/resolver/promote_binaries_helper.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/environment.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/elike_blocker.hh>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <list>
#include <map>
#include <thread>

#include "config.h"

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

namespace
{
    const std::shared_ptr<const Sequence<std::string> > add_resolver_targets(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<Resolver> & resolver,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets,
            bool & is_set)
    {
        Context context("When adding targets from commandline:");

        if (targets->empty())
            throw args::DoHelp("Must specify at least one target");

        const std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
        bool seen_sets(false);
        bool seen_packages(false);
        for (const auto & target : *targets)
        {
            if (target.first.empty())
                continue;

            std::string p_suggesion(target.first);
            try
            {
                auto b(split_elike_blocker(target.first));

                if (ebk_no_block != std::get<0>(b))
                {
                    p_suggesion = std::get<2>(b);
                    seen_packages = true;
                    PackageDepSpec s(parse_spec_with_nice_error(std::get<2>(b), env.get(), { }, filter::All()));
                    BlockDepSpec bs(make_uninstall_blocker(s));
                    result->push_back(stringify(bs));
                    resolver->add_target(bs, target.second);
                }
                else
                {
                    PackageDepSpec s(parse_spec_with_nice_error(std::get<2>(b), env.get(), { updso_throw_if_set }, filter::All()));
                    result->push_back(stringify(s));
                    resolver->add_target(s, target.second);
                    seen_packages = true;
                }
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(target.first), target.second);
                result->push_back(target.first);
                seen_sets = true;
            }
        }

        if (seen_sets + seen_packages > 1)
            throw args::DoHelp("Cannot specify set and non-set targets simultaneously");

        if (seen_sets)
            is_set = true;

        if (resolution_options.a_reinstall_dependents_of.specified())
        {
            auto installed_filter(filter::InstalledAtRoot(env->system_root_key()->parse_value()));
            auto installed_ids((*env)[selection::AllVersionsSorted(
                        generator::All() |
                        installed_filter)]);

            for (const auto & package : resolution_options.a_reinstall_dependents_of.args())
            {
                PackageDepSpec s(parse_spec_with_nice_error(package, env.get(), { }, installed_filter));
                auto ids((*env)[selection::AllVersionsSorted(generator::Matches(s, nullptr, { }) | installed_filter)]);
                if (ids->empty())
                    throw args::DoHelp("Found nothing installed matching '" + package + "' for --" + resolution_options.a_reinstall_dependents_of.long_name());

                for (const auto & id : *ids)
                {
                    auto dependents(collect_dependents(env.get(), id, installed_ids));
                    for (const auto & dependent : *dependents)
                    {
                        BlockDepSpec bs(make_uninstall_blocker(dependent->uniquely_identifying_spec()));
                        resolver->add_target(bs, "reinstalling dependents of " + stringify(s));
                    }
                }
            }
        }

        return result;
    }

    int reinstall_scm_days(const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (resolution_options.a_reinstall_scm.argument() == "always")
            return 0;
        else if (resolution_options.a_reinstall_scm.argument() == "daily")
            return 1;
        else if (resolution_options.a_reinstall_scm.argument() == "weekly")
            return 7;
        else if (resolution_options.a_reinstall_scm.argument() == "never")
            return -1;
        else
            throw args::DoHelp("Don't understand argument '" + resolution_options.a_reinstall_scm.argument() + "' to '--"
                    + resolution_options.a_reinstall_scm.long_name() + "'");
    }

    void serialise_resolved(StringListStream & ser_stream, const Resolved & resolved)
    {
        try
        {
            Serialiser ser(ser_stream);
            resolved.serialise(ser);
            ser_stream.nothing_more_to_write();
        }
        catch (const std::exception & e)
        {
            std::cerr << "Things are about go to horribly wrong. Got an exception whilst serialising: "
                << e.what() << std::endl;
            throw;
        }
    }

    int display_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions &,
            const ResolveCommandLineDisplayOptions & display_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets)
    {
        Context context("When displaying chosen resolution:");

        StringListStream ser_stream;
        std::thread ser_thread(std::bind(&serialise_resolved,
                    std::ref(ser_stream),
                    std::cref(*resolved)));

        int result;

        try
        {
            std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

            for (const auto & group : display_options)
            {
                for (const auto & option : group)
                {
                    if (option->specified())
                    {
                        const std::shared_ptr<const Sequence<std::string> > f(option->forwardable_args());
                        std::copy(f->begin(), f->end(), args->back_inserter());
                    }
                }
            }

            for (const auto & target : *targets)
                args->push_back(target.first);

            if (program_options.a_display_resolution_program.specified())
            {
                std::string command(program_options.a_display_resolution_program.argument());

                if (keys_if_import)
                {
                    for (const auto & kv : *keys_if_import)
                    {
                        args->push_back("--unpackaged-repository-params");
                        args->push_back(kv.first + "=" + kv.second);
                    }
                }

                for (const auto & arg : *args)
                    command = command + " " + args::escape(arg);

                Process process((ProcessCommand(command)));
                process
                    .send_input_to_fd(ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

                result = process.run().wait();
            }
            else
                result = DisplayResolutionCommand().run(env, args, resolved);

            ser_thread.join();
        }
        catch (...)
        {
            ser_thread.join();
            throw;
        }

        return result;
    }

    int graph_jobs(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions &,
            const ResolveCommandLineGraphJobsOptions & graph_jobs_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets)
    {
        Context context("When graphing jobs:");

        if (! graph_jobs_options.a_graph_jobs_basename.specified())
            return 0;

        StringListStream ser_stream;
        std::thread ser_thread(std::bind(&serialise_resolved,
                    std::ref(ser_stream),
                    std::cref(*resolved)));

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        for (const auto & group : graph_jobs_options)
        {
            for (const auto & option : group)
            {
                if (option->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f(option->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
            }
        }

        for (const auto & group : program_options)
        {
            for (const auto & option : group)
            {
                if (option->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f(option->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
            }
        }

        for (const auto & target : *targets)
            args->push_back(target.first);

        int result;
        if (program_options.a_graph_jobs_program.specified())
        {
            std::string command(program_options.a_graph_jobs_program.argument());

            if (keys_if_import)
            {
                for (const auto & kv : *keys_if_import)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(kv.first + "=" + kv.second);
                }
            }

            for (const auto & arg : *args)
                command = command + " " + args::escape(arg);

            Process process((ProcessCommand(command)));
            process
                .send_input_to_fd(ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            result = process.run().wait();
        }
        else
            result = GraphJobsCommand().run(env, args, resolved);

        ser_thread.join();
        return result;
    }

    void serialise_job_lists(StringListStream & ser_stream, const JobLists & job_lists)
    {
        Serialiser ser(ser_stream);
        job_lists.serialise(ser);
        ser_stream.nothing_more_to_write();
    }

    int perform_resolution(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<const Resolved> & resolved,
            const ResolveCommandLineResolutionOptions & resolution_options,
            const ResolveCommandLineExecutionOptions & execution_options,
            const ResolveCommandLineProgramOptions & program_options,
            const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets,
            const std::shared_ptr<const Sequence<std::string> > & world_specs,
            const std::shared_ptr<const Sequence<std::string> > & removed_if_dependent_names,
            const bool is_set,
            const bool pretend_only)
    {
        Context context("When performing chosen resolution:");

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        if (is_set)
            args->push_back("--set");

        for (const auto & group : program_options)
        {
            for (const auto & option : group)
            {
                if (option->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f(option->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
            }
        }

        for (const auto & group : execution_options)
        {
            for (const auto & option : group)
            {
                if (option->specified())
                {
                    const std::shared_ptr<const Sequence<std::string> > f(option->forwardable_args());
                    std::copy(f->begin(), f->end(), args->back_inserter());
                }
            }
        }

        if (pretend_only || ! resolution_options.a_execute.specified())
            args->push_back("--pretend");

        for (const auto & spec : *world_specs)
        {
            args->push_back("--world-specs");
            args->push_back(spec);
        }

        for (const auto & name : *removed_if_dependent_names)
        {
            args->push_back("--removed-if-dependent-names");
            args->push_back(name);
        }

        for (const auto & target : *targets)
            args->push_back(target.first);

        if (program_options.a_execute_resolution_program.specified() || resolution_options.a_execute.specified())
        {
            StringListStream ser_stream;
            serialise_job_lists(ser_stream, *resolved->job_lists());

            std::string command;
            if (program_options.a_execute_resolution_program.specified())
                command = program_options.a_execute_resolution_program.argument();
            else
                command = "$CAVE execute-resolution";

            if (keys_if_import)
            {
                for (const auto & kv : *keys_if_import)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(kv.first + "=" + kv.second);
                }
            }

            for (const auto & arg : *args)
                command = command + " " + args::escape(arg);

            Process process((ProcessCommand(command)));
            process
                .send_input_to_fd(ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD")
                .as_main_process();

            int retcode(process.run().wait());
            _exit(retcode);
        }
        else
            return ExecuteResolutionCommand().run(env, args, resolved->job_lists());
    }

    struct KindNameVisitor
    {
        const std::string visit(const RemoveDecision &) const
        {
            return "remove_decision";
        }

        const std::string visit(const BreakDecision &) const
        {
            return "break_decision";
        }

        const std::string visit(const UnableToMakeDecision &) const
        {
            return "unable_to_make_decision";
        }

        const std::string visit(const NothingNoChangeDecision &) const
        {
            return "nothing_no_change";
        }

        const std::string visit(const ExistingNoChangeDecision &) const
        {
            return "existing_no_change";
        }

        const std::string visit(const ChangesToMakeDecision &) const
        {
            return "changes_to_make";
        }
    };

    std::string stringify_change_by_resolvent(const ChangeByResolvent & r)
    {
        return stringify(*r.package_id());
    }

    struct ShortReasonName
    {
        const std::string visit(const DependencyReason & r) const
        {
            return "from " + stringify(*r.from_id()) + " dependency " + (r.sanitised_dependency().spec().if_package() ?
                    stringify(*r.sanitised_dependency().spec().if_package()) : stringify(*r.sanitised_dependency().spec().if_block()));
        }

        const std::string visit(const WasUsedByReason & r) const
        {
            if (r.ids_and_resolvents_being_removed()->empty())
                return "from was unused";
            else
                return "from was used by " + join(r.ids_and_resolvents_being_removed()->begin(),
                        r.ids_and_resolvents_being_removed()->end(), ", ", stringify_change_by_resolvent);
        }

        const std::string visit(const DependentReason & r) const
        {
            return "from dependent " + stringify(*r.dependent_upon().package_id());
        }

        const std::string visit(const TargetReason & r) const
        {
            return "from target" + (r.extra_information().empty() ? "" : " (" + r.extra_information() + ")");
        }

        const std::string visit(const ViaBinaryReason &) const
        {
            return "from via binary";
        }

        const std::string visit(const PresetReason & r) const
        {
            std::string result("from preset");
            if (! r.maybe_explanation().empty())
                result = result + " (" + r.maybe_explanation() + ")";
            if (r.maybe_reason_for_preset())
                result = result + " (" + r.maybe_reason_for_preset()->accept_returning<std::string>(*this) + ")";
            return result;
        }

        const std::string visit(const SetReason & r) const
        {
            return "from " + stringify(r.set_name()) + " (" + r.reason_for_set()->accept_returning<std::string>(*this) + ")";
        }

        const std::string visit(const LikeOtherDestinationTypeReason & r) const
        {
            return "from being like " + stringify(r.other_resolvent())
                + " (" + r.reason_for_other()->accept_returning<std::string>(*this) + ")";
        }
    };

    void display_restarts_if_requested(const std::list<SuggestRestart> & restarts,
            const ResolveCommandLineResolutionOptions & resolution_options)
    {
        if (! resolution_options.a_dump_restarts.specified())
            return;

        std::cout << "Dumping restarts:" << std::endl << std::endl;

        for (const auto & restart : restarts)
        {
            std::cout << "* " << restart.resolvent() << std::endl;

            std::cout << "    Had decided upon ";
            auto c(get_decided_id_or_null(restart.previous_decision()));
            if (c)
                std::cout << *c;
            else
                std::cout << restart.previous_decision()->accept_returning<std::string>(KindNameVisitor());

            std::cout << std::endl;

            std::cout << "    Which did not satisfy " << restart.problematic_constraint()->spec()
                << ", use existing " << restart.problematic_constraint()->use_existing();
            if (restart.problematic_constraint()->nothing_is_fine_too())
                std::cout << ", nothing is fine too";
            std::cout << " " << restart.problematic_constraint()->reason()->accept_returning<std::string>(ShortReasonName());
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    UseExisting use_existing_from_arg(const args::EnumArg & arg, const bool is_set)
    {
        if (arg.argument() == "auto")
        {
            if (is_set)
                return ue_if_same;
            else
                return ue_never;
        }
        else if (arg.argument() == "never")
            return ue_never;
        else if (arg.argument() == "if-transient")
            return ue_only_if_transient;
        else if (arg.argument() == "if-same-metadata")
            return ue_if_same_metadata;
        else if (arg.argument() == "if-same")
            return ue_if_same;
        else if (arg.argument() == "if-same-version")
            return ue_if_same_version;
        else if (arg.argument() == "if-possible")
            return ue_if_possible;
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");
    }

    DestinationType destination_type_from_arg(
            const Environment * const env,
            const args::EnumArg & arg)
    {
        if (arg.argument() == "auto")
        {
            if (env->preferred_root_key()->parse_value() == FSPath("/"))
                return dt_install_to_slash;
            else
                return dt_install_to_chroot;
        }
        else if (arg.argument() == "binaries")
            return dt_create_binary;
        else if (arg.argument() == "install")
            return dt_install_to_slash;
        else if (arg.argument() == "chroot")
            return dt_install_to_chroot;
        else if (arg.argument() == "cross-compile")
            return dt_cross_compile;
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");
    }

    std::shared_ptr<const QualifiedPackageName> name_if_dependent_remove(
            const std::shared_ptr<const Resolution> & resolution)
    {
        const RemoveDecision * const remove_decision(visitor_cast<const RemoveDecision>(*resolution->decision()));
        if (remove_decision)
            for (const auto & constraint : *resolution->constraints())
                if (visitor_cast<const DependentReason>(*constraint->reason()))
                    return make_shared_copy((*remove_decision->ids()->begin())->name());
        return nullptr;
    }

    std::shared_ptr<Sequence<std::string> > get_removed_if_dependent_names(
            const Environment * const,
            const std::shared_ptr<const Resolved> & resolved)
    {
        auto result(std::make_shared<Sequence<std::string> >());
        for (const auto & decision : *resolved->taken_change_or_remove_decisions())
        {
            auto n(name_if_dependent_remove(*resolved->resolutions_by_resolvent()->find(decision.first->resolvent())));
            if (n)
                result->push_back(stringify(*n));
        }

        return result;
    }

    QualifiedPackageName disambiguate_if_necessary(const Environment * const env,
            const std::string & s)
    {
        if (std::string::npos != s.find('/'))
            return QualifiedPackageName(s);
        else
            return env->fetch_unique_qualified_package_name(PackageNamePart(s));
    }

    struct AllowPretend
    {
        bool visit(const ChangedChoicesConfirmation &) const
        {
            return false;
        }

        bool visit(const BreakConfirmation &) const
        {
            return true;
        }

        bool visit(const DowngradeConfirmation &) const
        {
            return true;
        }

        bool visit(const NotBestConfirmation &) const
        {
            return true;
        }

        bool visit(const RemoveDecision &) const
        {
            return true;
        }

        bool visit(const BreakDecision &) const
        {
            return true;
        }

        bool visit(const MaskedConfirmation &) const
        {
            return true;
        }

        bool visit(const RemoveSystemPackageConfirmation &) const
        {
            return true;
        }

        bool visit(const UninstallConfirmation &) const
        {
            return true;
        }
    };
}

int
paludis::cave::resolve_common(
        const std::shared_ptr<Environment> & env,
        const ResolveCommandLineResolutionOptions & resolution_options,
        const ResolveCommandLineExecutionOptions & execution_options,
        const ResolveCommandLineDisplayOptions & display_options,
        const ResolveCommandLineGraphJobsOptions & graph_jobs_options,
        const ResolveCommandLineProgramOptions & program_options,
        const std::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
        const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets_if_not_purge,
        const std::shared_ptr<const Sequence<std::string> > & world_specs_if_not_auto,
        const bool purge)
{
    int retcode(0);

    AllowChoiceChangesHelper allow_choice_changes_helper(env.get());
    allow_choice_changes_helper.set_allow_choice_changes(! resolution_options.a_no_override_flags.specified());

    AllowedToRemoveHelper allowed_to_remove_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        allowed_to_remove_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());
    for (const auto & spec : resolution_options.a_permit_uninstall.args())
        allowed_to_remove_helper.add_allowed_to_remove_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    AllowedToRestartHelper allowed_to_restart_helper(env.get());
    for (const auto & spec : resolution_options.a_no_restarts_for.args())
        allowed_to_restart_helper.add_no_restarts_for_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    AlwaysViaBinaryHelper always_via_binary_helper(env.get());
    for (const auto & spec : resolution_options.a_via_binary.args())
        always_via_binary_helper.add_always_via_binary_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    CanUseHelper can_use_helper(env.get());
    for (const auto & spec : resolution_options.a_not_usable.args())
        can_use_helper.add_cannot_use_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    ConfirmHelper confirm_helper(env.get());
    for (const auto & spec : resolution_options.a_permit_downgrade.args())
        confirm_helper.add_permit_downgrade_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_permit_old_version.args())
        confirm_helper.add_permit_old_version_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_uninstalls_may_break.args())
        if (spec == "system")
            confirm_helper.set_allowed_to_break_system(true);
        else
            confirm_helper.add_allowed_to_break_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    FindReplacingHelper find_replacing_helper(env.get());
    find_replacing_helper.set_one_binary_per_slot(resolution_options.a_one_binary_per_slot.specified());

    FindRepositoryForHelper find_repository_for_helper(env.get());
    if (resolution_options.a_chroot_path.specified())
        find_repository_for_helper.set_chroot_path(FSPath(resolution_options.a_chroot_path.argument()));
    if (resolution_options.a_cross_host.specified())
        find_repository_for_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());

    GetConstraintsForDependentHelper get_constraints_for_dependent_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        get_constraints_for_dependent_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());
    for (const auto & spec : resolution_options.a_less_restrictive_remove_blockers.args())
        get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    GetConstraintsForPurgeHelper get_constraints_for_purge_helper(env.get());
    for (const auto & spec : resolution_options.a_purge.args())
        get_constraints_for_purge_helper.add_purge_spec(parse_spec_with_nice_error(spec, env.get(), { updso_allow_wildcards }, filter::All()));

    GetConstraintsForViaBinaryHelper get_constraints_for_via_binary_helper(env.get());

    GetDestinationTypesForBlockerHelper get_destination_types_for_blocker_helper(env.get());
    get_destination_types_for_blocker_helper.set_target_destination_type(destination_type_from_arg(env.get(), resolution_options.a_make));

    GetDestinationTypesForErrorHelper get_destination_types_for_error_helper(env.get());
    get_destination_types_for_error_helper.set_target_destination_type(destination_type_from_arg(env.get(), resolution_options.a_make));

    GetInitialConstraintsForHelper get_initial_constraints_for_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        get_initial_constraints_for_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());
    for (const auto & spec : resolution_options.a_without.args())
        get_initial_constraints_for_helper.add_without_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));
    for (const auto & spec : resolution_options.a_preset.args())
        get_initial_constraints_for_helper.add_preset_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()), nullptr);
    get_initial_constraints_for_helper.set_reinstall_scm_days(reinstall_scm_days(resolution_options));

    RemoveHiddenHelper remove_hidden_helper(env.get());
    for (const auto & spec : resolution_options.a_hide.args())
        remove_hidden_helper.add_hide_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    GetResolventsForHelper get_resolvents_for_helper(env.get(), std::cref(remove_hidden_helper));
    get_resolvents_for_helper.set_target_destination_type(destination_type_from_arg(env.get(), resolution_options.a_make));

    if (resolution_options.a_make_dependencies.argument() == "auto")
    {
        if (dt_install_to_slash == destination_type_from_arg(env.get(), resolution_options.a_make))
        {
            get_resolvents_for_helper.set_want_target_dependencies(true);
            get_resolvents_for_helper.set_want_target_runtime_dependencies(true);
        }
        else
        {
            get_resolvents_for_helper.set_want_target_dependencies(false);
            get_resolvents_for_helper.set_want_target_runtime_dependencies(true);
        }
    }
    else if (resolution_options.a_make_dependencies.argument() == "runtime")
    {
        get_resolvents_for_helper.set_want_target_dependencies(false);
        get_resolvents_for_helper.set_want_target_runtime_dependencies(true);
    }
    else if (resolution_options.a_make_dependencies.argument() == "all")
    {
        get_resolvents_for_helper.set_want_target_dependencies(true);
        get_resolvents_for_helper.set_want_target_runtime_dependencies(true);
    }
    else if (resolution_options.a_make_dependencies.argument() == "none")
    {
        get_resolvents_for_helper.set_want_target_dependencies(false);
        get_resolvents_for_helper.set_want_target_runtime_dependencies(false);
    }
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_make_dependencies.argument() + "' to '--"
                + resolution_options.a_make_dependencies.long_name() + "'");

    if (resolution_options.a_slots.argument() == "best-or-installed")
        get_resolvents_for_helper.set_slots(true, false, true);
    else if (resolution_options.a_slots.argument() == "installed-or-best")
        get_resolvents_for_helper.set_slots(false, true, true);
    else if (resolution_options.a_slots.argument() == "all")
        get_resolvents_for_helper.set_slots(true, true, false);
    else if (resolution_options.a_slots.argument() == "best")
        get_resolvents_for_helper.set_slots(true, false, false);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_slots.argument() + "' to '--"
                + resolution_options.a_slots.long_name() + "'");

    if (resolution_options.a_dependencies_to_slash.argument() == "all")
    {
        get_resolvents_for_helper.set_want_dependencies_on_slash(true);
        get_resolvents_for_helper.set_want_runtime_dependencies_on_slash(true);
    }
    else if (resolution_options.a_dependencies_to_slash.argument() == "runtime")
    {
        get_resolvents_for_helper.set_want_dependencies_on_slash(false);
        get_resolvents_for_helper.set_want_runtime_dependencies_on_slash(true);
    }
    else if (resolution_options.a_dependencies_to_slash.argument() == "build")
    {
        get_resolvents_for_helper.set_want_dependencies_on_slash(true);
        get_resolvents_for_helper.set_want_runtime_dependencies_on_slash(false);
    }
    else if (resolution_options.a_dependencies_to_slash.argument() == "none")
    {
        get_resolvents_for_helper.set_want_dependencies_on_slash(false);
        get_resolvents_for_helper.set_want_runtime_dependencies_on_slash(false);
    }
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_dependencies_to_slash.argument() + "' to '--"
                + resolution_options.a_dependencies_to_slash.long_name() + "'");

    if (resolution_options.a_target_slots.argument() == "best-or-installed")
        get_resolvents_for_helper.set_target_slots(true, false, true);
    else if (resolution_options.a_target_slots.argument() == "installed-or-best")
        get_resolvents_for_helper.set_target_slots(false, true, true);
    else if (resolution_options.a_target_slots.argument() == "all")
        get_resolvents_for_helper.set_target_slots(true, true, false);
    else if (resolution_options.a_target_slots.argument() == "best")
        get_resolvents_for_helper.set_target_slots(true, false, false);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_target_slots.argument() + "' to '--"
                + resolution_options.a_target_slots.long_name() + "'");

    GetUseExistingNothingHelper get_use_existing_nothing_helper(env.get());
    for (const auto & spec : resolution_options.a_without.args())
        get_use_existing_nothing_helper.add_without_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_with.args())
        get_use_existing_nothing_helper.add_with_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    get_use_existing_nothing_helper.set_use_existing_for_dependencies(use_existing_from_arg(resolution_options.a_keep, false));
    get_use_existing_nothing_helper.set_use_existing_for_targets(use_existing_from_arg(resolution_options.a_keep_targets, false));
    get_use_existing_nothing_helper.set_use_existing_for_set_targets(use_existing_from_arg(resolution_options.a_keep_targets, true));

    MakeDestinationFilteredGeneratorHelper make_destination_filtered_generator_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        make_destination_filtered_generator_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());

    MakeOriginFilteredGeneratorHelper make_origin_filtered_generator_helper(env.get());
    make_origin_filtered_generator_helper.set_making_binaries(
            dt_create_binary == destination_type_from_arg(env.get(), resolution_options.a_make));

    MakeUnmaskableFilterHelper make_unmaskable_filter_helper(env.get());
    make_unmaskable_filter_helper.set_override_masks(! resolution_options.a_no_override_masks.specified());

    OrderEarlyHelper order_early_helper(env.get());
    for (const auto & spec : resolution_options.a_early.args())
        order_early_helper.add_early_spec(parse_spec_with_nice_error(spec, env.get(), { updso_allow_wildcards }, filter::All()));

    for (const auto & spec : resolution_options.a_late.args())
        order_early_helper.add_late_spec(parse_spec_with_nice_error(spec, env.get(), { updso_allow_wildcards }, filter::All()));

    PreferOrAvoidHelper prefer_or_avoid_helper(env.get());
    for (const auto & spec : resolution_options.a_favour.args())
        prefer_or_avoid_helper.add_prefer_name(disambiguate_if_necessary(env.get(), spec));

    for (const auto & spec : resolution_options.a_avoid.args())
        prefer_or_avoid_helper.add_avoid_name(disambiguate_if_necessary(env.get(), spec));

    for (const auto & spec : resolution_options.a_favour_matching.args())
        prefer_or_avoid_helper.add_prefer_matching((*env)[selection::AllVersionsUnsorted(generator::Matches(parse_spec_with_nice_error(spec, env.get(), {},
                                                                                                                                       filter::All()),
                                                                                                            nullptr, { }))]);

    for (const auto & spec : resolution_options.a_avoid_matching.args())
        prefer_or_avoid_helper.add_avoid_matching((*env)[selection::AllVersionsUnsorted(generator::Matches(parse_spec_with_nice_error(spec, env.get(), {},
                                                                                                                                      filter::All()),
                                                                                                           nullptr, {}))]);

    RemoveIfDependentHelper remove_if_dependent_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        remove_if_dependent_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());
    for (const auto & spec : resolution_options.a_remove_if_dependent.args())
        remove_if_dependent_helper.add_remove_if_dependent_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    InterestInSpecHelper interest_in_spec_helper(env.get());
    if (resolution_options.a_cross_host.specified())
        interest_in_spec_helper.set_cross_compile_host(resolution_options.a_cross_host.argument());
    for (const auto & spec : resolution_options.a_take.args())
    {
        bool might_be_group(std::string::npos ==
                            spec.find_first_not_of("abcdefghijklmnopqrstuvwxyz"
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "0123456789-_"));

        if (might_be_group)
        {
            interest_in_spec_helper.add_take_group(spec);
            try
            {
                interest_in_spec_helper.add_take_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));
            }
            catch (const Exception &)
            {
            }
        }
        else
        {
            interest_in_spec_helper.add_take_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));
        }
    }

    for (const auto & spec : resolution_options.a_take_from.args())
        interest_in_spec_helper.add_take_from_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_ignore.args())
    {
        bool might_be_group(std::string::npos ==
                            spec.find_first_not_of("abcdefghijklmnopqrstuvwxyz"
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "0123456789-_"));

        if (might_be_group)
        {
            interest_in_spec_helper.add_ignore_group(spec);
            try
            {
                interest_in_spec_helper.add_ignore_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));
            }
            catch (const Exception &)
            {
            }
        }
        else
        {
            interest_in_spec_helper.add_ignore_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));
        }
    }

    for (const auto & spec : resolution_options.a_ignore_from.args())
        interest_in_spec_helper.add_ignore_from_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_no_dependencies_from.args())
        interest_in_spec_helper.add_no_dependencies_from_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    for (const auto & spec : resolution_options.a_no_blockers_from.args())
        interest_in_spec_helper.add_no_blockers_from_spec(parse_spec_with_nice_error(spec, env.get(), {updso_allow_wildcards}, filter::All()));

    interest_in_spec_helper.set_follow_installed_dependencies(! resolution_options.a_no_follow_installed_dependencies.specified());
    interest_in_spec_helper.set_follow_installed_build_dependencies(resolution_options.a_follow_installed_build_dependencies.specified());

    if (resolution_options.a_suggestions.argument() == "take")
        interest_in_spec_helper.set_take_suggestions(true);
    else if (resolution_options.a_suggestions.argument() == "display")
        interest_in_spec_helper.set_take_suggestions(indeterminate);
    else if (resolution_options.a_suggestions.argument() == "ignore")
        interest_in_spec_helper.set_take_suggestions(false);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_suggestions.argument() + "' to '--"
                + resolution_options.a_suggestions.long_name() + "'");

    if (resolution_options.a_recommendations.argument() == "take")
        interest_in_spec_helper.set_take_recommendations(true);
    else if (resolution_options.a_recommendations.argument() == "display")
        interest_in_spec_helper.set_take_recommendations(indeterminate);
    else if (resolution_options.a_recommendations.argument() == "ignore")
        interest_in_spec_helper.set_take_recommendations(false);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_recommendations.argument() + "' to '--"
                + resolution_options.a_recommendations.long_name() + "'");

    PromoteBinariesHelper promote_binaries_helper(env.get());
    if (resolution_options.a_promote_binaries.argument() == "never")
        promote_binaries_helper.set_promote_binaries(pb_never);
    else if (resolution_options.a_promote_binaries.argument() == "if-same")
        promote_binaries_helper.set_promote_binaries(pb_if_same);
    else
        throw args::DoHelp("Don't understand argument '" + resolution_options.a_promote_binaries.argument() + "' to '--"
                + resolution_options.a_promote_binaries.long_name() + "'");

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                n::allow_choice_changes_fn() = std::cref(allow_choice_changes_helper),
                n::allowed_to_remove_fn() = std::cref(allowed_to_remove_helper),
                n::allowed_to_restart_fn() = std::cref(allowed_to_restart_helper),
                n::always_via_binary_fn() = std::cref(always_via_binary_helper),
                n::can_use_fn() = std::cref(can_use_helper),
                n::confirm_fn() = std::cref(confirm_helper),
                n::find_replacing_fn() = std::cref(find_replacing_helper),
                n::find_repository_for_fn() = std::cref(find_repository_for_helper),
                n::get_constraints_for_dependent_fn() = std::cref(get_constraints_for_dependent_helper),
                n::get_constraints_for_purge_fn() = std::cref(get_constraints_for_purge_helper),
                n::get_constraints_for_via_binary_fn() = std::cref(get_constraints_for_via_binary_helper),
                n::get_destination_types_for_blocker_fn() = std::cref(get_destination_types_for_blocker_helper),
                n::get_destination_types_for_error_fn() = std::cref(get_destination_types_for_error_helper),
                n::get_initial_constraints_for_fn() = std::cref(get_initial_constraints_for_helper),
                n::get_resolvents_for_fn() = std::cref(get_resolvents_for_helper),
                n::get_use_existing_nothing_fn() = std::cref(get_use_existing_nothing_helper),
                n::interest_in_spec_fn() = std::cref(interest_in_spec_helper),
                n::make_destination_filtered_generator_fn() = std::cref(make_destination_filtered_generator_helper),
                n::make_origin_filtered_generator_fn() = std::cref(make_origin_filtered_generator_helper),
                n::make_unmaskable_filter_fn() = std::cref(make_unmaskable_filter_helper),
                n::order_early_fn() = std::cref(order_early_helper),
                n::prefer_or_avoid_fn() = std::cref(prefer_or_avoid_helper),
                n::promote_binaries_fn() = std::cref(promote_binaries_helper),
                n::remove_hidden_fn() = std::cref(remove_hidden_helper),
                n::remove_if_dependent_fn() = std::cref(remove_if_dependent_helper)
                ));

    std::shared_ptr<Resolver> resolver(std::make_shared<Resolver>(env.get(), resolver_functions));
    bool is_set(false);
    std::shared_ptr<const Sequence<std::string> > targets_cleaned_up;
    std::list<SuggestRestart> restarts;

    try
    {
        {
            DisplayCallback display_callback("Resolving: ");
            ScopedNotifierCallback display_callback_holder(env.get(),
                    NotifierCallbackFunction(std::cref(display_callback)));

            bool first(true);
            while (true)
            {
                try
                {
                    if (purge)
                    {
                        resolver->purge();
                        targets_cleaned_up = std::make_shared<Sequence<std::string>>();
                    } else
                        targets_cleaned_up = add_resolver_targets(env, resolver, resolution_options, targets_if_not_purge, is_set);

                    if (first)
                    {
                        if (targets_cleaned_up)
                            for (const auto & target : *targets_cleaned_up)
                                if ('!' != target.at(0) && std::string::npos != target.find('/'))
                                {
                                    PackageDepSpec ts(parse_spec_with_nice_error(target, env.get(), { }, filter::All()));
                                    if (ts.version_requirements_ptr() && ! ts.version_requirements_ptr()->empty())
                                    {
                                        confirm_helper.add_permit_downgrade_spec(ts);
                                        confirm_helper.add_permit_old_version_spec(ts);
                                    }
                                }

                        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                        first = false;
                    }

                    resolver->resolve();
                    break;
                }
                catch (const SuggestRestart & e)
                {
                    restarts.push_back(e);
                    display_callback(ResolverRestart());
                    get_initial_constraints_for_helper.add_suggested_restart(e);
                    resolver = std::make_shared<Resolver>(env.get(), resolver_functions);

                    if (restarts.size() > 9000)
                        throw InternalError(PALUDIS_HERE, "Restarted over nine thousand times. Something's "
                                "probably gone horribly wrong. Consider using --dump-restarts and having "
                                "a look.");
                }
            }
        }

        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);

        retcode |= display_resolution(env, resolver->resolved(), resolution_options,
                display_options, program_options, keys_if_import,
                purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge);

        retcode |= graph_jobs(env, resolver->resolved(), resolution_options,
                graph_jobs_options, program_options, keys_if_import,
                purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge);

        if (! resolution_options.a_ignore_unable_decisions.specified())
            if (! resolver->resolved()->taken_unable_to_make_decisions()->empty())
                retcode |= 1;

        bool unconfirmed_but_allow_pretend(false);
        if (! resolver->resolved()->taken_unconfirmed_decisions()->empty())
        {
            unconfirmed_but_allow_pretend = true;
            for (auto c(resolver->resolved()->taken_unconfirmed_decisions()->begin()),
                    c_end(resolver->resolved()->taken_unconfirmed_decisions()->end()) ;
                    c != c_end && unconfirmed_but_allow_pretend ; ++c)
                if ((*c)->required_confirmations_if_any())
                    for (auto r((*c)->required_confirmations_if_any()->begin()),
                            r_end((*c)->required_confirmations_if_any()->end()) ;
                            r != r_end && unconfirmed_but_allow_pretend ; ++r)
                        unconfirmed_but_allow_pretend &= (*r)->accept_returning<bool>(AllowPretend());

            retcode |= 2;
        }

        if (! resolution_options.a_ignore_unorderable_jobs.specified())
            if (! resolver->resolved()->taken_unorderable_decisions()->empty())
                retcode |= 4;

        if (0 == retcode || (2 == retcode && unconfirmed_but_allow_pretend))
            return perform_resolution(env, resolver->resolved(), resolution_options,
                    execution_options, program_options, keys_if_import,
                    purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge,
                    world_specs_if_not_auto ? world_specs_if_not_auto : targets_cleaned_up,
                    get_removed_if_dependent_names(env.get(), resolver->resolved()),
                    is_set, unconfirmed_but_allow_pretend);
    }
    catch (...)
    {
        if (! restarts.empty())
            display_restarts_if_requested(restarts, resolution_options);

        dump_if_requested(env, resolver, resolution_options);
        throw;
    }

    return retcode | 1;
}

namespace paludis
{
    template class Sequence<std::pair<std::string, std::string> >;
    template class WrappedForwardIterator<Sequence<std::pair<std::string, std::string> >::ConstIteratorTag,
             const std::pair<std::string, std::string> >;
}

