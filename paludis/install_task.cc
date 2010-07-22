/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/install_task.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/hook.hh>
#include <paludis/repository.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/version_requirements.hh>
#include <paludis/tasks_exceptions.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/handled_information.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/output_manager_from_environment.hh>
#include <paludis/output_manager.hh>
#include <paludis/standard_output_manager.hh>
#include <functional>
#include <sstream>
#include <algorithm>
#include <list>
#include <vector>
#include <set>

using namespace paludis;

#include <paludis/install_task-se.cc>

namespace
{
    WantPhase want_all_phases_function(
            InstallTask * const task,
            OutputManagerFromEnvironment & output_manager_holder,
            bool & done_any, const std::string & phase)
    {
        output_manager_holder.construct_standard_if_unconstructed();
        task->on_phase_proceed_unconditionally(output_manager_holder.output_manager_if_constructed(), phase);
        done_any = true;
        return wp_yes;
    }

    WantPhase want_phase_function(
            InstallTask * const task,
            OutputManagerFromEnvironment & output_manager_holder,
            const std::shared_ptr<const Set<std::string> > & abort_at_phases,
            const std::shared_ptr<const Set<std::string> > & skip_phases,
            const std::shared_ptr<const Set<std::string> > & skip_until_phases,
            bool & done_any, const std::string & phase)
    {
        output_manager_holder.construct_standard_if_unconstructed();
        if (abort_at_phases->end() != abort_at_phases->find(phase))
        {
            task->on_phase_abort(output_manager_holder.output_manager_if_constructed(), phase);
            return wp_abort;
        }

        if (! skip_until_phases->empty())
            if (! done_any)
                if (skip_until_phases->end() == skip_until_phases->find(phase))
                {
                    task->on_phase_skip_until(output_manager_holder.output_manager_if_constructed(), phase);
                    return wp_skip;
                }

        /* make --skip-until-phase foo --skip-phase foo work */
        done_any = true;

        if (skip_phases->end() != skip_phases->find(phase))
        {
            task->on_phase_skip(output_manager_holder.output_manager_if_constructed(), phase);
            return wp_skip;
        }

        task->on_phase_proceed_conditionally(output_manager_holder.output_manager_if_constructed(), phase);
        return wp_yes;
    }
}

namespace paludis
{
    template<>
    struct Implementation<InstallTask>
    {
        Environment * const env;
        DepList dep_list;

        std::list<std::string> raw_targets;
        std::shared_ptr<SetSpecTree> targets;
        std::shared_ptr<std::string> add_to_world_spec;
        std::shared_ptr<const DestinationsSet> destinations;

        bool pretend;
        bool fetch_only;
        bool preserve_world;
        bool safe_resume;

        bool had_set_targets;
        bool had_package_targets;
        bool override_target_type;

        InstallTaskContinueOnFailure continue_on_failure;

        std::shared_ptr<const Set<std::string> > abort_at_phases;
        std::shared_ptr<const Set<std::string> > skip_phases;
        std::shared_ptr<const Set<std::string> > skip_until_phases;

        bool phase_options_apply_to_first;
        bool phase_options_apply_to_last;
        bool phase_options_apply_to_all;

        bool had_resolution_failures;

        Implementation<InstallTask>(Environment * const e, const DepListOptions & o,
                std::shared_ptr<const DestinationsSet> d) :
            env(e),
            dep_list(e, o),
            targets(new SetSpecTree(std::make_shared<AllDepSpec>())),
            destinations(d),
            pretend(false),
            fetch_only(false),
            preserve_world(false),
            safe_resume(false),
            had_set_targets(false),
            had_package_targets(false),
            override_target_type(false),
            continue_on_failure(itcof_if_fetch_only),
            abort_at_phases(new Set<std::string>),
            skip_phases(new Set<std::string>),
            skip_until_phases(new Set<std::string>),
            phase_options_apply_to_first(false),
            phase_options_apply_to_last(false),
            phase_options_apply_to_all(false),
            had_resolution_failures(false)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<InstallTask::TargetsConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };
}

InstallTask::InstallTask(Environment * const env, const DepListOptions & options,
        const std::shared_ptr<const DestinationsSet> & d) :
    PrivateImplementationPattern<InstallTask>(new Implementation<InstallTask>(env, options, d))
{
}

InstallTask::~InstallTask()
{
}

void
InstallTask::clear()
{
    _imp->targets.reset(new SetSpecTree(std::make_shared<AllDepSpec>()));
    _imp->had_set_targets = false;
    _imp->had_package_targets = false;
    _imp->dep_list.clear();
    _imp->raw_targets.clear();
    _imp->had_package_targets = false;
}

void
InstallTask::set_targets_from_user_specs(const std::shared_ptr<const Sequence<std::string> > & s)
{
    using namespace std::placeholders;
    std::for_each(s->begin(), s->end(), std::bind(&InstallTask::_add_target, this, _1));
}

void
InstallTask::set_targets_from_exact_packages(const std::shared_ptr<const PackageIDSequence> & s)
{
    using namespace std::placeholders;
    std::for_each(s->begin(), s->end(), std::bind(&InstallTask::_add_package_id, this, _1));
}

namespace
{
    std::shared_ptr<DepListEntryHandled> handled_from_string(const std::string & s,
            const Environment * const env)
    {
        Context context("When decoding DepListEntryHandled value '" + s + "':");

        if (s.empty())
            throw InternalError(PALUDIS_HERE, "Empty DepListEntryHandled value");

        switch (s.at(0))
        {
            case 'S':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "S takes no extra value");
                return std::make_shared<DepListEntryHandledSuccess>();

            case 'E':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "E takes no extra value");
                return std::make_shared<DepListEntryHandledFetchFailed>();

            case 'T':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "T takes no extra value");
                return std::make_shared<DepListEntryHandledFetchSuccess>();

            case 'U':
                return std::make_shared<DepListEntryHandledSkippedUnsatisfied>(
                            parse_user_package_dep_spec(s.substr(1), env, UserPackageDepSpecOptions()));

            case 'D':
                return std::make_shared<DepListEntryHandledSkippedDependent>(
                            *(*env)[selection::RequireExactlyOne(generator::Matches(
                                    parse_user_package_dep_spec(s.substr(1), env,
                                        UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            case 'F':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "F takes no extra value");
                return std::make_shared<DepListEntryHandledFailed>();

            case 'P':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "P takes no extra value");
                return std::make_shared<DepListEntryUnhandled>();

            case 'N':
                if (s.length() != 1)
                    throw InternalError(PALUDIS_HERE, "N takes no extra value");
                return std::make_shared<DepListEntryNoHandlingRequired>();

            default:
                throw InternalError(PALUDIS_HERE, "Unknown value '" + s + "'");
        }
    }
}

