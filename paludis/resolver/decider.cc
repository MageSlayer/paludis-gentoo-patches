/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2013, 2014 Ciaran McCreesh
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

#include <paludis/resolver/decider.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/collect_depped_upon.hh>
#include <paludis/resolver/collect_installed.hh>
#include <paludis/resolver/collect_purges.hh>
#include <paludis/resolver/accumulate_deps.hh>
#include <paludis/resolver/why_changed_choices.hh>
#include <paludis/resolver/same_slot.hh>
#include <paludis/resolver/reason_utils.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/resolver/has_behaviour-fwd.hh>
#include <paludis/resolver/get_sameness.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/repository.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/match_package.hh>
#include <paludis/version_requirements.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/choice.hh>
#include <paludis/action.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/package_id.hh>
#include <paludis/changed_choices.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/slot.hh>

#include <paludis/util/pimp-impl.hh>

#include <list>
#include <algorithm>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<Decider>
    {
        const Environment * const env;
        const ResolverFunctions fns;

        const std::shared_ptr<ResolutionsByResolvent> resolutions_by_resolvent;

        Imp(const Environment * const e, const ResolverFunctions & f,
                const std::shared_ptr<ResolutionsByResolvent> & l) :
            env(e),
            fns(f),
            resolutions_by_resolvent(l)
        {
        }
    };
}

Decider::Decider(const Environment * const e, const ResolverFunctions & f,
        const std::shared_ptr<ResolutionsByResolvent> & l) :
    _imp(e, f, l)
{
}

Decider::~Decider() = default;

void
Decider::_resolve_decide_with_dependencies()
{
    Context context("When resolving and adding dependencies recursively:");

    enum State { deciding_non_suggestions, deciding_nothings, deciding_suggestions, finished } state = deciding_non_suggestions;
    bool changed(true);
    while (true)
    {
        if (! changed)
            state = State(state + 1);
        if (state == finished)
            break;

        changed = false;
        for (const auto & resolution : *_imp->resolutions_by_resolvent)
        {
            /* we've already decided */
            if (resolution->decision())
                continue;

            /* we're only being suggested. don't do this on the first pass, so
             * we don't have to do restarts for suggestions later becoming hard
             * deps. */
            if (state < deciding_suggestions && resolution->constraints()->all_untaken())
                continue;

            /* avoid deciding nothings until after we've decided things we've
             * taken, so adding extra destinations doesn't get messy. */
            if (state < deciding_nothings && resolution->constraints()->nothing_is_fine_too())
                continue;

            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            changed = true;
            _decide(resolution);

            _add_dependencies_if_necessary(resolution);
        }
    }
}

bool
Decider::_resolve_vias()
{
    Context context("When finding vias:");

    bool changed(false);

    for (const auto & resolution : *_imp->resolutions_by_resolvent)
    {
        if (resolution->resolvent().destination_type() == dt_create_binary)
            continue;

        if (! _imp->fns.always_via_binary_fn()(resolution))
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        Resolvent binary_resolvent(resolution->resolvent());
        binary_resolvent.destination_type() = dt_create_binary;

        bool already(false);
        const std::shared_ptr<Resolution> binary_resolution(_resolution_for_resolvent(binary_resolvent, true));
        for (const auto & constraint : *binary_resolution->constraints())
            if (visitor_cast<const ViaBinaryReason>(*constraint->reason()))
            {
                already = true;
                break;
            }

        if (already)
            continue;

        changed = true;

        const std::shared_ptr<const ConstraintSequence> constraints(_imp->fns.get_constraints_for_via_binary_fn()(binary_resolution, resolution));
        for (const auto & constraint : *constraints)
            _apply_resolution_constraint(binary_resolution, constraint);

        _decide(binary_resolution);
    }

    return changed;
}

bool
Decider::_resolve_dependents()
{
    Context context("When finding dependents:");

    bool changed(false);
    const std::pair<
        std::shared_ptr<const ChangeByResolventSequence>,
        std::shared_ptr<const ChangeByResolventSequence> > changing(_collect_changing());

    if (changing.first->empty())
        return false;

    const std::shared_ptr<const PackageIDSequence> staying(_collect_staying(changing.first));

    for (const auto & package : *staying)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (! package->supports_action(SupportsActionTest<UninstallAction>()))
            continue;

        auto dependent_upon_ids(dependent_upon(_imp->env, package, changing.first, changing.second, staying));
        if (dependent_upon_ids->empty())
            continue;

        Resolvent resolvent(package, dt_install_to_slash);
        bool remove(_imp->fns.remove_if_dependent_fn()(package));

        /* we've changed things if we've not already done anything for this
         * resolvent, but only if we're going to remove it rather than mark it
         * as broken */
        if (remove && _imp->resolutions_by_resolvent->end() == _imp->resolutions_by_resolvent->find(resolvent))
            changed = true;

        auto resolution(_resolution_for_resolvent(resolvent, true));
        auto constraints(_imp->fns.get_constraints_for_dependent_fn()(resolution, package, dependent_upon_ids));
        for (const auto & constraint : *constraints)
            _apply_resolution_constraint(resolution, constraint);

        if ((! remove) && (! resolution->decision()))
        {
            if (! _try_to_find_decision_for(resolution, false, false, false, false, false))
                resolution->decision() = std::make_shared<BreakDecision>(
                        resolvent,
                        package,
                        true);
            else
            {
                /* we'll do the actual deciding plus deps etc later */
                changed = true;
            }
        }
    }

    return changed;
}

namespace
{
    const std::shared_ptr<const PackageID> get_change_by_resolvent_id(const ChangeByResolvent & r)
    {
        return r.package_id();
    }
}

namespace
{
    struct ChangingCollector
    {
        std::shared_ptr<Resolution> current_resolution;

        std::shared_ptr<ChangeByResolventSequence> going_away;
        std::shared_ptr<ChangeByResolventSequence> newly_available;

        ChangingCollector() :
            going_away(std::make_shared<ChangeByResolventSequence>()),
            newly_available(std::make_shared<ChangeByResolventSequence>())
        {
        }

        void visit(const NothingNoChangeDecision &)
        {
        }

        void visit(const ExistingNoChangeDecision &)
        {
        }

        void visit(const RemoveDecision & d)
        {
            for (const auto & package : *d.ids())
                going_away->push_back(make_named_values<ChangeByResolvent>(
                            n::package_id() = package,
                            n::resolvent() = current_resolution->resolvent()
                            ));
        }

        void visit(const ChangesToMakeDecision & d)
        {
            for (const auto & package : *d.destination()->replacing())
                going_away->push_back(make_named_values<ChangeByResolvent>(
                            n::package_id() = package,
                            n::resolvent() = current_resolution->resolvent()
                            ));

            newly_available->push_back(make_named_values<ChangeByResolvent>(
                        n::package_id() = d.origin_id(),
                        n::resolvent() = current_resolution->resolvent()
                        ));
        }

        void visit(const UnableToMakeDecision &)
        {
        }

        void visit(const BreakDecision &)
        {
        }
    };
}

const std::pair<
    std::shared_ptr<const ChangeByResolventSequence>,
    std::shared_ptr<const ChangeByResolventSequence> >
Decider::_collect_changing() const
{
    ChangingCollector c;

    for (const auto & resolution : *_imp->resolutions_by_resolvent)
        if (resolution->decision() && resolution->decision()->taken())
        {
            c.current_resolution = resolution;
            resolution->decision()->accept(c);
        }

    return std::make_pair(c.going_away, c.newly_available);
}

namespace
{
    struct ChangeByResolventPackageIDIs
    {
        const std::shared_ptr<const PackageID> id;

        ChangeByResolventPackageIDIs(const std::shared_ptr<const PackageID> & i) :
            id(i)
        {
        }

        bool operator() (const ChangeByResolvent & r) const
        {
            return *r.package_id() == *id;
        }
    };
}

