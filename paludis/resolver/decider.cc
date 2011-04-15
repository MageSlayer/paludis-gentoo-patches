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

#include <paludis/resolver/decider.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/spec_rewriter.hh>
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
#include <paludis/resolver/accumulate_deps_and_provides.hh>
#include <paludis/resolver/why_changed_choices.hh>
#include <paludis/resolver/same_slot.hh>
#include <paludis/resolver/reason_utils.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/resolver/has_behaviour-fwd.hh>

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
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>

#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/repository.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filter.hh>
#include <paludis/match_package.hh>
#include <paludis/choice.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/changed_choices.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/package_dep_spec_requirement.hh>
#include <paludis/version_operator.hh>
#include <paludis/dep_spec_data.hh>

#include <paludis/util/pimp-impl.hh>

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
        SpecRewriter rewriter;

        const std::shared_ptr<ResolutionsByResolvent> resolutions_by_resolvent;

        Imp(const Environment * const e, const ResolverFunctions & f,
                const std::shared_ptr<ResolutionsByResolvent> & l) :
            env(e),
            fns(f),
            rewriter(env),
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

Decider::~Decider()
{
}

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
        for (ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->begin()),
                i_end(_imp->resolutions_by_resolvent->end()) ;
                i != i_end ; ++i)
        {
            /* we've already decided */
            if ((*i)->decision())
                continue;

            /* we're only being suggested. don't do this on the first pass, so
             * we don't have to do restarts for suggestions later becoming hard
             * deps. */
            if (state < deciding_suggestions && (*i)->constraints()->all_untaken())
                continue;

            /* avoid deciding nothings until after we've decided things we've
             * taken, so adding extra destinations doesn't get messy. */
            if (state < deciding_nothings && (*i)->constraints()->nothing_is_fine_too())
                continue;

            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            changed = true;
            _decide(*i);

            _add_dependencies_if_necessary(*i);
        }
    }
}

