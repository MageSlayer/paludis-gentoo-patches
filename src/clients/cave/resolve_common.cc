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

#include "resolve_common.hh"
#include "cmd_resolve_display_callback.hh"
#include "cmd_resolve_dump.hh"
#include "cmd_display_resolution.hh"
#include "cmd_execute_resolution.hh"
#include "cmd_graph_jobs.hh"
#include "exceptions.hh"
#include "command_command_line.hh"

#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/system.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/string_list_stream.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
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

#include <paludis/resolver/allow_choice_changes_helper.hh>
#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/always_via_binary_helper.hh>
#include <paludis/resolver/can_use_helper.hh>
#include <paludis/resolver/confirm_helper.hh>
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
#include <paludis/resolver/remove_if_dependent_helper.hh>
#include <paludis/resolver/prefer_or_avoid_helper.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/environment.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/selection_cache.hh>
#include <paludis/package_id.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <list>
#include <map>

#include "config.h"

using namespace paludis;
using namespace paludis::resolver;
using namespace cave;

using std::cout;
using std::endl;

namespace
{
    const std::shared_ptr<const Sequence<std::string> > add_resolver_targets(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<Resolver> & resolver,
            const ResolveCommandLineResolutionOptions &,
            const std::shared_ptr<const Sequence<std::pair<std::string, std::string> > > & targets,
            bool & is_set)
    {
        Context context("When adding targets from commandline:");

        if (targets->empty())
            throw args::DoHelp("Must specify at least one target");

        const std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
        bool seen_sets(false), seen_packages(false);
        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
        {
            if (p->first.empty())
                continue;

            try
            {
                if ('!' == p->first.at(0))
                {
                    seen_packages = true;
                    PackageDepSpec s(parse_user_package_dep_spec(p->first.substr(1), env.get(), { }));
                    BlockDepSpec bs("!" + stringify(s), s, bk_weak);
                    result->push_back(stringify(bs));
                    resolver->add_target(bs, p->second);
                }
                else
                {
                    PackageDepSpec s(parse_user_package_dep_spec(p->first, env.get(),
                                { updso_throw_if_set }));
                    result->push_back(stringify(s));
                    resolver->add_target(s, p->second);
                    seen_packages = true;
                }
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                if (seen_sets)
                    throw args::DoHelp("Cannot specify multiple set targets");

                resolver->add_target(SetName(p->first), p->second);
                result->push_back(p->first);
                seen_sets = true;
            }
        }

        if (seen_sets + seen_packages > 1)
            throw args::DoHelp("Cannot specify set and non-set targets simultaneously");

        if (seen_sets)
            is_set = true;

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
        Serialiser ser(ser_stream);
        resolved.serialise(ser);
        ser_stream.nothing_more_to_write();
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
        Thread ser_thread(std::bind(&serialise_resolved,
                    std::ref(ser_stream),
                    std::cref(*resolved)));

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        for (args::ArgsSection::GroupsConstIterator g(display_options.begin()), g_end(display_options.end()) ;
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

        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(p->first);

        if (program_options.a_display_resolution_program.specified())
        {
            std::string command(program_options.a_display_resolution_program.argument());

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            Process process((ProcessCommand(command)));
            process
                .send_input_to_fd(ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            return process.run().wait();
        }
        else
            return DisplayResolutionCommand().run(env, args, resolved);
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
        Thread ser_thread(std::bind(&serialise_resolved,
                    std::ref(ser_stream),
                    std::cref(*resolved)));

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        for (args::ArgsSection::GroupsConstIterator g(graph_jobs_options.begin()), g_end(graph_jobs_options.end()) ;
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

        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(p->first);

        if (program_options.a_graph_jobs_program.specified())
        {
            std::string command(program_options.a_graph_jobs_program.argument());

            if (keys_if_import)
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

            Process process((ProcessCommand(command)));
            process
                .send_input_to_fd(ser_stream, -1, "PALUDIS_SERIALISED_RESOLUTION_FD");

            return process.run().wait();
        }
        else
            return GraphJobsCommand().run(env, args, resolved);
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
            const bool is_set)
    {
        Context context("When performing chosen resolution:");

        std::shared_ptr<Sequence<std::string> > args(std::make_shared<Sequence<std::string>>());

        if (is_set)
            args->push_back("--set");

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

        if (! resolution_options.a_execute.specified())
            args->push_back("--pretend");

        for (Sequence<std::string>::ConstIterator p(world_specs->begin()), p_end(world_specs->end()) ;
                p != p_end ; ++p)
        {
            args->push_back("--world-specs");
            args->push_back(*p);
        }

        for (auto p(removed_if_dependent_names->begin()), p_end(removed_if_dependent_names->end()) ;
                p != p_end ; ++p)
        {
            args->push_back("--removed-if-dependent-names");
            args->push_back(*p);
        }

        for (Sequence<std::pair<std::string, std::string> >::ConstIterator p(targets->begin()), p_end(targets->end()) ;
                p != p_end ; ++p)
            args->push_back(p->first);

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
                for (Map<std::string, std::string>::ConstIterator k(keys_if_import->begin()),
                        k_end(keys_if_import->end()) ;
                        k != k_end ; ++k)
                {
                    args->push_back("--unpackaged-repository-params");
                    args->push_back(k->first + "=" + k->second);
                }

            for (Sequence<std::string>::ConstIterator a(args->begin()), a_end(args->end()) ;
                    a != a_end ; ++a)
                command = command + " " + args::escape(*a);

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
            return "from dependent " + stringify(*r.id_and_resolvent_being_removed().package_id());
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

        for (std::list<SuggestRestart>::const_iterator r(restarts.begin()), r_end(restarts.end()) ;
                r != r_end ; ++r)
        {
            std::cout << "* " << r->resolvent() << std::endl;

            std::cout << "    Had decided upon ";
            auto c(get_decided_id_or_null(r->previous_decision()));
            if (c)
                std::cout << *c;
            else
                std::cout << r->previous_decision()->accept_returning<std::string>(KindNameVisitor());

            std::cout << std::endl;

            std::cout << "    Which did not satisfy " << r->problematic_constraint()->spec()
                << ", use existing " << r->problematic_constraint()->use_existing();
            if (r->problematic_constraint()->nothing_is_fine_too())
                std::cout << ", nothing is fine too";
            std::cout << " " << r->problematic_constraint()->reason()->accept_returning<std::string>(ShortReasonName());
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
            if (env->preferred_root_key()->value() == FSPath("/"))
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
        else
            throw args::DoHelp("Don't understand argument '" + arg.argument() + "' to '--"
                    + arg.long_name() + "'");
    }

    std::shared_ptr<const QualifiedPackageName> name_if_dependent_remove(
            const std::shared_ptr<const Resolution> & resolution)
    {
        const RemoveDecision * const remove_decision(simple_visitor_cast<const RemoveDecision>(*resolution->decision()));
        if (remove_decision)
            for (auto r(resolution->constraints()->begin()), r_end(resolution->constraints()->end()) ;
                    r != r_end ; ++r)
                if (simple_visitor_cast<const DependentReason>(*(*r)->reason()))
                    return make_shared_copy((*remove_decision->ids()->begin())->name());
        return make_null_shared_ptr();
    }

    std::shared_ptr<Sequence<std::string> > get_removed_if_dependent_names(
            const Environment * const,
            const std::shared_ptr<const Resolved> & resolved)
    {
        auto result(std::make_shared<Sequence<std::string> >());
        for (auto d(resolved->taken_change_or_remove_decisions()->begin()),
                d_end(resolved->taken_change_or_remove_decisions()->end()) ;
                d != d_end ; ++d)
        {
            auto n(name_if_dependent_remove(*resolved->resolutions_by_resolvent()->find(d->first->resolvent())));
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
            return env->package_database()->fetch_unique_qualified_package_name(PackageNamePart(s));
    }
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

    AllowChoiceChangesHelper allow_choice_changes_helper(env.get());;
    allow_choice_changes_helper.set_allow_choice_changes(! resolution_options.a_no_override_flags.specified());

    AllowedToRemoveHelper allowed_to_remove_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_uninstall.begin_args()),
            i_end(resolution_options.a_permit_uninstall.end_args()) ;
            i != i_end ; ++i)
        allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    AlwaysViaBinaryHelper always_via_binary_helper(env.get());
#ifdef ENABLE_PBINS
    for (args::StringSetArg::ConstIterator i(resolution_options.a_via_binary.begin_args()),
            i_end(resolution_options.a_via_binary.end_args()) ;
            i != i_end ; ++i)
        always_via_binary_helper.add_always_via_binary_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
#endif

    CanUseHelper can_use_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_not_usable.begin_args()),
            i_end(resolution_options.a_not_usable.end_args()) ;
            i != i_end ; ++i)
        can_use_helper.add_cannot_use_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    ConfirmHelper confirm_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_downgrade.begin_args()),
            i_end(resolution_options.a_permit_downgrade.end_args()) ;
            i != i_end ; ++i)
        confirm_helper.add_permit_downgrade_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
    for (args::StringSetArg::ConstIterator i(resolution_options.a_permit_old_version.begin_args()),
            i_end(resolution_options.a_permit_old_version.end_args()) ;
            i != i_end ; ++i)
        confirm_helper.add_permit_old_version_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));
    for (args::StringSetArg::ConstIterator i(resolution_options.a_uninstalls_may_break.begin_args()),
            i_end(resolution_options.a_uninstalls_may_break.end_args()) ;
            i != i_end ; ++i)
        if (*i == "system")
            confirm_helper.set_allowed_to_break_system(true);
        else
            confirm_helper.add_allowed_to_break_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    FindRepositoryForHelper find_repository_for_helper(env.get());

    GetConstraintsForDependentHelper get_constraints_for_dependent_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_less_restrictive_remove_blockers.begin_args()),
            i_end(resolution_options.a_less_restrictive_remove_blockers.end_args()) ;
            i != i_end ; ++i)
        get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(
                parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    GetConstraintsForPurgeHelper get_constraints_for_purge_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_purge.begin_args()),
            i_end(resolution_options.a_purge.end_args()) ;
            i != i_end ; ++i)
        get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    GetConstraintsForViaBinaryHelper get_constraints_for_via_binary_helper(env.get());

    GetDestinationTypesForBlockerHelper get_destination_types_for_blocker_helper(env.get());
    get_destination_types_for_blocker_helper.set_target_destination_type(destination_type_from_arg(env.get(), resolution_options.a_make));

    GetDestinationTypesForErrorHelper get_destination_types_for_error_helper(env.get());
    get_destination_types_for_error_helper.set_target_destination_type(destination_type_from_arg(env.get(), resolution_options.a_make));

    GetInitialConstraintsForHelper get_initial_constraints_for_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_without.begin_args()),
            i_end(resolution_options.a_without.end_args()) ;
            i != i_end ; ++i)
        get_initial_constraints_for_helper.add_without_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_preset.begin_args()),
            i_end(resolution_options.a_preset.end_args()) ;
            i != i_end ; ++i)
        get_initial_constraints_for_helper.add_preset_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    get_initial_constraints_for_helper.set_reinstall_scm_days(reinstall_scm_days(resolution_options));

    GetResolventsForHelper get_resolvents_for_helper(env.get());
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
    for (args::StringSetArg::ConstIterator i(resolution_options.a_without.begin_args()),
            i_end(resolution_options.a_without.end_args()) ;
            i != i_end ; ++i)
        get_use_existing_nothing_helper.add_without_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_with.begin_args()),
            i_end(resolution_options.a_with.end_args()) ;
            i != i_end ; ++i)
        get_use_existing_nothing_helper.add_with_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    get_use_existing_nothing_helper.set_use_existing_for_dependencies(use_existing_from_arg(resolution_options.a_keep, false));
    get_use_existing_nothing_helper.set_use_existing_for_targets(use_existing_from_arg(resolution_options.a_keep_targets, false));
    get_use_existing_nothing_helper.set_use_existing_for_set_targets(use_existing_from_arg(resolution_options.a_keep_targets, true));

    MakeDestinationFilteredGeneratorHelper make_destination_filtered_generator_helper(env.get());

    MakeOriginFilteredGeneratorHelper make_origin_filtered_generator_helper(env.get());
    make_origin_filtered_generator_helper.set_making_binaries(
            dt_create_binary == destination_type_from_arg(env.get(), resolution_options.a_make));

    MakeUnmaskableFilterHelper make_unmaskable_filter_helper(env.get());
    make_unmaskable_filter_helper.set_override_masks(! resolution_options.a_no_override_masks.specified());

    OrderEarlyHelper order_early_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_early.begin_args()),
            i_end(resolution_options.a_early.end_args()) ;
            i != i_end ; ++i)
        order_early_helper.add_early_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_late.begin_args()),
            i_end(resolution_options.a_late.end_args()) ;
            i != i_end ; ++i)
        order_early_helper.add_late_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    PreferOrAvoidHelper prefer_or_avoid_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_favour.begin_args()),
            i_end(resolution_options.a_favour.end_args()) ;
            i != i_end ; ++i)
        prefer_or_avoid_helper.add_prefer_name(disambiguate_if_necessary(env.get(), *i));
    for (args::StringSetArg::ConstIterator i(resolution_options.a_avoid.begin_args()),
            i_end(resolution_options.a_avoid.end_args()) ;
            i != i_end ; ++i)
        prefer_or_avoid_helper.add_avoid_name(disambiguate_if_necessary(env.get(), *i));

    RemoveIfDependentHelper remove_if_dependent_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_remove_if_dependent.begin_args()),
            i_end(resolution_options.a_remove_if_dependent.end_args()) ;
            i != i_end ; ++i)
        remove_if_dependent_helper.add_remove_if_dependent_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    InterestInSpecHelper interest_in_spec_helper(env.get());
    for (args::StringSetArg::ConstIterator i(resolution_options.a_take.begin_args()),
            i_end(resolution_options.a_take.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_take_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_take_from.begin_args()),
            i_end(resolution_options.a_take_from.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_take_from_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore.begin_args()),
            i_end(resolution_options.a_ignore.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_ignore_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_ignore_from.begin_args()),
            i_end(resolution_options.a_ignore_from.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_ignore_from_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_dependencies_from.begin_args()),
            i_end(resolution_options.a_no_dependencies_from.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_no_dependencies_from_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

    for (args::StringSetArg::ConstIterator i(resolution_options.a_no_blockers_from.begin_args()),
            i_end(resolution_options.a_no_blockers_from.end_args()) ;
            i != i_end ; ++i)
        interest_in_spec_helper.add_no_blockers_from_spec(parse_user_package_dep_spec(*i, env.get(), { updso_allow_wildcards }));

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

    ResolverFunctions resolver_functions(make_named_values<ResolverFunctions>(
                n::allow_choice_changes_fn() = std::cref(allow_choice_changes_helper),
                n::allowed_to_remove_fn() = std::cref(allowed_to_remove_helper),
                n::always_via_binary_fn() = std::cref(always_via_binary_helper),
                n::can_use_fn() = std::cref(can_use_helper),
                n::confirm_fn() = std::cref(confirm_helper),
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
                n::remove_if_dependent_fn() = std::cref(remove_if_dependent_helper)
                ));

    ScopedSelectionCache selection_cache(env.get());
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

        if (! resolver->resolved()->taken_unconfirmed_decisions()->empty())
            retcode |= 2;

        if (! resolution_options.a_ignore_unorderable_jobs.specified())
            if (! resolver->resolved()->taken_unorderable_decisions()->empty())
                retcode |= 4;

        if (0 == retcode)
            return perform_resolution(env, resolver->resolved(), resolution_options,
                    execution_options, program_options, keys_if_import,
                    purge ? std::make_shared<const Sequence<std::pair<std::string, std::string> > >() : targets_if_not_purge,
                    world_specs_if_not_auto ? world_specs_if_not_auto : targets_cleaned_up,
                    get_removed_if_dependent_names(env.get(), resolver->resolved()),
                    is_set);
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

template class Sequence<std::pair<std::string, std::string> >;
template class WrappedForwardIterator<Sequence<std::pair<std::string, std::string> >::ConstIteratorTag,
         const std::pair<std::string, std::string> >;