const std::shared_ptr<const PackageIDSequence>
Decider::_collect_staying(const std::shared_ptr<const ChangeByResolventSequence> & going_away) const
{
    Context context("When collecting staying packages:");

    const std::shared_ptr<const PackageIDSequence> existing((*_imp->env)[selection::AllVersionsUnsorted(
                generator::All() | filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);

    const std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
    for (const auto & package : *existing)
        if (going_away->end() == std::find_if(going_away->begin(), going_away->end(), ChangeByResolventPackageIDIs(package)))
            result->push_back(package);

    return result;
}

void
Decider::_resolve_confirmations()
{
    Context context("When resolving confirmations:");

    for (const auto & resolution : *_imp->resolutions_by_resolvent)
        _confirm(resolution);
}

void
Decider::_fixup_changes_to_make_decision(
        const std::shared_ptr<const Resolution> & resolution,
        ChangesToMakeDecision & decision) const
{
    decision.set_destination(_make_destination_for(resolution, decision));
    decision.set_change_type(_make_change_type_for(resolution, decision));
}

const std::shared_ptr<Destination>
Decider::_make_destination_for(
        const std::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    const std::shared_ptr<const Repository> repo(_imp->fns.find_repository_for_fn()(resolution, decision));
    if ((! repo->destination_interface()) ||
            (! repo->destination_interface()->is_suitable_destination_for(decision.origin_id())))
        throw InternalError(PALUDIS_HERE, stringify(repo->name()) + " is not a suitable destination for "
                + stringify(*decision.origin_id()));

    return std::make_shared<Destination>(make_named_values<Destination>(
                    n::replacing() = _imp->fns.find_replacing_fn()(decision.origin_id(), repo),
                    n::repository() = repo->name()
                    ));
}

const ChangeType
Decider::_make_change_type_for(
        const std::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    Context context("When determining change type for '" + stringify(resolution->resolvent()) + "':");

    if (decision.destination()->replacing()->empty())
    {
        const std::shared_ptr<const PackageIDSequence> others((*_imp->env)[selection::AllVersionsUnsorted(
                    generator::Package(decision.origin_id()->name()) &
                    generator::InRepository(decision.destination()->repository())
                    )]);
        if (others->empty())
            return ct_new;
        else
        {
            for (const auto & other : *others)
                if (same_slot(other, decision.origin_id()))
                    return ct_add_to_slot;

            return ct_slot_new;
        }
    }
    else
    {
        /* we pick the worst, so replacing 1 and 3 with 2 requires permission to
         * downgrade */
        ChangeType result(last_ct);
        for (const auto & package : *decision.destination()->replacing())
        {
            if (package->version() == decision.origin_id()->version())
                result = std::min(result, ct_reinstall);
            else if (package->version() < decision.origin_id()->version())
                result = std::min(result, ct_upgrade);
            else if (package->version() > decision.origin_id()->version())
                result = std::min(result, ct_downgrade);
        }

        return result;
    }
}

const std::shared_ptr<Resolution>
Decider::_create_resolution_for_resolvent(const Resolvent & r) const
{
    return std::make_shared<Resolution>(make_named_values<Resolution>(
                    n::constraints() = _imp->fns.get_initial_constraints_for_fn()(r),
                    n::decision() = nullptr,
                    n::resolvent() = r
                    ));
}

const std::shared_ptr<Resolution>
Decider::_resolution_for_resolvent(const Resolvent & r, const Tribool if_not_exist)
{
    ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->find(r));
    if (_imp->resolutions_by_resolvent->end() == i)
    {
        if (if_not_exist.is_true())
        {
            std::shared_ptr<Resolution> resolution(_create_resolution_for_resolvent(r));
            i = _imp->resolutions_by_resolvent->insert_new(resolution);
        }
        else if (if_not_exist.is_false())
            throw InternalError(PALUDIS_HERE, "resolver bug: expected resolution for "
                    + stringify(r) + " to exist, but it doesn't");
        else
            return nullptr;
    }

    return *i;
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_target(
        const std::shared_ptr<const Resolution> & resolution,
        const PackageOrBlockDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    if (spec.if_package())
    {
        auto existing(_imp->fns.get_use_existing_nothing_fn()(resolution, *spec.if_package(), reason));

        const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());
        result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                            n::destination_type() = resolution->resolvent().destination_type(),
                            n::force_unable() = false,
                            n::from_id() = nullptr,
                            n::nothing_is_fine_too() = existing.second,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = false,
                            n::use_existing() = existing.first
                            )));
        return result;
    }
    else if (spec.if_block())
        return _make_constraints_from_blocker(resolution, *spec.if_block(), reason);
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

const std::shared_ptr<Constraint>
Decider::_make_constraint_from_package_dependency(
        const std::shared_ptr<const Resolution> & resolution,
        const SanitisedDependency & dep,
        const std::shared_ptr<const Reason> & reason,
        const SpecInterest interest) const
{
    auto existing(_imp->fns.get_use_existing_nothing_fn()(resolution, *dep.spec().if_package(), reason));
    return std::make_shared<Constraint>(make_named_values<Constraint>(
                n::destination_type() = resolution->resolvent().destination_type(),
                n::force_unable() = false,
                n::from_id() = dep.from_id(),
                n::nothing_is_fine_too() = existing.second,
                n::reason() = reason,
                n::spec() = *dep.spec().if_package(),
                n::untaken() = si_untaken == interest,
                n::use_existing() = existing.first
                ));
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_dependency(
        const std::shared_ptr<const Resolution> & resolution,
        const SanitisedDependency & dep,
        const std::shared_ptr<const Reason> & reason,
        const SpecInterest interest) const
{
    if (dep.spec().if_package())
    {
        const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());
        result->push_back(_make_constraint_from_package_dependency(resolution, dep, reason, interest));
        return result;
    }
    else if (dep.spec().if_block())
        return _make_constraints_from_blocker(resolution, *dep.spec().if_block(), reason);
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_blocker(
        const std::shared_ptr<const Resolution> & resolution,
        const BlockDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    const std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());

    bool nothing_is_fine_too(true);
    bool force_unable(false);
    switch (find_blocker_role_in_annotations(spec.maybe_annotations()))
    {
        case dsar_blocker_weak:
        case dsar_blocker_strong:
        case dsar_blocker_uninstall_blocked_before:
        case dsar_blocker_uninstall_blocked_after:
            break;

        case dsar_blocker_manual:
            force_unable = ! _block_dep_spec_has_nothing_installed(spec, maybe_from_package_id_from_reason(reason), resolution->resolvent());
            break;

        case dsar_blocker_upgrade_blocked_before:
            nothing_is_fine_too = _block_dep_spec_has_nothing_installed(spec, maybe_from_package_id_from_reason(reason), resolution->resolvent());
            break;

        default:
            throw InternalError(PALUDIS_HERE, "unexpected role");
    }

    DestinationTypes destination_types(_imp->fns.get_destination_types_for_blocker_fn()(spec, reason));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        if (destination_types[*t])
            result->push_back(std::make_shared<Constraint>(make_named_values<Constraint>(
                                n::destination_type() = *t,
                                n::force_unable() = force_unable,
                                n::from_id() = maybe_from_package_id_from_reason(reason),
                                n::nothing_is_fine_too() = nothing_is_fine_too,
                                n::reason() = reason,
                                n::spec() = spec,
                                n::untaken() = false,
                                n::use_existing() = ue_if_possible
                                )));

    return result;
}

void
Decider::_apply_resolution_constraint(
        const std::shared_ptr<Resolution> & resolution,
        const std::shared_ptr<const Constraint> & constraint)
{
    if (resolution->decision())
        if (! _verify_new_constraint(resolution, constraint))
            _made_wrong_decision(resolution, constraint);

    resolution->constraints()->add(constraint);
}

namespace
{
    struct CheckConstraintVisitor
    {
        const Environment * const env;
        const std::shared_ptr<const ChangedChoices> changed_choices_for_constraint;
        const Constraint constraint;

        CheckConstraintVisitor(const Environment * const e,
                const std::shared_ptr<const ChangedChoices> & h,
                const Constraint & c) :
            env(e),
            changed_choices_for_constraint(h),
            constraint(c)
        {
        }

        bool ok(const std::shared_ptr<const PackageID> & chosen_id,
                const std::shared_ptr<const ChangedChoices> & changed_choices) const
        {
            if (constraint.force_unable())
                return false;

            if (constraint.spec().if_package())
            {
                if (! match_package_with_maybe_changes(*env, *constraint.spec().if_package(),
                            changed_choices_for_constraint.get(), chosen_id, constraint.from_id(), changed_choices.get(), { }))
                    return false;
            }
            else
            {
                if (match_package_with_maybe_changes(*env, constraint.spec().if_block()->blocking(),
                            changed_choices_for_constraint.get(), chosen_id, constraint.from_id(), changed_choices.get(), { }))
                    return false;
            }

            return true;
        }