bool
Decider::_resolve_vias()
{
    Context context("When finding vias:");

    bool changed(false);

    for (ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->begin()),
            i_end(_imp->resolutions_by_resolvent->end()) ;
            i != i_end ; ++i)
    {
        if ((*i)->resolvent().destination_type() == dt_create_binary)
            continue;

        if (! _imp->fns.always_via_binary_fn()(*i))
            continue;

        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        Resolvent binary_resolvent((*i)->resolvent());
        binary_resolvent.destination_type() = dt_create_binary;

        const std::shared_ptr<Resolution> binary_resolution(_resolution_for_resolvent(binary_resolvent, true));

        bool already(false);
        for (Constraints::ConstIterator c(binary_resolution->constraints()->begin()),
                c_end(binary_resolution->constraints()->end()) ;
                c != c_end ; ++c)
            if (visitor_cast<const ViaBinaryReason>(*(*c)->reason()))
            {
                already = true;
                break;
            }

        if (already)
            continue;

        changed = true;

        const std::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_via_binary(binary_resolution, *i));

        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(binary_resolution, *c);

        _decide(binary_resolution);
    }

    return changed;
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_for_via_binary(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const Resolution> & other_resolution) const
{
    return _imp->fns.get_constraints_for_via_binary_fn()(resolution, other_resolution);
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

    for (PackageIDSequence::ConstIterator s(staying->begin()), s_end(staying->end()) ;
            s != s_end ; ++s)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (! (*s)->supports_action(SupportsActionTest<UninstallAction>()))
            continue;

        const std::shared_ptr<const ChangeByResolventSequence> dependent_upon_ids(dependent_upon(
                    _imp->env, *s, changing.first, changing.second, staying));
        if (dependent_upon_ids->empty())
            continue;

        Resolvent resolvent(*s, dt_install_to_slash);
        bool remove(_remove_if_dependent(*s));

        /* we've changed things if we've not already done anything for this
         * resolvent, but only if we're going to remove it rather than mark it
         * as broken */
        if (remove && _imp->resolutions_by_resolvent->end() == _imp->resolutions_by_resolvent->find(resolvent))
            changed = true;

        const std::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));
        const std::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_dependent(
                    resolution, *s, dependent_upon_ids));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

        if ((! remove) && (! resolution->decision()))
        {
            if (! _try_to_find_decision_for(resolution, false, false, false, false, false))
                resolution->decision() = std::make_shared<BreakDecision>(
                        resolvent,
                        *s,
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

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_for_dependent(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & r) const
{
    return _imp->fns.get_constraints_for_dependent_fn()(resolution, id, r);
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
            for (PackageIDSequence::ConstIterator i(d.ids()->begin()), i_end(d.ids()->end()) ;
                    i != i_end ; ++i)
                going_away->push_back(make_named_values<ChangeByResolvent>(
                            n::package_id() = *i,
                            n::resolvent() = current_resolution->resolvent()
                            ));
        }

        void visit(const ChangesToMakeDecision & d)
        {
            for (PackageIDSequence::ConstIterator i(d.destination()->replacing()->begin()), i_end(d.destination()->replacing()->end()) ;
                    i != i_end ; ++i)
                going_away->push_back(make_named_values<ChangeByResolvent>(
                            n::package_id() = *i,
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

    for (ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->begin()),
            i_end(_imp->resolutions_by_resolvent->end()) ;
            i != i_end ; ++i)
        if ((*i)->decision() && (*i)->decision()->taken())
        {
            c.current_resolution = *i;
            (*i)->decision()->accept(c);
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
    for (PackageIDSequence::ConstIterator x(existing->begin()), x_end(existing->end()) ;
            x != x_end ; ++x)
        if (going_away->end() == std::find_if(going_away->begin(), going_away->end(), ChangeByResolventPackageIDIs(*x)))
            result->push_back(*x);

    return result;
}

void
Decider::_resolve_confirmations()
{
    Context context("When resolving confirmations:");

    for (ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->begin()),
            i_end(_imp->resolutions_by_resolvent->end()) ;
            i != i_end ; ++i)
        _confirm(*i);
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
    const std::shared_ptr<const Repository> repo(_find_repository_for(resolution, decision));
    if ((! repo->destination_interface()) ||
            (! repo->destination_interface()->is_suitable_destination_for(decision.origin_id())))
        throw InternalError(PALUDIS_HERE, stringify(repo->name()) + " is not a suitable destination for "
                + stringify(*decision.origin_id()));

    return std::make_shared<Destination>(make_named_values<Destination>(
                    n::replacing() = _find_replacing(decision.origin_id(), repo),
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
            for (auto o(others->begin()), o_end(others->end()) ;
                    o != o_end ; ++o)
                if (same_slot(*o, decision.origin_id()))
                    return ct_add_to_slot;

            return ct_slot_new;
        }
    }
    else
    {
        /* we pick the worst, so replacing 1 and 3 with 2 requires permission to
         * downgrade */
        ChangeType result(last_ct);
        for (PackageIDSequence::ConstIterator i(decision.destination()->replacing()->begin()),
                i_end(decision.destination()->replacing()->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->version() == decision.origin_id()->version())
                result = std::min(result, ct_reinstall);
            else if ((*i)->version() < decision.origin_id()->version())
                result = std::min(result, ct_upgrade);
            else if ((*i)->version() > decision.origin_id()->version())
                result = std::min(result, ct_downgrade);
        }

        return result;
    }
}

const std::shared_ptr<const Repository>
Decider::_find_repository_for(
        const std::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    return _imp->fns.find_repository_for_fn()(resolution, decision);
}

FilteredGenerator
Decider::_make_destination_filtered_generator(const Generator & g,
        const std::shared_ptr<const Resolution> & resolution) const
{
    return _imp->fns.make_destination_filtered_generator_fn()(g, resolution);
}

FilteredGenerator
Decider::_make_origin_filtered_generator(const Generator & g) const
{
    return _imp->fns.make_origin_filtered_generator_fn()(g);
}

Filter
Decider::_make_unmaskable_filter(const QualifiedPackageName & n) const
{
    return _imp->fns.make_unmaskable_filter_fn()(n);
}

bool
Decider::_allow_choice_changes_for(const std::shared_ptr<const Resolution> & resolution) const
{
    return _imp->fns.allow_choice_changes_fn()(resolution);
}

const std::shared_ptr<const PackageIDSequence>
Decider::_find_replacing(
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Repository> & repo) const
{
    return _imp->fns.find_replacing_fn()(id, repo);
}

const std::shared_ptr<Resolution>
Decider::_create_resolution_for_resolvent(const Resolvent & r) const
{
    return std::make_shared<Resolution>(make_named_values<Resolution>(
                    n::constraints() = _initial_constraints_for(r),
                    n::decision() = make_null_shared_ptr(),
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
            return make_null_shared_ptr();
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
                            n::from_id() = make_null_shared_ptr(),
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

    bool nothing_is_fine_too(true), force_unable(false);
    switch (find_blocker_role_in_annotations(spec.maybe_annotations()))
    {
        case dsar_blocker_weak:
        case dsar_blocker_strong:
        case dsar_blocker_uninstall_blocked_before:
        case dsar_blocker_uninstall_blocked_after:
            break;

        case dsar_blocker_manual:
            force_unable = ! _block_dep_spec_already_met(spec, maybe_from_package_id_from_reason(reason), resolution->resolvent());
            break;

        case dsar_blocker_upgrade_blocked_before:
            nothing_is_fine_too = ! _block_dep_spec_already_met(spec, maybe_from_package_id_from_reason(reason), resolution->resolvent());
            break;

        default:
            throw InternalError(PALUDIS_HERE, "unexpected role");
    }

    DestinationTypes destination_types(_get_destination_types_for_blocker(spec, reason));
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
                    decision.if_changed_choices() ? decision.if_changed_choices()->changed_choices() : make_null_shared_ptr());
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            return ok(decision.existing_id(), make_null_shared_ptr());
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

                case ue_only_if_transient:
                    if (! decision.is_transient())
                        return false;
                    break;

                case ue_if_same:
                    if (! decision.is_same())
                        return false;
                    break;

                case ue_if_same_version:
                    if (! decision.is_same_version())
                        return false;
                    break;

                case ue_never:
                case last_ue:
                    return false;
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
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const DependentReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const PresetReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const DependencyReason & r) const
        {
            return r.from_id_changed_choices();
        }

        const std::shared_ptr<const ChangedChoices> visit(const ViaBinaryReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const WasUsedByReason &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const LikeOtherDestinationTypeReason &) const
        {
            return make_null_shared_ptr();
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
            return d.if_changed_choices() ? d.if_changed_choices()->changed_choices() : make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const ExistingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::shared_ptr<const ChangedChoices> visit(const BreakDecision &) const
        {
            return make_null_shared_ptr();
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

    if (_allowed_to_restart(adapted_resolution))
    {
        const std::shared_ptr<Decision> decision(_try_to_find_decision_for(
                    adapted_resolution, _allow_choice_changes_for(resolution), false, true, false, true));
        if (decision)
        {
            resolution->decision()->accept(WrongDecisionVisitor(std::bind(
                            &Decider::_suggest_restart_with, this, resolution, constraint, decision)));
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

bool
Decider::_allowed_to_restart(
        const std::shared_ptr<const Resolution> & resolution) const
{
    return _imp->fns.allowed_to_restart_fn()(resolution);
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
    MutablePackageDepSpecData result(*spec.data());

    /* we don't want to copy use deps from the constraint, since things like
     * [foo?] start to get weird when there's no longer an associated ID. */
    result.unrequire_choices();

    /* but we do want to impose our own ChangedChoices if necessary. */
    if (changed_choices)
        changed_choices->add_requirements_to(result);

    return result;
}

void
Decider::_decide(const std::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(resolution->resolvent()) + "':");

    _copy_other_destination_constraints(resolution);

    std::shared_ptr<Decision> decision(_try_to_find_decision_for(
                resolution, _allow_choice_changes_for(resolution), false, true, false, true));
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

        const std::shared_ptr<Resolution> copy_from_resolution(
                _resolution_for_resolvent(copy_from_resolvent, indeterminate));
        if (! copy_from_resolution)
            continue;

        for (Constraints::ConstIterator c(copy_from_resolution->constraints()->begin()),
                c_end(copy_from_resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_other_destination(
                        resolution, copy_from_resolution, *c));
            for (ConstraintSequence::ConstIterator d(constraints->begin()), d_end(constraints->end()) ;
                    d != d_end ; ++d)
                _apply_resolution_constraint(resolution, *d);
        }
    }
}

namespace
{
    struct DependenciesNecessityVisitor
    {
        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const NothingNoChangeDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const RemoveDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const UnableToMakeDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const ExistingNoChangeDecision & decision) const
        {
            if (decision.taken())
                return std::make_pair(decision.existing_id(), make_null_shared_ptr());
            else
                return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const ChangesToMakeDecision & decision) const
        {
            if (decision.taken())
                return std::make_pair(decision.origin_id(),
                        decision.if_changed_choices() ? decision.if_changed_choices()->changed_choices() : make_null_shared_ptr());
            else
                return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
        }

        const std::pair<std::shared_ptr<const PackageID>, std::shared_ptr<const ChangedChoices> > visit(const BreakDecision &) const
        {
            return std::make_pair(make_null_shared_ptr(), make_null_shared_ptr());
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

    for (SanitisedDependencies::ConstIterator s(deps->begin()), s_end(deps->end()) ;
            s != s_end ; ++s)
    {
        Context context_2("When handling dependency '" + stringify(s->spec()) + "':");

        SpecInterest interest(_interest_in_spec(our_resolution, package_id, *s));

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
                    package_id, changed_choices, our_resolution->resolvent(), *s, indeterminate));

        /* empty resolvents is always ok for blockers, since blocking on things
         * that don't exist is fine */
        bool empty_is_ok(s->spec().if_block());
        std::shared_ptr<const Resolvents> resolvents;

        if (s->spec().if_package())
            std::tie(resolvents, empty_is_ok) = _get_resolvents_for(*s->spec().if_package(), nearly_reason);
        else
            resolvents = _get_resolvents_for_blocker(*s->spec().if_block(), nearly_reason);

        if ((! empty_is_ok) && resolvents->empty())
            resolvents = _get_error_resolvents_for(*s->spec().if_package(), nearly_reason);

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            /* now we can find out per-resolvent whether we're really already met */
            const std::shared_ptr<DependencyReason> reason(std::make_shared<DependencyReason>(
                        package_id, changed_choices, our_resolution->resolvent(), *s,
                        s->spec().if_block() ? _block_dep_spec_already_met(*s->spec().if_block(), package_id, *r) :
                        _package_dep_spec_already_met(*s->spec().if_package(), package_id)));

            const std::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(*r, true));
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(our_resolution, *s, reason, interest));

            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                _apply_resolution_constraint(dep_resolution, *c);
        }
    }
}

SpecInterest
Decider::_interest_in_spec(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const SanitisedDependency & dep) const
{
    return _imp->fns.interest_in_spec_fn()(resolution, id, dep);
}

const std::shared_ptr<Constraints>
Decider::_initial_constraints_for(const Resolvent & r) const
{
    return _imp->fns.get_initial_constraints_for_fn()(r);
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
    if (spec.all_version_requirements() && ! spec.all_version_requirements()->empty())
    {
        OperatorScore score(os_worse_than_worst);
        for (auto v(spec.all_version_requirements()->begin()), v_end(spec.all_version_requirements()->end()) ;
                v != v_end ; ++v)
        {
            OperatorScore local_score(os_worse_than_worst);

            switch ((*v)->version_operator().value())
            {
                case vo_greater:
                case vo_greater_equal:
                    local_score = is_block ? os_less : os_greater_or_none;
                    break;

                case vo_equal:
                case vo_tilde:
                case vo_nice_equal_star:
                case vo_stupid_equal_star:
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
                switch ((*v)->combiner())
                {
                    case vrc_and:
                        score = is_block ? std::max(score, local_score) : std::min(score, local_score);
                        break;

                    case vrc_or:
                        score = is_block ? std::min(score, local_score) : std::max(score, local_score);
                        break;

                    case last_vrc:
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
    if (spec.package_name_requirement())
    {
        Tribool prefer_or_avoid(_imp->fns.prefer_or_avoid_fn()(spec.package_name_requirement()->name()));
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
                    generator::Matches(spec, our_id, { mpo_ignore_choice_requirements })
                        | filter::SupportsAction<InstallAction>() | filter::NotMasked()
                    )]);
        if (ids->empty())
            return std::make_pair(acs_vacuous_blocker, operator_bias);
    }

    const std::shared_ptr<DependencyReason> reason_unless_block(is_block ? make_null_shared_ptr() : std::make_shared<DependencyReason>(
                our_id, make_null_shared_ptr(), our_resolution->resolvent(), dep, _package_dep_spec_already_met(*dep.spec().if_package(), our_id)));
    const std::shared_ptr<const Resolvents> resolvents_unless_block(_get_resolvents_for(spec, reason_unless_block).first);

    /* next: will already be installing */
    static_assert(acs_will_be_installing < acs_vacuous_blocker, "acs order changed");
    if (! is_block)
    {
        for (Resolvents::ConstIterator r(resolvents_unless_block->begin()), r_end(resolvents_unless_block->end()) ;
                r != r_end ; ++r)
        {
            bool any(false), any_bad(false);
            ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->find(*r));
            if (i != _imp->resolutions_by_resolvent->end() && (*i)->decision())
            {
                auto constraint(_make_constraint_from_package_dependency(our_resolution, dep, reason_unless_block, si_take));
                if (_check_constraint(constraint, (*i)->decision()))
                    any = true;
                else
                {
                    any_bad = false;
                    break;
                }
            }

            if (any && ! any_bad)
                return std::make_pair(acs_will_be_installing, operator_bias);
        }
    }

    /* next: already installed */
    static_assert(acs_already_installed < acs_will_be_installing, "acs order changed");
    {
        Context sub_context("When working out whether it's acs_already_installed:");

        const std::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, our_id, { }) |
                    filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
        if (! installed_ids->empty() ^ is_block)
            return std::make_pair(acs_already_installed, operator_bias);
    }

    /* next: could install */
    if (! is_block)
    {
        static_assert(acs_could_install < acs_already_installed, "acs order changed");
        for (Resolvents::ConstIterator r(resolvents_unless_block->begin()), r_end(resolvents_unless_block->end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<Resolution> resolution(_create_resolution_for_resolvent(*r));
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(
                        our_resolution, dep, reason_unless_block, si_take));
            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                resolution->constraints()->add(*c);
            const std::shared_ptr<Decision> decision(_try_to_find_decision_for(resolution, false, false, false, false, false));
            if (decision)
                return std::make_pair(acs_could_install, operator_bias);
        }
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

const std::shared_ptr<const Resolvents>
Decider::_get_resolvents_for_blocker(const BlockDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::shared_ptr<SlotName> exact_slot;
    if (spec.blocking().exact_slot_requirement())
        exact_slot = make_shared_copy(spec.blocking().exact_slot_requirement()->name());

    DestinationTypes destination_types(_get_destination_types_for_blocker(spec, reason));
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
                    generator::Package(spec.blocking().package_name_requirement()->name())
                    )]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
                if (destination_types[*t])
                    result->push_back(Resolvent(*i, *t));
    }

    return result;
}