void
InstallTask::set_targets_from_serialisation(const std::string & format,
        const std::shared_ptr<const Sequence<std::string> > & ss)
{
    if (format != "0.25" && format != "0.37")
        throw InternalError(PALUDIS_HERE, "Serialisation format '" + format + "' not supported by this version of Paludis");

    for (Sequence<std::string>::ConstIterator s(ss->begin()), s_end(ss->end()) ;
            s != s_end ; ++s)
    {
        Context context("When adding serialised entry '" + *s + "':");

        std::list<std::string> tokens;
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(*s, ";", "", std::back_inserter(tokens));

        if (tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too short: no kind");
        const DepListEntryKind kind(destringify<DepListEntryKind>(*tokens.begin()));
        tokens.pop_front();

        if (tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too short: no package_id");
        const std::shared_ptr<const PackageID> package_id(*(*_imp->env)[selection::RequireExactlyOne(
                    generator::Matches(parse_user_package_dep_spec(*tokens.begin(),
                            _imp->env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
        tokens.pop_front();

        if (tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too short: no destination");
        std::shared_ptr<Repository> destination;
        if ("0" != *tokens.begin())
            destination = _imp->env->package_database()->fetch_repository(RepositoryName(*tokens.begin()));
        tokens.pop_front();

        if (tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too short: no state");
        const DepListEntryState state(destringify<DepListEntryState>(*tokens.begin()));
        tokens.pop_front();

        if (tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too short: no handled");
        std::shared_ptr<DepListEntryHandled> handled(handled_from_string(*tokens.begin(), _imp->env));
        tokens.pop_front();

        if (! tokens.empty())
            throw InternalError(PALUDIS_HERE, "Serialised value '" + *s + "' too long");

        _imp->dep_list.push_back(make_named_values<DepListEntry>(
                n::associated_entry() = static_cast<DepListEntry *>(0),
                n::destination() = destination,
                n::generation() = 0,
                n::handled() = handled,
                n::kind() = kind,
                n::package_id() = package_id,
                n::state() = state,
                n::tags() = std::make_shared<DepListEntryTags>()
                ));
    }
}

std::string
InstallTask::serialised_format() const
{
    return "0.37";
}

namespace
{
    struct HandledDisplayer
    {
        std::string result;
        const bool undo_failures;

        HandledDisplayer(const bool b) :
            undo_failures(b)
        {
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
            result = "N";
        }

        void visit(const DepListEntryHandledSuccess &)
        {
            result = "S";
        }

        void visit(const DepListEntryHandledFetchFailed &)
        {
            if (undo_failures)
                result = "P";
            else
                result = "E";
        }

        void visit(const DepListEntryHandledFetchSuccess &)
        {
            result = "T";
        }

        void visit(const DepListEntryHandledFailed &)
        {
            if (undo_failures)
                result = "P";
            else
                result = "F";
        }

        void visit(const DepListEntryUnhandled &)
        {
            result = "P";
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied & s)
        {
            if (undo_failures)
                result = "P";
            else
                result = "U" + stringify(s.spec());
        }

        void visit(const DepListEntryHandledSkippedDependent & s)
        {
            if (undo_failures)
                result = "P";
            else
                result = "D=" + stringify(*s.id());
        }
    };
}

std::string
InstallTask::serialise(const bool undo_failures) const
{
    std::ostringstream result;

    for (DepList::ConstIterator d(_imp->dep_list.begin()), d_end(_imp->dep_list.end()) ;
            d != d_end ; ++d)
    {
        switch (d->kind())
        {
            case dlk_already_installed:
            case dlk_virtual:
            case dlk_provided:
            case dlk_block:
            case dlk_masked:
            case dlk_suggested:
                continue;

            case dlk_package:
            case dlk_subpackage:
                break;

            case last_dlk:
                throw InternalError(PALUDIS_HERE, "Bad d->kind");
        }

        if (! result.str().empty())
            result << " ";

        result << "'";

        result << d->kind() << ";";

        result << "=" << *d->package_id() << ";";

        if (d->destination())
            result << d->destination()->name() << ";";
        else
            result << "0" << ";";

        result << d->state() << ";";

        HandledDisplayer h(undo_failures);
        d->handled()->accept(h);
        result << h.result;

        result << "'";
    }

    return result.str();
}

void
InstallTask::_add_target(const std::string & target)
{
    Context context("When adding install target '" + target + "':");

    std::shared_ptr<SetSpecTree> s;

    try
    {
        std::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(parse_user_package_dep_spec(target,
                        _imp->env, UserPackageDepSpecOptions() + updso_throw_if_set + updso_allow_wildcards,
                        filter::SupportsAction<InstallAction>())));

        bool spec_is_installed(false);

        if (spec->in_repository_ptr() &&
                _imp->env->package_database()->has_repository_named(*spec->in_repository_ptr()) &&
                _imp->env->package_database()->fetch_repository(*spec->in_repository_ptr())->installed_root_key())
            spec_is_installed = true;
        else if (spec->from_repository_ptr())
            spec_is_installed = true;
        else if (spec->installed_at_path_ptr())
            spec_is_installed = true;

        if (spec_is_installed)
            Log::get_instance()->message("install_task.installed_spec", ll_warning, lc_context)
                << "Target '" << target << "' appears to make use of ::repository restrictions "
                << "that restrict to an installed repository. This probably does not do what you "
                << "think it does. Consult the FAQ for an explanation";

        if (_imp->had_set_targets)
            throw HadBothPackageAndSetTargets();
        _imp->had_package_targets = true;
        if (! _imp->override_target_type)
            _imp->dep_list.options()->target_type() = dl_target_package;

        if (spec->package_ptr())
        {
            /* no wildcards */
            spec->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
            _imp->targets->root()->append(spec);
        }
        else
        {
            std::shared_ptr<const PackageIDSequence> names((*_imp->env)[selection::BestVersionOnly(
                        generator::Matches(*spec, MatchPackageOptions()) | filter::SupportsAction<InstallAction>())]);

            if (names->empty())
            {
                /* no match. we'll get an error from this later anyway. */
                spec->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
                _imp->targets->root()->append(spec);
            }
            else
                for (PackageIDSequence::ConstIterator i(names->begin()), i_end(names->end()) ;
                        i != i_end ; ++i)
                {
                    PartiallyMadePackageDepSpec p(*spec);
                    p.package((*i)->name());
                    std::shared_ptr<PackageDepSpec> specn(new PackageDepSpec(p));
                    specn->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
                    _imp->targets->root()->append(specn);
                }
        }

        _imp->raw_targets.push_back(stringify(*spec));
    }
    catch (const GotASetNotAPackageDepSpec &)
    {
        if (_imp->had_set_targets)
            throw MultipleSetTargetsSpecified();
        if (_imp->had_package_targets)
            throw HadBothPackageAndSetTargets();
        _imp->had_set_targets = true;

        _imp->targets->root()->append(std::make_shared<NamedSetDepSpec>(SetName(target)));
        if (! _imp->override_target_type)
            _imp->dep_list.options()->target_type() = dl_target_set;

        _imp->raw_targets.push_back(stringify(target));
    }
}

void
InstallTask::_add_package_id(const std::shared_ptr<const PackageID> & target)
{
    Context context("When adding install target '" + stringify(*target) + "' from ID:");

    if (_imp->had_set_targets)
    {
        _imp->had_resolution_failures = true;
        throw HadBothPackageAndSetTargets();
    }

    _imp->had_package_targets = true;
    if (! _imp->override_target_type)
        _imp->dep_list.options()->target_type() = dl_target_package;

    PartiallyMadePackageDepSpec part_spec((PartiallyMadePackageDepSpecOptions()));
    part_spec.package(target->name());
    part_spec.in_repository(target->repository()->name());
    part_spec.version_requirement(make_named_values<VersionRequirement>(
                n::version_operator() = vo_equal,
                n::version_spec() = target->version()));

    if (target->slot_key())
        part_spec.slot_requirement(std::make_shared<UserSlotExactRequirement>(target->slot_key()->value()));

    std::shared_ptr<PackageDepSpec> spec(make_shared_copy(PackageDepSpec(part_spec)));
    spec->set_tag(std::shared_ptr<const DepTag>(new TargetDepTag));
    _imp->targets->root()->append(spec);

    _imp->raw_targets.push_back(stringify(*spec));
}

void
InstallTask::_build_dep_list()
{
    Context context("When building dependency list:");

    on_build_deplist_pre();
    _imp->dep_list.add(*_imp->targets, _imp->destinations);
    on_build_deplist_post();
}

void
InstallTask::_display_task_list()
{
    Context context("When displaying task list:");

    if (_imp->pretend &&
        0 != perform_hook(Hook("install_pretend_pre")
                                    ("TARGETS", join(
                                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                                    ("DEPLIST_HAS_ERRORS", stringify(_imp->dep_list.has_errors()))
                        ).max_exit_status())
        throw ActionAbortedError("Pretend install aborted by hook");

    on_display_merge_list_pre();

    /* display our task list */
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_pre")
                    ("TARGET", stringify(*dep->package_id()))
                    ("KIND", stringify(dep->kind()))).max_exit_status())
            throw ActionAbortedError("Pretend install aborted by hook");

        on_display_merge_list_entry(*dep);

        if (_imp->pretend &&
                0 != perform_hook(Hook("install_pretend_display_item_post")
                    ("TARGET", stringify(*dep->package_id()))
                    ("KIND", stringify(dep->kind()))).max_exit_status())
            throw ActionAbortedError("Pretend install aborted by hook");
    }

    /* we're done displaying our task list */
    on_display_merge_list_post();
}

namespace
{
    struct SummaryVisitor
    {
        int total, successes, skipped, failures, unreached;
        InstallTask & task;
        const DepListEntry * entry;

        SummaryVisitor(InstallTask & t) :
            total(0),
            successes(0),
            skipped(0),
            failures(0),
            unreached(0),
            task(t),
            entry(0)
        {
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied & s)
        {
            ++skipped;
            ++total;
            task.on_display_failure_summary_skipped_unsatisfied(*entry, s.spec());
        }

        void visit(const DepListEntryHandledSkippedDependent & s)
        {
            ++skipped;
            ++total;
            task.on_display_failure_summary_skipped_dependent(*entry, s.id());
        }

        void visit(const DepListEntryHandledFailed &)
        {
            ++failures;
            ++total;
            task.on_display_failure_summary_failure(*entry);
        }

        void visit(const DepListEntryHandledFetchFailed &)
        {
            ++failures;
            ++total;
            task.on_display_failure_summary_failure(*entry);
        }

        void visit(const DepListEntryUnhandled &)
        {
            ++unreached;
            ++total;
        }

        void visit(const DepListEntryHandledFetchSuccess &)
        {
            ++unreached;
            ++total;
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
        }

        void visit(const DepListEntryHandledSuccess &)
        {
            ++successes;
            ++total;
            task.on_display_failure_summary_success(*entry);
        }
    };
}

void
InstallTask::_display_failure_summary()
{
    Context context("When displaying summary:");

    if (! had_action_failures())
        return;

    on_display_failure_summary_pre();

    /* display our summary */
    SummaryVisitor s(*this);
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        s.entry = &*dep;
        dep->handled()->accept(s);
    }

    /* we're done displaying our task list */
    on_display_failure_summary_totals(s.total, s.successes, s.skipped, s.failures, s.unreached);
    on_display_failure_summary_post();
}

bool
InstallTask::_pretend()
{
    Context context("When performing pretend actions:");

    bool pretend_failed(false);

    on_pretend_all_pre();

    SupportsActionTest<PretendAction> pretend_action_query;
    SupportsActionTest<FetchAction> fetch_action_query;
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if ((dlk_package != dep->kind()) || already_done(*dep))
            continue;

        if (dep->package_id()->supports_action(pretend_action_query) ||
                dep->package_id()->supports_action(fetch_action_query))
        {
            on_pretend_pre(*dep);

            OutputManagerFromEnvironment output_manager_holder(_imp->env, dep->package_id(), oe_exclusive,
                    ClientOutputFeatures());

            bool success(true);
            if (dep->package_id()->supports_action(pretend_action_query))
            {
                PretendActionOptions options(make_named_values<PretendActionOptions>(
                            n::make_output_manager() = std::ref(output_manager_holder)
                            ));
                PretendAction pretend_action(options);
                dep->package_id()->perform_action(pretend_action);
                if (pretend_action.failed())
                {
                    pretend_failed = true;
                    success = false;
                    dep->handled().reset(new DepListEntryHandledFailed);
                }
            }

            if (dep->package_id()->supports_action(fetch_action_query))
            {
                FetchActionOptions options(make_named_values<FetchActionOptions>(
                            n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                            n::exclude_unmirrorable() = false,
                            n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
                            n::ignore_not_in_manifest() = false,
                            n::ignore_unfetched() = true,
                            n::make_output_manager() = std::ref(output_manager_holder),
                            n::safe_resume() = _imp->safe_resume,
                            n::want_phase() = std::bind(return_literal_function(wp_yes))
                            ));
                FetchAction fetch_action(options);
                try
                {
                    dep->package_id()->perform_action(fetch_action);
                }
                catch (const ActionFailedError & e)
                {
                    pretend_failed = true;
                    success = false;
                    if (output_manager_holder.output_manager_if_constructed())
                        on_fetch_action_error(output_manager_holder.output_manager_if_constructed(), e,
                                options.errors());
                    else
                        on_fetch_action_error(std::make_shared<StandardOutputManager>(), e,
                                options.errors());
                }
            }

            if (success)
            {
                if (output_manager_holder.output_manager_if_constructed())
                    output_manager_holder.output_manager_if_constructed()->succeeded();
            }

            on_pretend_post(*dep);
        }
    }

    on_pretend_all_post();

    if (_imp->pretend)
    {
        if (0 != perform_hook(Hook("install_pretend_post")
                                    ("TARGETS", join(
                                        _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                                    ("DEPLIST_HAS_ERRORS", stringify(_imp->dep_list.has_errors()))
                        ).max_exit_status())
            throw ActionAbortedError("Pretend install aborted by hook");
    }

    return pretend_failed;
}

void
InstallTask::_clean(
        const DepList::Iterator dep,
        const std::shared_ptr<const PackageID> & id,
        const UninstallActionOptions & options,
        const std::string & cpvr,
        const int x, const int y, const int s, const int f)
{
    /* clean one item */
    if (0 != perform_hook(Hook("clean_pre")("TARGET", stringify(*id))
                 ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
        throw ActionAbortedError("Clean of '" + cpvr + "' aborted by hook");
    on_clean_pre(*dep, *id, x, y, s, f);

    try
    {
        UninstallAction uninstall_action(options);
        id->perform_action(uninstall_action);
    }
    catch (const ActionFailedError & e)
    {
        on_clean_fail(*dep, *id, x, y, s, f);
        HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("clean_fail")
                    ("TARGET", stringify(*id))("MESSAGE", e.message())));
        throw;
    }

    on_clean_post(*dep, *id, x, y, s, f);
    if (0 != perform_hook(Hook("clean_post")("TARGET", stringify(*id))
                 ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
        throw ActionAbortedError("Clean of '" + cpvr + "' aborted by hook");
}

void
InstallTask::_one(const DepList::Iterator dep, const int x, const int y, const int s, const int f, const bool is_first, const bool is_last,
        std::shared_ptr<OutputManagerFromEnvironment> & output_manager_holder)
{
    std::string cpvr(stringify(*dep->package_id()));

    bool live_destination(false);
    if (dep->destination())
        if ((*dep->destination()).destination_interface() && (*dep->destination()).destination_interface()->want_pre_post_phases())
            live_destination = true;

    if (already_done(*dep))
    {
        on_skip_already_done(*dep, x, y, s, f);
        return;
    }

    /* we're about to fetch / install one item */
    if (_imp->fetch_only)
    {
        if (0 != perform_hook(Hook("fetch_pre")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
            throw ActionAbortedError("Fetch of '" + cpvr + "' aborted by hook");
        on_fetch_pre(*dep, x, y, s, f);
    }
    else
    {
        if (0 != perform_hook(Hook("install_pre")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))
                     ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status())
            throw ActionAbortedError("Install of '" + cpvr + "' aborted by hook");
        on_install_pre(*dep, x, y, s, f);
    }

    /* fetch / install one item */
    try
    {
        SupportsActionTest<FetchAction> test_fetch;
        if (dep->package_id()->supports_action(test_fetch))
        {
            output_manager_holder.reset(new OutputManagerFromEnvironment(_imp->env, dep->package_id(), oe_exclusive,
                        ClientOutputFeatures()));
            FetchActionOptions fetch_options(make_fetch_action_options(*dep, *output_manager_holder));
            FetchAction fetch_action(fetch_options);
            dep->package_id()->perform_action(fetch_action);
            if (output_manager_holder->output_manager_if_constructed())
                output_manager_holder->output_manager_if_constructed()->succeeded();
            output_manager_holder.reset();
            dep->handled().reset(new DepListEntryHandledFetchSuccess);
        }

        if (! _imp->fetch_only)
        {
            output_manager_holder.reset(new OutputManagerFromEnvironment(_imp->env, dep->package_id(),
                        oe_exclusive, ClientOutputFeatures()));

            std::shared_ptr<PackageIDSequence> replacing;

            // look for packages with the same name in the same slot in the destination repos
            if (dep->destination())
                replacing = (*_imp->env)[selection::AllVersionsSorted(
                        (generator::Package(dep->package_id()->name()) &
                        generator::InRepository(dep->destination()->name())) |
                        filter::SupportsAction<UninstallAction>() |
                        filter::SameSlot(dep->package_id()))];
            else
                replacing.reset(new PackageIDSequence);

            InstallActionOptions install_options(make_named_values<InstallActionOptions>(
                        n::destination() = dep->destination(),
                        n::make_output_manager() = std::ref(*output_manager_holder),
                        n::perform_uninstall() = std::bind(&InstallTask::_clean, this, dep,
                                std::placeholders::_1, std::placeholders::_2, cpvr, x, y, s, f),
                        n::replacing() = replacing,
                        n::want_phase() = std::function<WantPhase (const std::string &)>()
                    ));

            bool done_any(false);

            bool apply_phases(false);
            if (! _imp->abort_at_phases->empty() || ! _imp->skip_phases->empty() || ! _imp->skip_until_phases->empty())
            {
                if (is_first && _imp->phase_options_apply_to_first)
                    apply_phases = true;
                if (is_last && _imp->phase_options_apply_to_last)
                    apply_phases = true;
                if (_imp->phase_options_apply_to_all)
                    apply_phases = true;
            }
            if (apply_phases)
                install_options.want_phase() = std::bind(&want_phase_function, this, std::ref(*output_manager_holder),
                    std::cref(_imp->abort_at_phases), std::cref(_imp->skip_phases), std::cref(_imp->skip_until_phases),
                    std::ref(done_any), std::placeholders::_1);
            else
                install_options.want_phase() = std::bind(&want_all_phases_function, this, std::ref(*output_manager_holder),
                    std::ref(done_any), std::placeholders::_1);

            InstallAction install_action(install_options);
            dep->package_id()->perform_action(install_action);

            if (output_manager_holder->output_manager_if_constructed())
                output_manager_holder->output_manager_if_constructed()->succeeded();

            output_manager_holder.reset();
        }
    }
    catch (const ActionFailedError & e)
    {
        on_install_fail(*dep, x, y, s, f);
        HookResult PALUDIS_ATTRIBUTE((unused)) dummy(perform_hook(Hook("install_fail")("TARGET", cpvr)("MESSAGE", e.message())));
        throw;
    }

    /* we've fetched / installed one item */
    if (_imp->fetch_only)
    {
        on_fetch_post(*dep, x, y, s, f);
        if (0 != perform_hook(Hook("fetch_post")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status())
            throw ActionAbortedError("Fetch of '" + cpvr + "' aborted by hook");
    }
    else
    {
        on_install_post(*dep, x, y, s, f);
        if (0 != perform_hook(Hook("install_post")
                     ("TARGET", cpvr)
                     ("X_OF_Y", stringify(x) + " of " + stringify(y))
                     ("PALUDIS_NO_LIVE_DESTINATION", live_destination ? "" : "yes")).max_exit_status())
            throw ActionAbortedError("Install of '" + cpvr + "' aborted by hook");
    }

    if (_imp->fetch_only || ! live_destination)
        return;

    // manually invalidate repos, they're probably wrong now
    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
        (*r)->invalidate();

    dep->handled().reset(new DepListEntryHandledSuccess);

    /* if we installed paludis and a re-exec is available, use it. */
    if (_imp->env->is_paludis_package(dep->package_id()->name()))
    {
        DepList::Iterator d(dep);
        do
        {
            ++d;
            if (d == _imp->dep_list.end())
                break;
        }
        while (dlk_package != d->kind());

        if (d != _imp->dep_list.end())
            on_installed_paludis();
    }
}

void
InstallTask::_main_actions_pre_hooks()
{
    /* we're about to fetch / install the entire list */
    if (_imp->fetch_only)
    {
        if (0 != perform_hook(Hook("fetch_all_pre")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status())
            throw ActionAbortedError("Fetch aborted by hook");
        on_fetch_all_pre();
    }
    else
    {
        bool any_live_destination(false);
        for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
                dep != dep_end && ! any_live_destination ; ++dep)
            if (dlk_package == dep->kind() && dep->destination())
                if ((*dep->destination()).destination_interface() && (*dep->destination()).destination_interface()->want_pre_post_phases())
                    any_live_destination = true;

        if (0 != perform_hook(Hook("install_all_pre")
                     ("TARGETS", join(_imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                     ("PALUDIS_NO_LIVE_DESTINATION", any_live_destination ? "" : "yes")).max_exit_status())
            throw ActionAbortedError("Install aborted by hook");
        on_install_all_pre();
    }
}

void
InstallTask::_main_actions_all(const int y, const DepList::Iterator dep_last_package)
{
    int x(0), s(0), f(0);
    bool is_first(true), is_last(false);

    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        if (dlk_package != dep->kind())
            continue;

        is_last = (dep == dep_last_package);

        ++x;

        if (had_action_failures())
        {
            switch (_imp->continue_on_failure)
            {
                case itcof_if_fetch_only:
                    if (_imp->fetch_only)
                        break;
                    ++s;
                    continue;

                case itcof_never:
                    ++s;
                    continue;

                case itcof_if_satisfied:
                    {
                        std::shared_ptr<const PackageDepSpec> d(_unsatisfied(*dep));
                        if (! d)
                            break;
                        dep->handled().reset(new DepListEntryHandledSkippedUnsatisfied(*d));
                        on_skip_unsatisfied(*dep, *d, x, y, s, f);
                        ++s;
                        continue;
                    }

                case itcof_if_independent:
                    {
                        std::shared_ptr<const PackageID> d(_dependent(*dep));
                        if (! d)
                            break;
                        dep->handled().reset(new DepListEntryHandledSkippedDependent(d));
                        on_skip_dependent(*dep, d, x, y, s, f);
                        ++s;
                        continue;
                    }

                case itcof_always:
                    break;

                case last_itcof:
                    throw InternalError(PALUDIS_HERE, "Bad continue_on_failure");
            }
        }

        std::shared_ptr<OutputManagerFromEnvironment> output_manager_holder;
        try
        {
            _one(dep, x, y, s, f, is_first, is_last, output_manager_holder);
        }
        catch (const ActionFailedError & e)
        {
            dep->handled().reset(new DepListEntryHandledFailed);
            if (output_manager_holder && output_manager_holder->output_manager_if_constructed())
                on_non_fetch_action_error(output_manager_holder->output_manager_if_constructed(), e);
            else
                on_non_fetch_action_error(std::make_shared<StandardOutputManager>(), e);
            ++f;
        }

        is_first = false;
    }
}


void
InstallTask::_main_actions()
{
    using namespace std::placeholders;

    _main_actions_pre_hooks();

    /* fetch / install our entire list */
    int y(0);
    DepList::Iterator dep_last_package(_imp->dep_list.end());
    for (DepList::Iterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
        if (dep->kind() == dlk_package)
        {
            dep_last_package = dep;
            ++y;
        }

    _main_actions_all(y, dep_last_package);

    /* go no further if we had failures */
    if (had_action_failures())
    {
        _display_failure_summary();
        return;
    }

    _main_actions_post_hooks();
    _do_world_updates();
}

void
InstallTask::_do_world_updates()
{
    /* update world */
    if (! _imp->fetch_only)
    {
        if (! _imp->preserve_world)
        {
            on_update_world_pre();

            if (_imp->add_to_world_spec)
            {
                bool s_had_package_targets(_imp->had_package_targets), s_had_set_targets(_imp->had_set_targets);
                if (_imp->add_to_world_spec)
                {
                    s_had_package_targets = ((std::string::npos != _imp->add_to_world_spec->find('/')));
                    s_had_set_targets = (! s_had_package_targets) && (std::string::npos != _imp->add_to_world_spec->find_first_not_of(
                                "() \t\r\n"));
                }

                std::shared_ptr<SetSpecTree> all(new SetSpecTree(std::make_shared<AllDepSpec>()));
                std::list<std::string> tokens;
                tokenise_whitespace(*_imp->add_to_world_spec, std::back_inserter(tokens));
                if ((! tokens.empty()) && ("(" == *tokens.begin()) && (")" == *previous(tokens.end())))
                {
                    tokens.erase(tokens.begin());
                    tokens.erase(previous(tokens.end()));
                }

                for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    if (s_had_package_targets)
                        all->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(*t, _imp->env,
                                            UserPackageDepSpecOptions())));
                    else
                        all->root()->append(std::make_shared<NamedSetDepSpec>(SetName(*t)));
                }

                if (s_had_package_targets)
                    world_update_packages(all);
                else if (s_had_set_targets)
                    world_update_set(SetName(*_imp->add_to_world_spec));
            }
            else
            {
                if (_imp->had_package_targets)
                    world_update_packages(_imp->targets);
                else if (_imp->had_set_targets)
                {
                    for (std::list<std::string>::const_iterator t(_imp->raw_targets.begin()), t_end(_imp->raw_targets.end()) ;
                            t != t_end ; ++t)
                        world_update_set(SetName(*t));
                }
            }

            on_update_world_post();
        }
        else
            on_preserve_world();
    }
}

void
InstallTask::_main_actions_post_hooks()
{
    /* we've fetched / installed the entire list */
    if (_imp->fetch_only)
    {
        on_fetch_all_post();
        if (0 != perform_hook(Hook("fetch_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status())
            throw ActionAbortedError("Fetch aborted by hook");
    }
    else
    {
        on_install_all_post();
        if (0 != perform_hook(Hook("install_all_post")("TARGETS", join(
                             _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))).max_exit_status())
            throw ActionAbortedError("Install aborted by hook");
    }
}

void
InstallTask::_execute()
{
    Context context("When executing install task:");

    _build_dep_list();
    _display_task_list();
    bool pretend_failed(_pretend());

    if (_imp->dep_list.has_errors() || pretend_failed)
    {
        on_not_continuing_due_to_errors();
        return;
    }

    if (_imp->pretend)
        return;

    _main_actions();
}

void
InstallTask::execute()
{
    bool success(false);
    try
    {
        if (0 != perform_hook(Hook("install_task_execute_pre")
                    ("TARGETS", join(
                                     _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                    ("PRETEND", stringify(_imp->pretend))
                    ("FETCH_ONLY", stringify(_imp->fetch_only))
                    ("DEPLIST_HAS_ERRORS", stringify(_imp->dep_list.has_errors()))
                    ).max_exit_status())
            throw ActionAbortedError("Install task execute aborted by hook");

        _execute();
        success = true;
    }
    catch (const AmbiguousPackageNameError & e)
    {
        _imp->had_resolution_failures = true;
        on_ambiguous_package_name_error(e);
    }
    catch (const NoSuchPackageError & e)
    {
        _imp->had_resolution_failures = true;
        on_no_such_package_error(e);
    }
    catch (const AllMaskedError & e)
    {
        _imp->had_resolution_failures = true;
        on_all_masked_error(e);
    }
    catch (const AdditionalRequirementsNotMetError & e)
    {
        _imp->had_resolution_failures = true;
        on_additional_requirements_not_met_error(e);
    }
    catch (const DepListError & e)
    {
        _imp->had_resolution_failures = true;
        on_dep_list_error(e);
    }
    catch (const HadBothPackageAndSetTargets & e)
    {
        _imp->had_resolution_failures = true;
        on_had_both_package_and_set_targets_error(e);
    }
    catch (const MultipleSetTargetsSpecified & e)
    {
        _imp->had_resolution_failures = true;
        on_multiple_set_targets_specified(e);
    }

    if (0 != perform_hook(Hook("install_task_execute_post")
                ("TARGETS", join(
                                 _imp->raw_targets.begin(), _imp->raw_targets.end(), " "))
                ("PRETEND", stringify(_imp->pretend))
                ("FETCH_ONLY", stringify(_imp->fetch_only))
                ("SUCCESS", stringify(success))
                ("DEPLIST_HAS_ERRORS", stringify(_imp->dep_list.has_errors()))
                ).max_exit_status())
        throw ActionAbortedError("Install task execute aborted by hook");
}

const DepList &
InstallTask::dep_list() const
{
    return _imp->dep_list;
}

void
InstallTask::set_fetch_only(const bool value)
{
    _imp->fetch_only = value;
}

void
InstallTask::set_pretend(const bool value)
{
    _imp->pretend = value;
}

void
InstallTask::set_preserve_world(const bool value)
{
    _imp->preserve_world = value;
}

void
InstallTask::set_add_to_world_spec(const std::string & value)
{
    _imp->add_to_world_spec.reset(new std::string(value));
}

InstallTask::TargetsConstIterator
InstallTask::begin_targets() const
{
    return TargetsConstIterator(_imp->raw_targets.begin());
}

InstallTask::TargetsConstIterator
InstallTask::end_targets() const
{
    return TargetsConstIterator(_imp->raw_targets.end());
}

Environment *
InstallTask::environment()
{
    return _imp->env;
}

const Environment *
InstallTask::environment() const
{
    return _imp->env;
}

void
InstallTask::on_installed_paludis()
{
}

void
InstallTask::set_safe_resume(const bool value)
{
    _imp->safe_resume = value;
}

HookResult
InstallTask::perform_hook(const Hook & hook)
{
    return _imp->env->perform_hook(hook);
}

void
InstallTask::override_target_type(const DepListTargetType t)
{
    _imp->override_target_type = true;
    _imp->dep_list.options()->target_type() = t;
}

void
InstallTask::world_update_set(const SetName & s)
{
    if (s == SetName("world") || s == SetName("system") || s == SetName("security")
            || s == SetName("everything") || s == SetName("installed-packages")
            || s == SetName("installed-slots") || s == SetName("insecurity"))
    {
        on_update_world_skip(s, "special sets cannot be added to world");
        return;
    }

    _imp->env->add_to_world(s);

    on_update_world(s);
}

namespace
{
    struct WorldTargetFinder
    {
        Environment * const env;
        InstallTask * const task;

        WorldTargetFinder(Environment * const e, InstallTask * const t) :
            env(e),
            task(t)
        {
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (package_dep_spec_has_properties(*node.spec(), make_named_values<PackageDepSpecProperties>(
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
                env->add_to_world(*node.spec()->package_ptr());
            }
            else
                task->on_update_world_skip(*node.spec(), "not a simple cat/pkg");
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }
    };
}

void
InstallTask::world_update_packages(const std::shared_ptr<const SetSpecTree> & a)
{
    WorldTargetFinder w(_imp->env, this);
    a->root()->accept(w);
}

bool
InstallTask::had_resolution_failures() const
{
    return _imp->had_resolution_failures;
}

void
InstallTask::set_continue_on_failure(const InstallTaskContinueOnFailure c)
{
    _imp->continue_on_failure = c;
}

namespace
{
    struct CheckSatisfiedVisitor
    {
        const Environment * const env;
        const PackageID & id;
        std::shared_ptr<const PackageDepSpec> failure;
        std::set<SetName> recursing_sets;

        CheckSatisfiedVisitor(const Environment * const e,
                const PackageID & i) :
            env(e),
            id(i)
        {
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (! failure)
                if ((*env)[selection::SomeArbitraryVersion(generator::Matches(*node.spec(), MatchPackageOptions())
                            | filter::InstalledAtRoot(env->root()))]->empty())
                    failure = node.spec();
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                        accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            if (failure)
                return;

            failure.reset();
            for (DependencySpecTree::NodeType<ConditionalDepSpec>::Type::ConstIterator cur(node.begin()) ;
                    cur != node.end() ; ++cur)
            {
                failure.reset();
                (*cur)->accept(*this);
                if (! failure)
                    break;
            }
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            std::shared_ptr<const SetSpecTree> set(env->set(node.spec()->name()));

            if (! set)
            {
                Log::get_instance()->message("install_task.unknown_set", ll_warning, lc_context) << "Unknown set '"
                    << node.spec()->name() << "'";
                return;
            }

            if (! recursing_sets.insert(node.spec()->name()).second)
            {
                Log::get_instance()->message("install_task.recursive_set", ll_warning, lc_context)
                    << "Recursively defined set '" << node.spec()->name() << "'";
                return;
            }

            set->root()->accept(*this);

            recursing_sets.erase(node.spec()->name());
        }
    };
}

std::shared_ptr<const PackageDepSpec>
InstallTask::_unsatisfied(const DepListEntry & e) const
{
    Context context("When checking whether dependencies for '" + stringify(*e.package_id()) + "' are satisfied:");

    CheckSatisfiedVisitor v(environment(), *e.package_id());

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_pre() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_pre())
        if (e.package_id()->build_dependencies_key())
            e.package_id()->build_dependencies_key()->value()->root()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_runtime() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_runtime())
        if (e.package_id()->run_dependencies_key())
            e.package_id()->run_dependencies_key()->value()->root()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_post() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_post())
        if (e.package_id()->post_dependencies_key())
            e.package_id()->post_dependencies_key()->value()->root()->accept(v);

    if ((dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_suggested() ||
                dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_suggested())
            && dl_suggested_install == _imp->dep_list.options()->suggested())
        if (e.package_id()->suggested_dependencies_key())
            e.package_id()->suggested_dependencies_key()->value()->root()->accept(v);

    return v.failure;
}

namespace
{
    struct CheckHandledVisitor
    {
        bool failure;
        bool skipped;
        bool success;

        CheckHandledVisitor() :
            failure(false),
            skipped(false),
            success(false)
        {
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied &)
        {
            skipped = true;
        }

        void visit(const DepListEntryHandledSuccess &)
        {
            success = true;
        }

        void visit(const DepListEntryHandledSkippedDependent &)
        {
            skipped = true;
        }

        void visit(const DepListEntryHandledFailed &)
        {
            failure = true;
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
        }

        void visit(const DepListEntryUnhandled &)
        {
        }

        void visit(const DepListEntryHandledFetchFailed &)
        {
            failure = true;
        }

        void visit(const DepListEntryHandledFetchSuccess &)
        {
        }
    };

    struct CheckIndependentVisitor
    {
        const Environment * const env;
        const DepList & dep_list;
        const std::shared_ptr<const PackageID> id;
        std::shared_ptr<PackageIDSet> already_checked;

        std::shared_ptr<const PackageID> failure;
        std::set<SetName> recursing_sets;

        CheckIndependentVisitor(
                const Environment * const e,
                const DepList & d,
                const std::shared_ptr<const PackageID> & i,
                const std::shared_ptr<PackageIDSet> & a) :
            env(e),
            dep_list(d),
            id(i),
            already_checked(a)
        {
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (failure)
                return;

            for (DepList::ConstIterator d(dep_list.begin()), d_end(dep_list.end()) ;
                    d != d_end ; ++d)
            {
                if (! d->handled())
                    continue;

                if (! match_package(*env, *node.spec(), *d->package_id(), MatchPackageOptions()))
                    continue;

                CheckHandledVisitor v;
                d->handled()->accept(v);

                if (v.failure || v.skipped)
                    failure = d->package_id();
                else if (v.success)
                    return;
            }

            /* no match on the dep list, fall back to installed packages. if
             * there are no matches here it's not a problem because of or-deps. */
            std::shared_ptr<const PackageIDSequence> installed((*env)[selection::AllVersionsUnsorted(
                        generator::Matches(*node.spec(), MatchPackageOptions()) |
                        filter::InstalledAtRoot(env->root()))]);

            for (PackageIDSequence::ConstIterator i(installed->begin()), i_end(installed->end()) ;
                    i != i_end ; ++i)
            {
                if (already_checked->end() != already_checked->find(*i))
                    continue;
                already_checked->insert(*i);

                CheckIndependentVisitor v(env, dep_list, *i, already_checked);

                if (dl_deps_pre == dep_list.options()->uninstalled_deps_pre() ||
                        dl_deps_pre_or_post == dep_list.options()->uninstalled_deps_pre())
                    if ((*i)->build_dependencies_key())
                        (*i)->build_dependencies_key()->value()->root()->accept(v);

                if (dl_deps_pre == dep_list.options()->uninstalled_deps_runtime() ||
                        dl_deps_pre_or_post == dep_list.options()->uninstalled_deps_runtime())
                    if ((*i)->run_dependencies_key())
                        (*i)->run_dependencies_key()->value()->root()->accept(v);

                if (dl_deps_pre == dep_list.options()->uninstalled_deps_post() ||
                        dl_deps_pre_or_post == dep_list.options()->uninstalled_deps_post())
                    if ((*i)->post_dependencies_key())
                        (*i)->post_dependencies_key()->value()->root()->accept(v);

                if ((dl_deps_pre == dep_list.options()->uninstalled_deps_suggested() ||
                            dl_deps_pre_or_post == dep_list.options()->uninstalled_deps_suggested())
                        && dl_suggested_install == dep_list.options()->suggested())
                    if ((*i)->suggested_dependencies_key())
                        (*i)->suggested_dependencies_key()->value()->root()->accept(v);

                if (v.failure)
                {
                    failure = v.failure;
                    return;
                }
            }
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (node.spec()->condition_met())
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()),
                        accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            std::shared_ptr<const SetSpecTree> set(env->set(node.spec()->name()));

            if (! set)
            {
                Log::get_instance()->message("install_task.unknown_set", ll_warning, lc_context) << "Unknown set '" << node.spec()->name() << "'";
                return;
            }

            if (! recursing_sets.insert(node.spec()->name()).second)
            {
                Log::get_instance()->message("install_task.recursive_set", ll_warning, lc_context)
                    << "Recursively defined set '" << node.spec()->name() << "'";
                return;
            }

            set->root()->accept(*this);

            recursing_sets.erase(node.spec()->name());
        }
    };
}

bool
InstallTask::had_action_failures() const
{
    for (DepList::ConstIterator dep(_imp->dep_list.begin()), dep_end(_imp->dep_list.end()) ;
            dep != dep_end ; ++dep)
    {
        CheckHandledVisitor v;
        dep->handled()->accept(v);
        if (v.failure)
            return true;
    }
    return false;
}


std::shared_ptr<const PackageID>
InstallTask::_dependent(const DepListEntry & e) const
{
    Context context("When checking whether dependencies for '" + stringify(*e.package_id()) + "' are independent of failed packages:");

    std::shared_ptr<PackageIDSet> already_checked(new PackageIDSet);
    CheckIndependentVisitor v(environment(), _imp->dep_list, e.package_id(), already_checked);
    already_checked->insert(e.package_id());

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_pre() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_pre())
        if (e.package_id()->build_dependencies_key())
            e.package_id()->build_dependencies_key()->value()->root()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_runtime() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_runtime())
        if (e.package_id()->run_dependencies_key())
            e.package_id()->run_dependencies_key()->value()->root()->accept(v);

    if (dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_post() ||
            dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_post())
        if (e.package_id()->post_dependencies_key())
            e.package_id()->post_dependencies_key()->value()->root()->accept(v);

    if ((dl_deps_pre == _imp->dep_list.options()->uninstalled_deps_suggested() ||
                dl_deps_pre_or_post == _imp->dep_list.options()->uninstalled_deps_suggested())
            && dl_suggested_install == _imp->dep_list.options()->suggested())
        if (e.package_id()->suggested_dependencies_key())
            e.package_id()->suggested_dependencies_key()->value()->root()->accept(v);

    return v.failure;
}

namespace
{
    struct AlreadyDoneVisitor
    {
        bool result;

        void visit(const DepListEntryHandledSuccess &)
        {
            result = true;
        }

        void visit(const DepListEntryHandledSkippedUnsatisfied &)
        {
            result = true;
        }

        void visit(const DepListEntryHandledSkippedDependent &)
        {
            result = true;
        }

        void visit(const DepListEntryHandledFailed &)
        {
            result = true;
        }

        void visit(const DepListEntryUnhandled &)
        {
            result = false;
        }

        void visit(const DepListEntryNoHandlingRequired &)
        {
            result = false;
        }

        void visit(const DepListEntryHandledFetchFailed &)
        {
            result = false;
        }

        void visit(const DepListEntryHandledFetchSuccess &)
        {
            result = false;
        }
    };
}

bool
InstallTask::already_done(const DepListEntry & e) const
{
    AlreadyDoneVisitor v;
    e.handled()->accept(v);
    return v.result;
}

void
InstallTask::set_skip_phases(const std::shared_ptr<const Set<std::string> > & s)
{
    _imp->skip_phases = s;
}

void
InstallTask::set_skip_until_phases(const std::shared_ptr<const Set<std::string> > & s)
{
    _imp->skip_until_phases = s;
}

void
InstallTask::set_abort_at_phases(const std::shared_ptr<const Set<std::string> > & s)
{
    _imp->abort_at_phases = s;
}

void
InstallTask::set_phase_options_apply_to_first(const bool b)
{
    _imp->phase_options_apply_to_first = b;
}

void
InstallTask::set_phase_options_apply_to_last(const bool b)
{
    _imp->phase_options_apply_to_last = b;
}

void
InstallTask::set_phase_options_apply_to_all(const bool b)
{
    _imp->phase_options_apply_to_all = b;
}

FetchActionOptions
InstallTask::make_fetch_action_options(const DepListEntry &, OutputManagerFromEnvironment & o) const
{
    return make_named_values<FetchActionOptions>(
            n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
            n::exclude_unmirrorable() = false,
            n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
            n::ignore_not_in_manifest() = false,
            n::ignore_unfetched() = false,
            n::make_output_manager() = std::ref(o),
            n::safe_resume() = _imp->safe_resume,
            n::want_phase() = std::bind(return_literal_function(wp_yes))
            );
}

template class WrappedForwardIterator<InstallTask::TargetsConstIteratorTag, const std::string>;