        bool visit(const ChangesToMakeDecision & decision) const
        {
            return ok(decision.origin_id(),
                    decision.if_changed_choices() ? decision.if_changed_choices()->changed_choices() : nullptr);
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            return ok(decision.existing_id(), nullptr);
        }

        bool visit(const NothingNoChangeDecision &) const
        {
            return constraint.nothing_is_fine_too();
        }

        bool visit(const UnableToMakeDecision &) const
        {
            return true;
        }

        bool visit(const RemoveDecision &) const
        {
            if (constraint.force_unable())
                return false;

            return constraint.nothing_is_fine_too();
        }

        bool visit(const BreakDecision &) const
        {
            return true;
        }
    };

    struct CheckUseExistingVisitor
    {
        const std::shared_ptr<const Constraint> constraint;

        CheckUseExistingVisitor(const std::shared_ptr<const Constraint> & c) :
            constraint(c)
        {
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            switch (constraint->use_existing())
            {
                case ue_if_possible:
                    break;

                case ue_never:
                case last_ue:
                    return false;

                case ue_only_if_transient: if (! decision.attributes()[epia_is_transient])     return false; break;
                case ue_if_same:           if (! decision.attributes()[epia_is_same])          return false; break;
                case ue_if_same_metadata:  if (! decision.attributes()[epia_is_same_metadata]) return false; break;
                case ue_if_same_version:   if (! decision.attributes()[epia_is_same_version])  return false; break;
            }

            return true;
        }

        bool visit(const NothingNoChangeDecision &) const
        {
            return true;
        }

        bool visit(const UnableToMakeDecision &) const
        {
            return true;
        }

        bool visit(const ChangesToMakeDecision &) const
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
    };

    struct GetConstraintChangedChoices
    {
        const std::shared_ptr<const ChangedChoices> visit(const TargetReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const DependentReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const PresetReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const DependencyReason & r) const
        {
            return r.from_id_changed_choices();
        }

        const std::shared_ptr<const ChangedChoices> visit(const ViaBinaryReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const WasUsedByReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const LikeOtherDestinationTypeReason &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<std::shared_ptr<const ChangedChoices> >(*this);
        }
    };

    std::shared_ptr<const ChangedChoices> get_changed_choices_for(
            const std::shared_ptr<const Constraint> & constraint)
    {
        return constraint->reason()->accept_returning<std::shared_ptr<const ChangedChoices> >(GetConstraintChangedChoices());
    }

    struct GetDecisionChangedChoices
    {
        const std::shared_ptr<const ChangedChoices> visit(const ChangesToMakeDecision & d) const
        {
            return d.if_changed_choices() ? d.if_changed_choices()->changed_choices() : nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const ExistingNoChangeDecision &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const NothingNoChangeDecision &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const UnableToMakeDecision &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const RemoveDecision &) const
        {
            return nullptr;
        }

        const std::shared_ptr<const ChangedChoices> visit(const BreakDecision &) const
        {
            return nullptr;
        }
    };

    std::shared_ptr<const ChangedChoices> get_changed_choices_for(
            const std::shared_ptr<const Decision> & decision)
    {
        return decision->accept_returning<std::shared_ptr<const ChangedChoices> >(GetDecisionChangedChoices());
    }
}

bool
Decider::_check_constraint(
        const std::shared_ptr<const Constraint> & constraint,
        const std::shared_ptr<const Decision> & decision) const
{
    if (! decision->accept_returning<bool>(CheckConstraintVisitor(_imp->env, get_changed_choices_for(constraint), *constraint)))
        return false;

    if (! decision->accept_returning<bool>(CheckUseExistingVisitor(constraint)))
        return false;

    if (! constraint->untaken())
    {
        if (! decision->taken())
            return false;
    }

    return true;
}

bool
Decider::_verify_new_constraint(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const Constraint> & constraint)
{
    return _check_constraint(constraint, resolution->decision());
}

namespace
{
    struct WrongDecisionVisitor
    {
        std::function<void ()> restart;

        WrongDecisionVisitor(const std::function<void ()> & r) :
            restart(r)
        {
        }

        void visit(const NothingNoChangeDecision &) const
        {
            /* going from nothing to something is fine */
        }

        void visit(const RemoveDecision &) const
        {
            restart();
        }

        void visit(const UnableToMakeDecision &) const
        {
            restart();
        }

        void visit(const ChangesToMakeDecision &) const
        {
            restart();
        }

        void visit(const ExistingNoChangeDecision &) const
        {
            restart();
        }

        void visit(const BreakDecision &) const PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "why are we trying to go from a BreakDecision to something else?");
        }
    };
}

void
Decider::_made_wrong_decision(
        const std::shared_ptr<Resolution> & resolution,
        const std::shared_ptr<const Constraint> & constraint)
{
    /* can we find a resolution that works for all our constraints? */
    std::shared_ptr<Resolution> adapted_resolution(std::make_shared<Resolution>(*resolution));
    adapted_resolution->constraints()->add(constraint);

    if (_imp->fns.allowed_to_restart_fn()(adapted_resolution))
    {
        const std::shared_ptr<Decision> decision(_try_to_find_decision_for(
                    adapted_resolution, _imp->fns.allow_choice_changes_fn()(resolution), false, true, false, true));
        if (decision)
        {
            resolution->decision()->accept(WrongDecisionVisitor([&] () { _suggest_restart_with(resolution, constraint, decision); }));
            resolution->decision() = decision;
        }
        else
            resolution->decision() = _cannot_decide_for(adapted_resolution);
    }
    else
        resolution->decision() = _cannot_decide_for(adapted_resolution);
}

void
Decider::_suggest_restart_with(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const Constraint> & constraint,
        const std::shared_ptr<const Decision> & decision) const
{
    throw SuggestRestart(resolution->resolvent(), resolution->decision(), constraint, decision,
            _make_constraint_for_preloading(decision, constraint));
}

const std::shared_ptr<const Constraint>
Decider::_make_constraint_for_preloading(
        const std::shared_ptr<const Decision> & decision,
        const std::shared_ptr<const Constraint> & c) const
{
    const std::shared_ptr<Constraint> result(std::make_shared<Constraint>(*c));

    const std::shared_ptr<PresetReason> reason(std::make_shared<PresetReason>("restarted because of", c->reason()));
    result->reason() = reason;

    const std::shared_ptr<const ChangedChoices> changed_choices(get_changed_choices_for(decision));

    if (result->spec().if_package())
    {
        PackageDepSpec s(_make_spec_for_preloading(*result->spec().if_package(), changed_choices));
        result->spec().if_package() = std::make_shared<PackageDepSpec>(s);
    }
    else
    {
        PackageDepSpec s(_make_spec_for_preloading(result->spec().if_block()->blocking(), changed_choices));
        result->spec().if_block() = make_shared_copy(make_uninstall_blocker(s));
    }

    return result;
}

const PackageDepSpec
Decider::_make_spec_for_preloading(const PackageDepSpec & spec,
        const std::shared_ptr<const ChangedChoices> & changed_choices) const
{
    PartiallyMadePackageDepSpec result(spec);

    /* we don't want to copy use deps from the constraint, since things like
     * [foo?] start to get weird when there's no longer an associated ID. */
    result.clear_additional_requirements();

    /* but we do want to impose our own ChangedChoices if necessary. */
    if (changed_choices)
        changed_choices->add_additional_requirements_to(result);

    return result;
}

void
Decider::_decide(const std::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(resolution->resolvent()) + "':");

    _copy_other_destination_constraints(resolution);

    std::shared_ptr<Decision> decision(_try_to_find_decision_for(
                resolution, _imp->fns.allow_choice_changes_fn()(resolution), false, true, false, true));
    if (decision)
        resolution->decision() = decision;
    else
        resolution->decision() = _cannot_decide_for(resolution);
}

void
Decider::_copy_other_destination_constraints(const std::shared_ptr<Resolution> & resolution)
{
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
    {
        if (*t == resolution->resolvent().destination_type())
            continue;

        Resolvent copy_from_resolvent(resolution->resolvent());
        copy_from_resolvent.destination_type() = *t;

        const std::shared_ptr<Resolution> copy_from_resolution(_resolution_for_resolvent(copy_from_resolvent, indeterminate));
        if (! copy_from_resolution)
            continue;

        for (const auto & copy : *copy_from_resolution->constraints())
        {
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_other_destination(resolution, copy_from_resolution, copy));
            for (const auto & constraint : *constraints)
                _apply_resolution_constraint(resolution, constraint);
        }
    }
}