const DestinationTypes
Decider::_get_destination_types_for_blocker(const BlockDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    return _imp->fns.get_destination_types_for_blocker_fn()(spec, reason);
}

const std::pair<std::shared_ptr<const Resolvents>, bool>
Decider::_get_resolvents_for(
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::shared_ptr<SlotName> exact_slot;

    if (spec.exact_slot_requirement())
        exact_slot = make_shared_copy(spec.exact_slot_requirement()->name());

    return _imp->fns.get_resolvents_for_fn()(spec, maybe_from_package_id_from_reason(reason), exact_slot, reason);
}

const DestinationTypes
Decider::_get_destination_types_for_error(
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    return _imp->fns.get_destination_types_for_error_fn()(spec, reason);
}

const std::shared_ptr<const Resolvents>
Decider::_get_error_resolvents_for(
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "', which can't be found the normal way:");

    std::shared_ptr<Resolvents> result(std::make_shared<Resolvents>());
    DestinationTypes destination_types(_get_destination_types_for_error(spec, reason));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        if (destination_types[*t])
        {
            Resolvent resolvent(spec, make_named_values<SlotNameOrNull>(
                        n::name_or_null() = make_null_shared_ptr(),
                        n::null_means_unknown() = true
                        ),
                    *t);

            auto ids(_find_installable_id_candidates_for(spec.package_name_requirement()->name(), filter::All(), true, true));
            if (! ids->empty())
                resolvent.slot() = make_named_values<SlotNameOrNull>(
                        n::name_or_null() = (*ids->rbegin())->slot_key() ?
                            make_shared_copy((*ids->rbegin())->slot_key()->parse_value()) :
                            make_null_shared_ptr(),
                        n::null_means_unknown() = true
                        );

            result->push_back(resolvent);
        }
    return result;
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
        return make_null_shared_ptr();

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
                    make_null_shared_ptr(),
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
            case ue_if_same:
            case ue_if_same_version:
                if (! is_transient)
                    return make_null_shared_ptr();
                break;

            case ue_never:
                return make_null_shared_ptr();

            case last_ue:
                break;
        }

        return std::make_shared<ExistingNoChangeDecision>(
                    resolution->resolvent(),
                    existing_id,
                    true,
                    true,
                    is_transient,
                    ! resolution->constraints()->all_untaken()
                    );
    }
    else if ((! existing_id) && (! installable_id))
    {
        /* we can't stick with our existing id, if there is one, and we can't
         * fix it by installing things. this might be an error, or we might be
         * able to remove things. */
        if (try_removes_if_allowed && resolution->constraints()->nothing_is_fine_too() && _installed_but_allowed_to_remove(resolution))
            return std::make_shared<RemoveDecision>(
                        resolution->resolvent(),
                        _installed_ids(resolution),
                        ! resolution->constraints()->all_untaken()
                        );
        else if (also_try_option_changes && ! try_option_changes_this_time)
            return _try_to_find_decision_for(resolution, true, true, also_try_masked, try_masked_this_time, try_removes_if_allowed);
        else if (also_try_masked && ! try_masked_this_time)
            return _try_to_find_decision_for(resolution, also_try_option_changes, try_option_changes_this_time, true, true, try_removes_if_allowed);
        else
            return make_null_shared_ptr();
    }
    else if (existing_id && installable_id)
    {
        bool is_same_version(existing_id->version() == installable_id->version());
        bool is_same(false);

        if (is_same_version)
        {
            is_same = true;

            std::set<ChoiceNameWithPrefix> common;
            std::shared_ptr<const Choices> installable_choices;
            std::shared_ptr<const Choices> existing_choices;

            if (existing_id->choices_key() && installable_id->choices_key())
            {
                installable_choices = installable_id->choices_key()->parse_value();
                existing_choices = existing_id->choices_key()->parse_value();

                std::set<ChoiceNameWithPrefix> i_common, u_common;
                for (Choices::ConstIterator k(installable_choices->begin()), k_end(installable_choices->end()) ;
                        k != k_end ; ++k)
                {
                    if (! (*k)->consider_added_or_changed())
                        continue;

                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->explicitly_listed())
                            i_common.insert((*i)->name_with_prefix());
                }

                for (Choices::ConstIterator k(existing_choices->begin()), k_end(existing_choices->end()) ;
                        k != k_end ; ++k)
                {
                    if (! (*k)->consider_added_or_changed())
                        continue;

                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->explicitly_listed())
                            u_common.insert((*i)->name_with_prefix());
                }

                std::set_intersection(
                        i_common.begin(), i_common.end(),
                        u_common.begin(), u_common.end(),
                        std::inserter(common, common.begin()));
            }

            for (std::set<ChoiceNameWithPrefix>::const_iterator f(common.begin()), f_end(common.end()) ;
                    f != f_end ; ++f)
                if (installable_choices->find_by_name_with_prefix(*f)->enabled() !=
                        existing_choices->find_by_name_with_prefix(*f)->enabled())
                {
                    is_same = false;
                    break;
                }
        }

        bool is_transient(has_behaviour(existing_id, "transient"));

        /* we've got existing and installable. do we have any reason not to pick the existing id? */
        const std::shared_ptr<Decision> existing(std::make_shared<ExistingNoChangeDecision>(
                    resolution->resolvent(),
                    existing_id,
                    is_same,
                    is_same_version,
                    is_transient,
                    ! resolution->constraints()->all_untaken()
                    ));
        const std::shared_ptr<Decision> changes_to_make(std::make_shared<ChangesToMakeDecision>(
                    resolution->resolvent(),
                    installable_id,
                    changed_choices,
                    best,
                    last_ct,
                    ! resolution->constraints()->all_untaken(),
                    make_null_shared_ptr(),
                    std::bind(&Decider::_fixup_changes_to_make_decision, this, resolution, std::placeholders::_1)
                    ));

        switch (resolution->constraints()->strictest_use_existing())
        {
            case ue_only_if_transient:
            case ue_never:
                return changes_to_make;

            case ue_if_same:
                if (is_same)
                    return existing;
                else
                    return changes_to_make;

            case ue_if_same_version:
                if (is_same_version)
                    return existing;
                else
                    return changes_to_make;

            case ue_if_possible:
                return existing;

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
                resolution->resolvent().package(), make_slot_filter(resolution->resolvent()), true, false));
    for (PackageIDSequence::ConstIterator i(installable_ids->begin()), i_end(installable_ids->end()) ;
            i != i_end ; ++i)
        unsuitable_candidates->push_back(_make_unsuitable_candidate(resolution, *i, false));

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
Decider::_installed_but_allowed_to_remove(const std::shared_ptr<const Resolution> & resolution) const
{
    const std::shared_ptr<const PackageIDSequence> ids(_installed_ids(resolution));
    if (ids->empty())
        return false;

    return ids->end() == std::find_if(ids->begin(), ids->end(),
            std::bind(std::logical_not<bool>(), std::bind(&Decider::_allowed_to_remove,
                    this, resolution, std::placeholders::_1)));
}

bool
Decider::_allowed_to_remove(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id) const
{
    return id->supports_action(SupportsActionTest<UninstallAction>()) && _imp->fns.allowed_to_remove_fn()(resolution, id);
}

bool
Decider::_remove_if_dependent(const std::shared_ptr<const PackageID> & id) const
{
    return _imp->fns.remove_if_dependent_fn()(id);
}

const std::shared_ptr<const PackageIDSequence>
Decider::_installed_ids(const std::shared_ptr<const Resolution> & resolution) const
{
    Context context("When finding installed IDs for '" + stringify(resolution->resolvent()) + "':");

    return (*_imp->env)[selection::AllVersionsSorted(
            _make_destination_filtered_generator(generator::Package(resolution->resolvent().package()), resolution) |
            make_slot_filter(resolution->resolvent())
            )];
}

const std::shared_ptr<const PackageIDSequence>
Decider::_find_installable_id_candidates_for(
        const QualifiedPackageName & package,
        const Filter & slot_filter,
        const bool include_errors,
        const bool include_unmaskable) const
{
    Context context("When finding installable ID candidates for '" + stringify(package) + "':");

    return _remove_hidden(
            (*_imp->env)[selection::AllVersionsSorted(
                _make_origin_filtered_generator(generator::Package(package)) |
                slot_filter |
                filter::SupportsAction<InstallAction>() |
                (include_errors ? filter::All() : include_unmaskable ? _make_unmaskable_filter(package) : filter::NotMasked())
                )]);
}