namespace
{
    struct DependenciesNecessityVisitor
    {
        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const NothingNoChangeDecision &) const
        {
            return std::make_pair(nullptr, nullptr);
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const RemoveDecision &) const
        {
            return std::make_pair(nullptr, nullptr);
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const UnableToMakeDecision &) const
        {
            return std::make_pair(nullptr, nullptr);
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const ExistingNoChangeDecision & decision) const
        {
            if (decision.taken())
                return std::make_pair(decision.existing_id(), nullptr);
            else
                return std::make_pair(nullptr, nullptr);
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const ChangesToMakeDecision & decision) const
        {
            if (decision.taken())
                return std::make_pair(decision.origin_id(),
                        decision.if_changed_choices() ? decision.if_changed_choices()->changed_choices() : nullptr);
            else
                return std::make_pair(nullptr, nullptr);
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const BreakDecision &) const
        {
            return std::make_pair(nullptr, nullptr);
        }
    };
}

void
Decider::_add_dependencies_if_necessary(
        const std::shared_ptr<Resolution> & our_resolution)
{
    std::shared_ptr<const PackageID> package_id;
    std::shared_ptr<const ChangedChoices> changed_choices;
    std::tie(package_id, changed_choices) = our_resolution->decision()->accept_returning<
        std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > >(DependenciesNecessityVisitor());

    if (! package_id)
        return;

    Context context("When adding dependencies for '" + stringify(our_resolution->resolvent()) + "' with '"
            + stringify(*package_id) + "':");

    const std::shared_ptr<SanitisedDependencies> deps(std::make_shared<SanitisedDependencies>());
    deps->populate(_imp->env, *this, our_resolution, package_id, changed_choices);

    for (const auto & dependency : *deps)
    {
        Context context_2("When handling dependency '" + stringify(dependency.spec()) + "':");

        SpecInterest interest(_imp->fns.interest_in_spec_fn()(our_resolution, package_id, dependency));

        switch (interest)
        {
            case si_ignore:
                continue;

            case si_untaken:
            case si_take:
            case last_si:
                break;
        }

        /* don't have an 'already met' initially, since already met varies between slots */
        const std::shared_ptr<DependencyReason> nearly_reason(std::make_shared<DependencyReason>(
                    package_id, changed_choices, our_resolution->resolvent(), dependency, indeterminate));

        /* empty resolvents is always ok for blockers, since blocking on things
         * that don't exist is fine */
        bool empty_is_ok(dependency.spec().if_block());
        std::shared_ptr<const Resolvents> resolvents;

        if (dependency.spec().if_package())
            std::tie(resolvents, empty_is_ok) = _get_resolvents_for(*dependency.spec().if_package(), nearly_reason);
        else
            resolvents = _get_resolvents_for_blocker(*dependency.spec().if_block(), nearly_reason);

        if ((! empty_is_ok) && resolvents->empty())
            resolvents = _get_error_resolvents_for(*dependency.spec().if_package(), nearly_reason);

        for (const auto & resolvent : *resolvents)
        {
            /* now we can find out per-resolvent whether we're really already met */
            const std::shared_ptr<DependencyReason> reason(std::make_shared<DependencyReason>(
                        package_id, changed_choices, our_resolution->resolvent(), dependency,
                        dependency.spec().if_block() ? _block_dep_spec_has_nothing_installed(*dependency.spec().if_block(), package_id, resolvent) :
                        _package_dep_spec_already_met(*dependency.spec().if_package(), package_id)));

            const std::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(resolvent, true));
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(our_resolution, dependency, reason, interest));
            for (const auto & constraint : *constraints)
                _apply_resolution_constraint(dep_resolution, constraint);
        }
    }
}

std::pair<AnyChildScore, OperatorScore>
Decider::find_any_score(
        const std::shared_ptr<const Resolution> & our_resolution,
        const std::shared_ptr<const PackageID> & our_id,
        const SanitisedDependency & dep) const
{
    Context context("When working out whether we'd like || child '" + stringify(dep.spec()) + "' because of '"
            + stringify(our_resolution->resolvent()) + "':");

    const bool is_block(dep.spec().if_block());
    const PackageDepSpec & spec(is_block ? dep.spec().if_block()->blocking() : *dep.spec().if_package());

    // note: make sure the worst_score declaration in
    // AnyDepSpecChildHandler::commit in satitised_dependencies.cc
    // matches this logic
    OperatorScore operator_bias(os_worse_than_worst);
    if (spec.version_requirements_ptr() && ! spec.version_requirements_ptr()->empty())
    {
        OperatorScore score(os_worse_than_worst);
        for (const auto & version : *spec.version_requirements_ptr())
        {
            OperatorScore local_score(os_worse_than_worst);

            switch (version.version_operator().value())
            {
                case vo_greater:
                case vo_greater_equal:
                    local_score = is_block ? os_less : os_greater_or_none;
                    break;

                case vo_equal:
                case vo_tilde:
                case vo_equal_star:
                case vo_tilde_greater:
                    local_score = os_equal;
                    break;

                case vo_less_equal:
                case vo_less:
                    local_score = is_block ? os_greater_or_none : os_less;
                    break;

                case last_vo:
                    local_score = os_less;
                    break;
            }

            if (score == os_worse_than_worst)
                score = local_score;
            else
                switch (spec.version_requirements_mode())
                {
                    case vr_and:
                        score = is_block ? std::max(score, local_score) : std::min(score, local_score);
                        break;

                    case vr_or:
                        score = is_block ? std::min(score, local_score) : std::max(score, local_score);
                        break;

                    case last_vr:
                        break;
                }
        }
        operator_bias = score;
    }
    else
    {
        /* don't bias no operator over a >= operator, so || ( >=foo-2 bar )
         * still likes foo. */
        operator_bias = is_block ? os_block_everything : os_greater_or_none;
    }

    /* explicit preferences come first */
    if (spec.package_ptr())
    {
        Tribool prefer_or_avoid(_imp->fns.prefer_or_avoid_fn()(spec, our_id));
        if (prefer_or_avoid.is_true())
            return std::make_pair(is_block ? acs_avoid : acs_prefer, operator_bias);
        else if (prefer_or_avoid.is_false())
            return std::make_pair(is_block ? acs_prefer : acs_avoid, operator_bias);
    }

    /* best: blocker that doesn't match anything */
    if (is_block)
    {
        Context sub_context("When working out whether it's acs_vacuous_blocker:");

        const std::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, our_id, { mpo_ignore_additional_requirements })
                        | filter::SupportsAction<InstallAction>() | filter::NotMasked()
                    )]);
        if (ids->empty())
            return std::make_pair(acs_vacuous_blocker, operator_bias);
    }

    const std::shared_ptr<DependencyReason> reason_unless_block(is_block ? nullptr : std::make_shared<DependencyReason>(
                our_id, nullptr, our_resolution->resolvent(), dep, _package_dep_spec_already_met(*dep.spec().if_package(), our_id)));
    const std::shared_ptr<const Resolvents> resolvents_unless_block(is_block ? nullptr :
            _get_resolvents_for(spec, reason_unless_block).first);
    std::list<std::shared_ptr<Decision> > could_install_decisions;
    if (resolvents_unless_block)
        for (const auto & resolvent : *resolvents_unless_block)
        {
            const std::shared_ptr<Resolution> could_install_resolution(_create_resolution_for_resolvent(resolvent));
            const std::shared_ptr<ConstraintSequence> could_install_constraints(_make_constraints_from_dependency(
                        our_resolution, dep, reason_unless_block, si_take));
            for (const auto & constraint : *could_install_constraints)
                could_install_resolution->constraints()->add(constraint);
            auto could_install_decision(_try_to_find_decision_for(could_install_resolution, false, false, false, false, false));
            if (could_install_decision)
                could_install_decisions.push_back(could_install_decision);
        }

    /* next: could install, and something similar is installed */
    if (! is_block)
    {
        static_assert(acs_already_installed < acs_could_install_and_installedish, "acs order changed");

        Context sub_context("When working out whether it's acs_could_install_and_installedish:");

        for (const auto & decision : could_install_decisions)
        {
            const auto installed_resolvent((*_imp->env)[selection::SomeArbitraryVersion(
                        generator::Package(decision->resolvent().package()) |
                        make_slot_filter(decision->resolvent()) |
                        filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
            if (! installed_resolvent->empty())
                return std::make_pair(acs_could_install_and_installedish, operator_bias);
        }
    }

    /* next: already installed */
    static_assert(acs_already_installed < acs_vacuous_blocker, "acs order changed");
    {
        Context sub_context("When working out whether it's acs_already_installed:");

        const std::shared_ptr<const PackageIDSequence> installed_id((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, our_id, { }) |
                    filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);

        if (! installed_id->empty() ^ is_block)
            return std::make_pair(acs_already_installed, operator_bias);
    }

    /* next: could install */
    if (! is_block)
    {
        static_assert(acs_could_install < acs_already_installed, "acs order changed");
        if (! could_install_decisions.empty())
            return std::make_pair(acs_could_install, operator_bias);
    }

    /* next: blocks installed package */
    static_assert(acs_blocks_installed < acs_could_install, "acs order changed");
    if (is_block)
    {
        Context sub_context("When working out whether it's acs_blocks_installed:");

        const std::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, our_id, { }) |
                    filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
        if (! installed_ids->empty())
            return std::make_pair(acs_blocks_installed, operator_bias);
    }

    static_assert(acs_not_installable < acs_could_install, "acs order changed");
    return std::make_pair(acs_not_installable, operator_bias);
}

namespace
{
    struct SlotNameFinder
    {
        std::shared_ptr<SlotName> visit(const SlotExactPartialRequirement & s)
        {
            return std::make_shared<SlotName>(s.slot());
        }

        std::shared_ptr<SlotName> visit(const SlotAnyPartialLockedRequirement & s)
        {
            return std::make_shared<SlotName>(s.slot());
        }

        std::shared_ptr<SlotName> visit(const SlotExactFullRequirement & s)
        {
            return std::make_shared<SlotName>(s.slots().first);
        }

        std::shared_ptr<SlotName> visit(const SlotAnyUnlockedRequirement &)
        {
            return nullptr;
        }

        std::shared_ptr<SlotName> visit(const SlotAnyAtAllLockedRequirement &)
        {
            return nullptr;
        }

        std::shared_ptr<SlotName> visit(const SlotUnknownRewrittenRequirement &) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Should not be finding resolvents matching a SlotUnknownRewrittenRequirement");
        }
    };
}