const std::shared_ptr<const PackageIDSequence>
Decider::_remove_hidden(const std::shared_ptr<const PackageIDSequence> & ids) const
{
    return _imp->fns.remove_hidden_fn()(ids);
}

const Decider::FoundID
Decider::_find_installable_id_for(const std::shared_ptr<const Resolution> & resolution,
        const bool include_option_changes,
        const bool include_unmaskable) const
{
    return _find_id_for_from(resolution, _find_installable_id_candidates_for(
                resolution->resolvent().package(), make_slot_filter(resolution->resolvent()), false, include_unmaskable),
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
        opts += mpo_ignore_choice_requirements;

    std::shared_ptr<const PackageID> best_version;
    for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
            i != i_end ; ++i)
    {
        if (! best_version)
            best_version = *i;

        bool ok(true);
        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            if ((*c)->spec().if_package())
                ok = ok && match_package(*_imp->env, *(*c)->spec().if_package(), *i, (*c)->from_id(), opts);
            else
                ok = ok && ! match_package(*_imp->env, (*c)->spec().if_block()->blocking(), *i, (*c)->from_id(), opts);

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
                for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                        c_end(resolution->constraints()->end()) ;
                        c != c_end ; ++c)
                {
                    if (! ok)
                        break;

                    if (! (*c)->spec().if_package())
                    {
                        if ((*c)->spec().if_block()->blocking().all_choice_requirements() &&
                                ! (*c)->spec().if_block()->blocking().all_choice_requirements()->empty())
                        {
                            /* too complicated for now */
                            ok = false;
                        }
                        break;
                    }

                    if (! (*c)->spec().if_package()->all_choice_requirements())
                    {
                        /* no additional requirements, so no tinkering required */
                        continue;
                    }

                    for (auto a((*c)->spec().if_package()->all_choice_requirements()->begin()),
                            a_end((*c)->spec().if_package()->all_choice_requirements()->end()) ;
                            a != a_end ; ++a)
                    {
                        auto b((*a)->accumulate_changes_to_make_met(_imp->env,
                                    get_changed_choices_for(*c).get(), *i, (*c)->from_id(),
                                    *why_changed_choices->changed_choices()));
                        if (b.is_false())
                        {
                            ok = false;
                            break;
                        }
                        else if (b.is_true())
                            why_changed_choices->reasons()->push_back((*c)->reason());
                    }
                }
            }

            /* might have an early requirement of [x], and a later [-x], and
             * chosen to change because of the latter */
            for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                    c_end(resolution->constraints()->end()) ;
                    c != c_end ; ++c)
            {
                if (! ok)
                    break;

                if ((*c)->spec().if_package())
                    ok = ok && match_package_with_maybe_changes(*_imp->env, *(*c)->spec().if_package(),
                            get_changed_choices_for(*c).get(), *i, (*c)->from_id(), why_changed_choices->changed_choices().get(), { });
                else
                    ok = ok && ! match_package_with_maybe_changes(*_imp->env, (*c)->spec().if_block()->blocking(),
                            get_changed_choices_for(*c).get(), *i, (*c)->from_id(), why_changed_choices->changed_choices().get(), { });
            }

            if (ok)
                return FoundID(*i,
                        why_changed_choices->changed_choices()->empty() ? make_null_shared_ptr() : why_changed_choices,
                        (*i)->version() == best_version->version());
        }
    }

    if (try_changing_choices && ! trying_changing_choices)
        return _find_id_for_from(resolution, ids, true, true);
    else
        return FoundID(make_null_shared_ptr(), make_null_shared_ptr(), false);
}

const std::shared_ptr<const Constraints>
Decider::_get_unmatching_constraints(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const bool existing) const
{
    const std::shared_ptr<Constraints> result(std::make_shared<Constraints>());

    for (Constraints::ConstIterator c(resolution->constraints()->begin()),
            c_end(resolution->constraints()->end()) ;
            c != c_end ; ++c)
    {
        std::shared_ptr<Decision> decision;

        if (existing)
        {
            bool is_transient(has_behaviour(id, "transient"));
            decision = std::make_shared<ExistingNoChangeDecision>(
                        resolution->resolvent(),
                        id,
                        true,
                        true,
                        is_transient,
                        ! (*c)->untaken()
                        );
        }
        else
            decision = std::make_shared<ChangesToMakeDecision>(
                        resolution->resolvent(),
                        id,
                        make_null_shared_ptr(),
                        false,
                        last_ct,
                        ! (*c)->untaken(),
                        make_null_shared_ptr(),
                        std::function<void (const ChangesToMakeDecision &)>()
                        );
        if (! _check_constraint(*c, decision))
            result->add(*c);
    }

    return result;
}

const std::shared_ptr<const RewrittenSpec>
Decider::rewrite_if_special(
        const PackageOrBlockDepSpec & spec,
        const std::shared_ptr<const Resolvent> & maybe_from) const
{
    return _imp->rewriter.rewrite_if_special(spec, maybe_from);
}