const std::shared_ptr<const Resolvents>
Decider::_get_resolvents_for_blocker(const BlockDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::shared_ptr<SlotName> exact_slot;
    if (spec.blocking().slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.blocking().slot_requirement_ptr()->accept_returning<std::shared_ptr<SlotName> >(f);
    }

    DestinationTypes destination_types(_imp->fns.get_destination_types_for_blocker_fn()(spec, reason));
    std::shared_ptr<Resolvents> result(std::make_shared<Resolvents>());
    if (exact_slot)
    {
        for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
            if (destination_types[*t])
                result->push_back(Resolvent(spec.blocking(), *exact_slot, *t));
    }
    else
    {
        const std::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionInEachSlot(
                    generator::Package(*spec.blocking().package_ptr())
                    )]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
                if (destination_types[*t])
                    result->push_back(Resolvent(*i, *t));
    }

    return result;
}

const std::pair<std::shared_ptr<const Resolvents>, bool>
Decider::_get_resolvents_for(
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::shared_ptr<SlotName> exact_slot;

    if (spec.slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.slot_requirement_ptr()->accept_returning<std::shared_ptr<SlotName> >(f);
    }

    return _imp->fns.get_resolvents_for_fn()(spec, maybe_from_package_id_from_reason(reason), exact_slot, reason);
}

const std::shared_ptr<const Resolvents>
Decider::_get_error_resolvents_for(
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "', which can't be found the normal way:");

    std::shared_ptr<Resolvents> result(std::make_shared<Resolvents>());
    DestinationTypes destination_types(_imp->fns.get_destination_types_for_error_fn()(spec, reason));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        if (destination_types[*t])
        {
            Resolvent resolvent(spec, make_named_values<SlotNameOrNull>(
                        n::name_or_null() = nullptr,
                        n::null_means_unknown() = true
                        ),
                    *t);

            auto ids(_find_installable_id_candidates_for(*spec.package_ptr(), filter::All(), filter::All(), true, true));
            if (! ids->empty())
                resolvent.slot() = make_named_values<SlotNameOrNull>(
                        n::name_or_null() = (*ids->rbegin())->slot_key() ?
                            make_shared_copy((*ids->rbegin())->slot_key()->parse_value().parallel_value()) :
                            nullptr,
                        n::null_means_unknown() = true
                        );

            result->push_back(resolvent);
        }
    return result;
}

namespace
{
    ExistingPackageIDAttributes existing_package_id_attributes_for_no_installable_id(bool is_transient)
    {
        return ExistingPackageIDAttributes({ epia_is_same, epia_is_same_metadata, epia_is_same_version }) |
            (is_transient ? ExistingPackageIDAttributes({ epia_is_transient }) : ExistingPackageIDAttributes({ }));
    }
}

const std::shared_ptr<Decision>
Decider::_try_to_find_decision_for(
        const std::shared_ptr<const Resolution> & resolution,
        const bool also_try_option_changes,
        const bool try_option_changes_this_time,
        const bool also_try_masked,
        const bool try_masked_this_time,
        const bool try_removes_if_allowed) const
{
    if (resolution->constraints()->any_force_unable())
        return nullptr;

    const std::shared_ptr<const PackageID> existing_id(_find_existing_id_for(resolution));

    std::shared_ptr<const PackageID> installable_id;
    std::shared_ptr<const WhyChangedChoices> changed_choices;
    bool best;
    std::tie(installable_id, changed_choices, best) = _find_installable_id_for(resolution, try_option_changes_this_time, try_masked_this_time);

    if (resolution->constraints()->nothing_is_fine_too())
    {
        const std::shared_ptr<const PackageIDSequence> existing_resolvent_ids(_installed_ids(resolution));
        if (existing_resolvent_ids->empty())
        {
            /* nothing existing, but nothing's ok */
            return std::make_shared<NothingNoChangeDecision>(
                        resolution->resolvent(),
                        ! resolution->constraints()->all_untaken()
                        );
        }
    }

    if (installable_id && ! existing_id)
    {
        /* there's nothing suitable existing. */
        return std::make_shared<ChangesToMakeDecision>(
                    resolution->resolvent(),
                    installable_id,
                    changed_choices,
                    best,
                    last_ct,
                    ! resolution->constraints()->all_untaken(),
                    nullptr,
                    std::bind(&Decider::_fixup_changes_to_make_decision, this, resolution, std::placeholders::_1)
                    );
    }
    else if (existing_id && ! installable_id)
    {
        /* there's nothing installable. this may or may not be ok. */
        bool is_transient(has_behaviour(existing_id, "transient"));

        switch (resolution->constraints()->strictest_use_existing())
        {
            case ue_if_possible:
                break;

            case ue_only_if_transient:
            case ue_if_same_metadata:
            case ue_if_same:
            case ue_if_same_version:
                if (! is_transient)
                    return nullptr;
                break;

            case ue_never:
                return nullptr;

            case last_ue:
                break;
        }

        return std::make_shared<ExistingNoChangeDecision>(
                    resolution->resolvent(),
                    existing_id,
                    existing_package_id_attributes_for_no_installable_id(is_transient),
                    ! resolution->constraints()->all_untaken()
                    );
    }
    else if ((! existing_id) && (! installable_id))
    {
        /* we can't stick with our existing id, if there is one, and we can't
         * fix it by installing things. this might be an error, or we might be
         * able to remove things. */
        if (try_removes_if_allowed && resolution->constraints()->nothing_is_fine_too() && _installed_but_allowed_to_remove(resolution, false))
            return std::make_shared<RemoveDecision>(
                        resolution->resolvent(),
                        _installed_ids(resolution),
                        ! resolution->constraints()->all_untaken()
                        );
        else if (try_removes_if_allowed && resolution->constraints()->nothing_is_fine_too() && _installed_but_allowed_to_remove(resolution, true)) {
            auto result = std::make_shared<RemoveDecision>(
                        resolution->resolvent(),
                        _installed_ids(resolution),
                        ! resolution->constraints()->all_untaken()
                        );
            result->add_required_confirmation(std::make_shared<UninstallConfirmation>());
            return result;
        }
        else if (also_try_option_changes && ! try_option_changes_this_time)
            return _try_to_find_decision_for(resolution, true, true, also_try_masked, try_masked_this_time, try_removes_if_allowed);
        else if (also_try_masked && ! try_masked_this_time)
            return _try_to_find_decision_for(resolution, also_try_option_changes, try_option_changes_this_time, true, true, try_removes_if_allowed);
        else
            return nullptr;
    }
    else if (existing_id && installable_id)
    {
        ExistingPackageIDAttributes existing_package_id_attributes(get_sameness(existing_id, installable_id));
        if (has_behaviour(existing_id, "transient"))
            existing_package_id_attributes += epia_is_transient;

        /* we've got existing and installable. do we have any reason not to pick the existing id? */
        const std::shared_ptr<Decision> existing(std::make_shared<ExistingNoChangeDecision>(
                    resolution->resolvent(),
                    existing_id,
                    existing_package_id_attributes,
                    ! resolution->constraints()->all_untaken()
                    ));
        const std::shared_ptr<Decision> changes_to_make(std::make_shared<ChangesToMakeDecision>(
                    resolution->resolvent(),
                    installable_id,
                    changed_choices,
                    best,
                    last_ct,
                    ! resolution->constraints()->all_untaken(),
                    nullptr,
                    std::bind(&Decider::_fixup_changes_to_make_decision, this, resolution, std::placeholders::_1)
                    ));

        switch (resolution->constraints()->strictest_use_existing())
        {
            case ue_only_if_transient: return changes_to_make;
            case ue_never:             return changes_to_make;
            case ue_if_same_metadata:  return existing_package_id_attributes[epia_is_same_metadata] ? existing : changes_to_make;
            case ue_if_same:           return existing_package_id_attributes[epia_is_same]          ? existing : changes_to_make;
            case ue_if_same_version:   return existing_package_id_attributes[epia_is_same_version]  ? existing : changes_to_make;
            case ue_if_possible:       return existing;

            case last_ue:
                break;
        }
    }

    throw InternalError(PALUDIS_HERE, "resolver bug: shouldn't be reached");
}