void
Decider::add_target_with_reason(const PackageOrBlockDepSpec & spec, const std::shared_ptr<const Reason> & reason)
{
    Context context("When adding target '" + stringify(spec) + "':");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::shared_ptr<const RewrittenSpec> if_rewritten(rewrite_if_special(spec, make_null_shared_ptr()));
    if (if_rewritten)
    {
        for (Sequence<PackageOrBlockDepSpec>::ConstIterator i(if_rewritten->specs()->begin()), i_end(if_rewritten->specs()->end()) ;
                i != i_end ; ++i)
            add_target_with_reason(*i, reason);
    }
    else
    {
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

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            Context context_2("When adding constraints from target '" + stringify(spec) + "' to resolvent '"
                    + stringify(*r) + "':");

            const std::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(*r, true));
            const std::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_target(dep_resolution, spec, reason));

            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                _apply_resolution_constraint(dep_resolution, *c);
        }
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

    for (PackageIDSet::ConstIterator i(unused->begin()), i_end(unused->end()) ;
            i != i_end ; ++i)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (has_behaviour(*i, "used") || ! (*i)->supports_action(SupportsActionTest<UninstallAction>()))
            continue;

        Resolvent resolvent(*i, dt_install_to_slash);
        const std::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        auto used_to_use(std::make_shared<ChangeByResolventSequence>());
        {
            auto i_seq(std::make_shared<PackageIDSequence>());
            i_seq->push_back(*i);
            for (auto u(unused->begin()), u_end(unused->end()) ;
                    u != u_end ; ++u)
                if ((*u)->supports_action(SupportsActionTest<UninstallAction>()) &&
                        ! collect_depped_upon(_imp->env, *u, i_seq, have_now_seq)->empty())
                    used_to_use->push_back(make_named_values<ChangeByResolvent>(
                                n::package_id() = *u,
                                n::resolvent() = Resolvent(*u, dt_install_to_slash)
                                ));
        }

        const std::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_purge(resolution, *i, used_to_use));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

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
                    std::bind(&Decider::_can_use, this, std::placeholders::_1)))
            return false;

        return true;
    }
}

bool
Decider::_block_dep_spec_already_met(const BlockDepSpec & spec, const std::shared_ptr<const PackageID> & from_id,
        const Resolvent & resolvent) const
{
    Context context("When determining already met for '" + stringify(spec) + "':");

    const std::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::SomeArbitraryVersion(
                generator::Matches(spec.blocking(), from_id, { }) |
                make_slot_filter(resolvent) |
                filter::InstalledAtRoot(_imp->env->system_root_key()->parse_value()))]);
    return installed_ids->empty();
}

bool
Decider::_can_use(
        const std::shared_ptr<const PackageID> & id) const
{
    return _imp->fns.can_use_fn()(id);
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
            for (PackageIDSequence::ConstIterator i(remove_decision.ids()->begin()), i_end(remove_decision.ids()->end()) ;
                    i != i_end && ! is_system ; ++i)
                if (match_package_in_set(*env, *env->set(SetName("system")), *i, { }))
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

    const std::shared_ptr<const PackageIDSet> used_originally(accumulate_deps_and_provides(_imp->env, going_away, will_eventually_have, false,
                std::bind(&Environment::trigger_notifier_callback, _imp->env, NotifierCallbackResolverStepEvent())));
    const std::shared_ptr<const PackageIDSet> used_afterwards(accumulate_deps_and_provides(_imp->env, newly_available, will_eventually_have, false,
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
    for (PackageIDSet::ConstIterator u(have_now_minus_going_away->begin()), u_end(have_now_minus_going_away->end()) ;
            u != u_end ; ++u)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        const std::shared_ptr<const PackageIDSet> used(collect_depped_upon(_imp->env, *u, newly_unused_seq, have_now_minus_going_away_seq));
        std::copy(used->begin(), used->end(), used_by_unchanging->inserter());
    }

    const std::shared_ptr<PackageIDSet> newly_really_unused(std::make_shared<PackageIDSet>());
    std::set_difference(newly_unused->begin(), newly_unused->end(),
            used_by_unchanging->begin(), used_by_unchanging->end(), newly_really_unused->inserter(), PackageIDSetComparator());

    const std::shared_ptr<const SetSpecTree> world(_imp->env->set(SetName("world")));

    bool changed(false);
    for (PackageIDSet::ConstIterator i(newly_really_unused->begin()), i_end(newly_really_unused->end()) ;
            i != i_end ; ++i)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (has_behaviour(*i, "used") ||
                (! (*i)->supports_action(SupportsActionTest<UninstallAction>())))
            continue;

        /* to catch packages being purged that are also in world and not used
         * by anything else */
        if (match_package_in_set(*_imp->env, *world, *i, { }))
            continue;

        const std::shared_ptr<ChangeByResolventSequence> used_to_use(std::make_shared<ChangeByResolventSequence>());
        const std::shared_ptr<PackageIDSequence> star_i_set(std::make_shared<PackageIDSequence>());
        star_i_set->push_back(*i);
        for (ChangeByResolventSequence::ConstIterator g(going_away_newly_available.first->begin()), g_end(going_away_newly_available.first->end()) ;
                g != g_end ; ++g)
            if (g->package_id()->supports_action(SupportsActionTest<UninstallAction>()) &&
                    ! collect_depped_upon(_imp->env, g->package_id(), star_i_set, have_now_minus_going_away_seq)->empty())
                used_to_use->push_back(*g);

        Resolvent resolvent(*i, dt_install_to_slash);
        const std::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        const std::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_purge(resolution, *i, used_to_use));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

        _decide(resolution);

        if (resolution->decision()->taken())
            changed = true;
    }

    return changed;
}

const std::shared_ptr<ConstraintSequence>
Decider::_make_constraints_for_purge(
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & r) const
{
    return _imp->fns.get_constraints_for_purge_fn()(resolution, id, r);
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