const std::shared_ptr<Decision>
Decider::_cannot_decide_for(
        const std::shared_ptr<const Resolution> & resolution) const
{
    const std::shared_ptr<UnsuitableCandidates> unsuitable_candidates(std::make_shared<UnsuitableCandidates>());

    const std::shared_ptr<const PackageID> existing_id(_find_existing_id_for(resolution));
    if (existing_id)
        unsuitable_candidates->push_back(_make_unsuitable_candidate(resolution, existing_id, true));

    const std::shared_ptr<const PackageIDSequence> installable_ids(_find_installable_id_candidates_for(
                resolution->resolvent().package(),
                make_slot_filter(resolution->resolvent()),
                make_destination_type_filter(resolution->resolvent().destination_type()),
                true, false));
    for (const auto & package : *installable_ids)
        unsuitable_candidates->push_back(_make_unsuitable_candidate(resolution, package, false));

    return std::make_shared<UnableToMakeDecision>(
                resolution->resolvent(),
                unsuitable_candidates,
                ! resolution->constraints()->all_untaken()
                );
}

UnsuitableCandidate
Decider::_make_unsuitable_candidate(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const bool existing) const
{
    return make_named_values<UnsuitableCandidate>(
            n::package_id() = id,
            n::unmet_constraints() = _get_unmatching_constraints(resolution, id, existing)
            );
}

const std::shared_ptr<const PackageID>
Decider::_find_existing_id_for(const std::shared_ptr<const Resolution> & resolution) const
{
    const std::shared_ptr<const PackageIDSequence> ids(_installed_ids(resolution));
    return std::get<0>(_find_id_for_from(resolution, ids, false, false));
}

bool
Decider::_installed_but_allowed_to_remove(const std::shared_ptr<const Resolution> & resolution,
        const bool with_confirmation) const
{
    const std::shared_ptr<const PackageIDSequence> ids(_installed_ids(resolution));
    if (ids->empty())
        return false;

    return ids->end() == std::find_if(ids->begin(), ids->end(),
            std::bind(std::logical_not<bool>(), std::bind(&Decider::_allowed_to_remove,
                    this, resolution, std::placeholders::_1, with_confirmation)));
}

bool
Decider::_allowed_to_remove(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const bool with_confirmation) const
{
    if (! id->supports_action(SupportsActionTest<UninstallAction>()))
        return false;

    if (with_confirmation) {
        bool all = true;
        bool any = false;
        for (auto & c : *resolution->constraints()) {
            any = true;
            if (! c->reason()->make_accept_returning(
                        [&] (const TargetReason &)                   { return false; },
                        [&] (const DependencyReason & r)             {
                            auto spec = r.sanitised_dependency().spec().if_block();
                            if (spec) {
                                auto annotations = spec->maybe_annotations();
                                if (annotations) {
                                    for (auto & a : *annotations) {
                                        switch (a.role()) {
                                            case dsar_blocker_uninstall_blocked_before: return true;
                                            case dsar_blocker_uninstall_blocked_after:  return true;
                                            default:                                    break;
                                        }
                                    }
                                }
                            }
                            return false;
                        },
                        [&] (const DependentReason &)                { return false; },
                        [&] (const WasUsedByReason &)                { return false; },
                        [&] (const PresetReason &)                   { return false; },
                        [&] (const SetReason &)                      { return false; },
                        [&] (const LikeOtherDestinationTypeReason &) { return false; },
                        [&] (const ViaBinaryReason &)                { return false; }
                        )) {
                all = false;
                break;
            }
        }

        return any && all;
    }
    else
        return _imp->fns.allowed_to_remove_fn()(resolution, id);
}

const std::shared_ptr<const PackageIDSequence>
Decider::_installed_ids(const std::shared_ptr<const Resolution> & resolution) const
{
    Context context("When finding installed IDs for '" + stringify(resolution->resolvent()) + "':");

    return (*_imp->env)[selection::AllVersionsSorted(_imp->fns.make_destination_filtered_generator_fn()(generator::Package(resolution->resolvent().package()), resolution) |
                                                     make_slot_filter(resolution->resolvent()))];
}

const std::shared_ptr<const PackageIDSequence>
Decider::_find_installable_id_candidates_for(
        const QualifiedPackageName & package,
        const Filter & slot_filter,
        const Filter & destination_type_filter,
        const bool include_errors,
        const bool include_unmaskable) const
{
    Context context("When finding installable ID candidates for '" + stringify(package) + "':");

    return _imp->fns.remove_hidden_fn()(
            (*_imp->env)[_imp->fns.promote_binaries_fn()(
                _imp->fns.make_origin_filtered_generator_fn()(generator::Package(package)) |
                slot_filter |
                destination_type_filter |
                filter::SupportsAction<InstallAction>() |
                (include_errors ? filter::All() : include_unmaskable ? _imp->fns.make_unmaskable_filter_fn()(package) : filter::NotMasked())
                )]);
}

const Decider::FoundID
Decider::_find_installable_id_for(const std::shared_ptr<const Resolution> & resolution,
        const bool include_option_changes,
        const bool include_unmaskable) const
{
    return _find_id_for_from(resolution, _find_installable_id_candidates_for(
                resolution->resolvent().package(),
                make_slot_filter(resolution->resolvent()),
                make_destination_type_filter(resolution->resolvent().destination_type()),
                false, include_unmaskable),
            include_option_changes, false);
}

const Decider::FoundID
Decider::_find_id_for_from(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageIDSequence> & ids,
        const bool try_changing_choices,
        const bool trying_changing_choices) const
{
    MatchPackageOptions opts;
    if (trying_changing_choices)
        opts += mpo_ignore_additional_requirements;

    std::shared_ptr<const PackageID> best_version;
    for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
            i != i_end ; ++i)
    {
        if (! best_version)
            best_version = *i;

        bool ok(true);
        for (const auto & constraint : *resolution->constraints())
        {
            if (constraint->spec().if_package())
                ok = ok && match_package(*_imp->env, *constraint->spec().if_package(), *i, constraint->from_id(), opts);
            else
                ok = ok && ! match_package(*_imp->env, constraint->spec().if_block()->blocking(), *i, constraint->from_id(), opts);

            if (! ok)
                break;
        }

        if (ok)
        {
            auto why_changed_choices(std::make_shared<WhyChangedChoices>(WhyChangedChoices(make_named_values<WhyChangedChoices>(
                                n::changed_choices() = std::make_shared<ChangedChoices>(),
                                n::reasons() = std::make_shared<Reasons>()
                                ))));
            if (trying_changing_choices)
            {
                for (const auto & constraint : *resolution->constraints())
                {
                    if (! ok)
                        break;

                    if (! constraint->spec().if_package())
                    {
                        if (constraint->spec().if_block()->blocking().additional_requirements_ptr() &&
                                ! constraint->spec().if_block()->blocking().additional_requirements_ptr()->empty())
                        {
                            /* too complicated for now */
                            ok = false;
                        }
                        break;
                    }

                    if (! constraint->spec().if_package()->additional_requirements_ptr())
                    {
                        /* no additional requirements, so no tinkering required */
                        continue;
                    }

                    for (const auto & requirement : *constraint->spec().if_package()->additional_requirements_ptr())
                    {
                        auto b(requirement->accumulate_changes_to_make_met(_imp->env,
                                    get_changed_choices_for(constraint).get(), *i, constraint->from_id(),
                                    *why_changed_choices->changed_choices()));
                        if (b.is_false())
                        {
                            ok = false;
                            break;
                        }
                        else if (b.is_true())
                            why_changed_choices->reasons()->push_back(constraint->reason());
                    }
                }
            }

            /* might have an early requirement of [x], and a later [-x], and
             * chosen to change because of the latter */
            for (const auto & constraint : *resolution->constraints())
            {
                if (! ok)
                    break;

                if (constraint->spec().if_package())
                    ok = ok && match_package_with_maybe_changes(*_imp->env, *constraint->spec().if_package(),
                            get_changed_choices_for(constraint).get(), *i, constraint->from_id(), why_changed_choices->changed_choices().get(), { });
                else
                    ok = ok && ! match_package_with_maybe_changes(*_imp->env, constraint->spec().if_block()->blocking(),
                            get_changed_choices_for(constraint).get(), *i, constraint->from_id(), why_changed_choices->changed_choices().get(), { });
            }

            if (ok)
                return FoundID(*i,
                        why_changed_choices->changed_choices()->empty() ? nullptr : why_changed_choices,
                        (*i)->version() == best_version->version());
        }
    }

    if (try_changing_choices && ! trying_changing_choices)
        return _find_id_for_from(resolution, ids, true, true);
    else
        return FoundID(nullptr, nullptr, false);
}

const std::shared_ptr<const Constraints>
Decider::_get_unmatching_constraints(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const bool existing) const
{
    const std::shared_ptr<Constraints> result(std::make_shared<Constraints>());

    for (const auto & constraint : *resolution->constraints())
    {
        std::shared_ptr<Decision> decision;

        if (existing)
        {
            bool is_transient(has_behaviour(id, "transient"));
            decision = std::make_shared<ExistingNoChangeDecision>(
                        resolution->resolvent(),
                        id,
                        existing_package_id_attributes_for_no_installable_id(is_transient),
                        ! constraint->untaken()
                        );
        }
        else
            decision = std::make_shared<ChangesToMakeDecision>(
                        resolution->resolvent(),
                        id,
                        nullptr,
                        false,
                        last_ct,
                        ! constraint->untaken(),
                        nullptr,
                        std::function<void (const ChangesToMakeDecision &)>()
                        );
        if (! _check_constraint(constraint, decision))
            result->add(constraint);
    }

    return result;
}

void
Decider::add_target_with_reason(const PackageOrBlockDepSpec & spec, const std::shared_ptr<const Reason> & reason)
{
    Context context("When adding target '" + stringify(spec) + "':");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    /* empty resolvents is always ok for blockers, since blocking on things
     * that don't exist is fine */
    bool empty_is_ok(spec.if_block());
    std::shared_ptr<const Resolvents> resolvents;

    if (spec.if_package())
        std::tie(resolvents, empty_is_ok) = _get_resolvents_for(*spec.if_package(), reason);
    else
        resolvents = _get_resolvents_for_blocker(*spec.if_block(), reason);

    if ((! empty_is_ok) && resolvents->empty())
        resolvents = _get_error_resolvents_for(*spec.if_package(), reason);

    for (const auto & resolvent : *resolvents)
    {
        Context context_2("When adding constraints from target '" + stringify(spec) + "' to resolvent '"
                + stringify(resolvent) + "':");

        const std::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(resolvent, true));
        const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_target(dep_resolution, spec, reason));
        for (const auto & constraint : *constraints)
            _apply_resolution_constraint(dep_resolution, constraint);
    }
}

void
Decider::purge()
{
    Context context("When purging everything:");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Collecting Unused"));

    const std::shared_ptr<const PackageIDSet> have_now(collect_installed(_imp->env));
    const std::shared_ptr<PackageIDSequence> have_now_seq(std::make_shared<PackageIDSequence>());
    std::copy(have_now->begin(), have_now->end(), have_now_seq->back_inserter());

    auto unused(collect_purges(_imp->env, have_now, have_now_seq,
                std::bind(&Environment::trigger_notifier_callback, _imp->env, NotifierCallbackResolverStepEvent())));

    for (const auto & package : *unused)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (has_behaviour(package, "used") || ! package->supports_action(SupportsActionTest<UninstallAction>()))
            continue;

        Resolvent resolvent(package, dt_install_to_slash);
        const std::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        auto used_to_use(std::make_shared<ChangeByResolventSequence>());
        {
            auto i_seq(std::make_shared<PackageIDSequence>());
            i_seq->push_back(package);
            for (const auto & id : *unused)
                if (id->supports_action(SupportsActionTest<UninstallAction>()) &&
                        ! collect_depped_upon(_imp->env, id, i_seq, have_now_seq)->empty())
                    used_to_use->push_back(make_named_values<ChangeByResolvent>(
                                n::package_id() = id,
                                n::resolvent() = Resolvent(id, dt_install_to_slash)
                                ));
        }

        const std::shared_ptr<const ConstraintSequence> constraints(_imp->fns.get_constraints_for_purge_fn()(resolution, package, used_to_use));
        for (const auto & constraint : *constraints)
            _apply_resolution_constraint(resolution, constraint);

        _decide(resolution);
    }
}

void
Decider::resolve()
{
    while (true)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Deciding"));
        _resolve_decide_with_dependencies();

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Vialating"));
        if (_resolve_vias())
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Finding Dependents"));
        if (_resolve_dependents())
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Finding Purgeables"));
        if (_resolve_purges())
            continue;

        break;
    }

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Confirming"));
    _resolve_confirmations();
}

bool
Decider::_package_dep_spec_already_met(const PackageDepSpec & spec, const std::shared_ptr<const PackageID> & from_id) const
{
    Context context("When determining already met for '" + stringify(spec) + "':");

    const std::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::AllVersionsUnsorted(
                generator::Matches(spec, from_id, { }) |
                filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
    if (installed_ids->empty())
        return false;
    else
    {
        if (installed_ids->end() == std::find_if(installed_ids->begin(), installed_ids->end(),
                    _imp->fns.can_use_fn()))
            return false;

        return true;
    }
}

bool
Decider::_block_dep_spec_has_nothing_installed(const BlockDepSpec & spec, const std::shared_ptr<const PackageID> & from_id,
        const Resolvent & resolvent) const
{
    Context context("When determining already met for '" + stringify(spec) + "':");

    const std::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::SomeArbitraryVersion(
                generator::Matches(spec.blocking(), from_id, { }) |
                make_slot_filter(resolvent) |
                filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
    return installed_ids->empty();
}

namespace
{
    struct ConfirmVisitor
    {
        const Environment * const env;
        const ResolverFunctions & fns;
        const std::shared_ptr<const Resolution> resolution;

        ConfirmVisitor(
                const Environment * const e,
                const ResolverFunctions & f,
                const std::shared_ptr<const Resolution> & r) :
            env(e),
            fns(f),
            resolution(r)
        {
        }

        void visit(ChangesToMakeDecision & changes_to_make_decision) const
        {
            if (! changes_to_make_decision.best())
            {
                const std::shared_ptr<RequiredConfirmation> c(std::make_shared<NotBestConfirmation>());
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }

            if (ct_downgrade == changes_to_make_decision.change_type())
            {
                const std::shared_ptr<DowngradeConfirmation> c(std::make_shared<DowngradeConfirmation>());
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }

            if (changes_to_make_decision.origin_id()->masked())
            {
                auto c(std::make_shared<MaskedConfirmation>());
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }

            if (changes_to_make_decision.if_changed_choices())
            {
                auto c(std::make_shared<ChangedChoicesConfirmation>());
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }
        }

        void visit(BreakDecision & break_decision) const
        {
            const std::shared_ptr<BreakConfirmation> c(std::make_shared<BreakConfirmation>());
            if (! fns.confirm_fn()(resolution, c))
                break_decision.add_required_confirmation(c);
        }

        void visit(UnableToMakeDecision &) const
        {
        }

        void visit(ExistingNoChangeDecision &) const
        {
        }

        void visit(NothingNoChangeDecision &) const
        {
        }

        void visit(RemoveDecision & remove_decision) const
        {
            /* we do BreakConfirmation elsewhere */
            bool is_system(false);
            for (const auto & package : *remove_decision.ids())
                if (match_package_in_set(*env, *env->set(SetName("system")), package, { }))
                    is_system = true;

            if (is_system)
            {
                const std::shared_ptr<RemoveSystemPackageConfirmation> c(std::make_shared<RemoveSystemPackageConfirmation>());
                if (! fns.confirm_fn()(resolution, c))
                    remove_decision.add_required_confirmation(c);
            }
        }
    };
}

void
Decider::_confirm(
        const std::shared_ptr<const Resolution> & resolution)
{
    resolution->decision()->accept(ConfirmVisitor(_imp->env, _imp->fns, resolution));
}

bool
Decider::_resolve_purges()
{
    Context context("When finding things to purge:");

    const std::pair<
        std::shared_ptr<const ChangeByResolventSequence>,
        std::shared_ptr<const ChangeByResolventSequence> > going_away_newly_available(_collect_changing());

    const std::shared_ptr<PackageIDSet> going_away(std::make_shared<PackageIDSet>());
    std::transform(going_away_newly_available.first->begin(), going_away_newly_available.first->end(),
            going_away->inserter(), get_change_by_resolvent_id);

    const std::shared_ptr<PackageIDSet> newly_available(std::make_shared<PackageIDSet>());
    std::transform(going_away_newly_available.second->begin(), going_away_newly_available.second->end(),
            newly_available->inserter(), get_change_by_resolvent_id);

    const std::shared_ptr<const PackageIDSet> have_now(collect_installed(_imp->env));

    const std::shared_ptr<PackageIDSet> have_now_minus_going_away(std::make_shared<PackageIDSet>());
    std::set_difference(have_now->begin(), have_now->end(),
            going_away->begin(), going_away->end(), have_now_minus_going_away->inserter(), PackageIDSetComparator());

    const std::shared_ptr<PackageIDSet> will_eventually_have_set(std::make_shared<PackageIDSet>());
    std::copy(have_now_minus_going_away->begin(), have_now_minus_going_away->end(), will_eventually_have_set->inserter());
    std::copy(newly_available->begin(), newly_available->end(), will_eventually_have_set->inserter());

    const std::shared_ptr<PackageIDSequence> will_eventually_have(std::make_shared<PackageIDSequence>());
    std::copy(will_eventually_have_set->begin(), will_eventually_have_set->end(), will_eventually_have->back_inserter());

    const std::shared_ptr<const PackageIDSet> used_originally(accumulate_deps(_imp->env, going_away, will_eventually_have, false,
                std::bind(&Environment::trigger_notifier_callback, _imp->env, NotifierCallbackResolverStepEvent())));
    const std::shared_ptr<const PackageIDSet> used_afterwards(accumulate_deps(_imp->env, newly_available, will_eventually_have, false,
                std::bind(&Environment::trigger_notifier_callback, _imp->env, NotifierCallbackResolverStepEvent())));

    const std::shared_ptr<PackageIDSet> used_originally_and_not_going_away(std::make_shared<PackageIDSet>());
    std::set_difference(used_originally->begin(), used_originally->end(),
            going_away->begin(), going_away->end(), used_originally_and_not_going_away->inserter(), PackageIDSetComparator());

    const std::shared_ptr<PackageIDSet> newly_unused(std::make_shared<PackageIDSet>());
    std::set_difference(used_originally_and_not_going_away->begin(), used_originally_and_not_going_away->end(),
            used_afterwards->begin(), used_afterwards->end(), newly_unused->inserter(), PackageIDSetComparator());

    if (newly_unused->empty())
        return false;

    const std::shared_ptr<PackageIDSequence> newly_unused_seq(std::make_shared<PackageIDSequence>());
    std::copy(newly_unused->begin(), newly_unused->end(), newly_unused_seq->back_inserter());

    const std::shared_ptr<PackageIDSequence> have_now_minus_going_away_seq(std::make_shared<PackageIDSequence>());
    std::copy(have_now_minus_going_away->begin(), have_now_minus_going_away->end(), have_now_minus_going_away_seq->back_inserter());

    const std::shared_ptr<PackageIDSet> used_by_unchanging(std::make_shared<PackageIDSet>());
    for (const auto & package : *have_now_minus_going_away)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        const std::shared_ptr<const PackageIDSet> used(collect_depped_upon(_imp->env, package, newly_unused_seq, have_now_minus_going_away_seq));
        std::copy(used->begin(), used->end(), used_by_unchanging->inserter());
    }

    const std::shared_ptr<PackageIDSet> newly_really_unused(std::make_shared<PackageIDSet>());
    std::set_difference(newly_unused->begin(), newly_unused->end(),
            used_by_unchanging->begin(), used_by_unchanging->end(), newly_really_unused->inserter(), PackageIDSetComparator());

    const std::shared_ptr<const SetSpecTree> world(_imp->env->set(SetName("world")));

    bool changed(false);
    for (const auto & package : *newly_really_unused)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (has_behaviour(package, "used") ||
                (! package->supports_action(SupportsActionTest<UninstallAction>())))
            continue;

        /* to catch packages being purged that are also in world and not used
         * by anything else */
        if (match_package_in_set(*_imp->env, *world, package, { }))
            continue;

        const std::shared_ptr<ChangeByResolventSequence> used_to_use(std::make_shared<ChangeByResolventSequence>());
        const std::shared_ptr<PackageIDSequence> star_i_set(std::make_shared<PackageIDSequence>());
        star_i_set->push_back(package);
        for (const auto & change : *going_away_newly_available.first)
            if (change.package_id()->supports_action(SupportsActionTest<UninstallAction>()) &&
                    ! collect_depped_upon(_imp->env, change.package_id(), star_i_set, have_now_minus_going_away_seq)->empty())
                used_to_use->push_back(change);

        Resolvent resolvent(package, dt_install_to_slash);
        const std::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        const std::shared_ptr<const ConstraintSequence> constraints(_imp->fns.get_constraints_for_purge_fn()(resolution, package, used_to_use));
        for (const auto & constraint : *constraints)
            _apply_resolution_constraint(resolution, constraint);

        _decide(resolution);

        if (resolution->decision()->taken())
            changed = true;
    }

    return changed;
}

namespace
{
    struct ConstraintFromOtherDestinationVisitor
    {
        const DestinationType destination_type;
        const std::shared_ptr<const Constraint> from_constraint;
        const Resolvent resolvent;

        ConstraintFromOtherDestinationVisitor(
                const DestinationType t,
                const std::shared_ptr<const Constraint> f,
                const Resolvent & r) :
            destination_type(t),
            from_constraint(f),
            resolvent(r)
        {
        }

        const std::shared_ptr<ConstraintSequence> visit(const LikeOtherDestinationTypeReason &) const
        {
            std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());
            return result;
        }

        const std::shared_ptr<ConstraintSequence> visit(const Reason &) const
        {
            std::shared_ptr<ConstraintSequence> result(std::make_shared<ConstraintSequence>());
            result->push_back(make_shared_copy(make_named_values<Constraint>(
                            n::destination_type() = destination_type,
                            n::force_unable() = false,
                            n::from_id() = from_constraint->from_id(),
                            n::nothing_is_fine_too() = true,
                            n::reason() = std::make_shared<LikeOtherDestinationTypeReason>(
                                    resolvent,
                                    from_constraint->reason()
                                    ),
                            n::spec() = from_constraint->spec(),
                            n::untaken() = from_constraint->untaken(),
                            n::use_existing() = ue_if_possible
                            )));
            return result;
        }
    };
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_other_destination(
        const std::shared_ptr<const Resolution> & new_resolution,
        const std::shared_ptr<const Resolution> & from_resolution,
        const std::shared_ptr<const Constraint> & from_constraint) const
{
    return from_constraint->reason()->accept_returning<std::shared_ptr<ConstraintSequence> >(
            ConstraintFromOtherDestinationVisitor(new_resolution->resolvent().destination_type(),
                from_constraint, from_resolution->resolvent()));
}

