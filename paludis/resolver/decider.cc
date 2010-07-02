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
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/log.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/repository.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/package_database.hh>
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
#include <paludis/dep_spec_flattener.hh>

#include <paludis/util/private_implementation_pattern-impl.hh>

#include <algorithm>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Decider>
    {
        const Environment * const env;
        const ResolverFunctions fns;
        SpecRewriter rewriter;

        const std::tr1::shared_ptr<ResolutionsByResolvent> resolutions_by_resolvent;

        Implementation(const Environment * const e, const ResolverFunctions & f,
                const std::tr1::shared_ptr<ResolutionsByResolvent> & l) :
            env(e),
            fns(f),
            rewriter(env),
            resolutions_by_resolvent(l)
        {
        }
    };
}

Decider::Decider(const Environment * const e, const ResolverFunctions & f,
        const std::tr1::shared_ptr<ResolutionsByResolvent> & l) :
    PrivateImplementationPattern<Decider>(new Implementation<Decider>(e, f, l))
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
Decider::_resolve_dependents()
{
    Context context("When finding dependents:");

    bool changed(false);
    const std::pair<
        std::tr1::shared_ptr<const ChangeByResolventSequence>,
        std::tr1::shared_ptr<const ChangeByResolventSequence> > changing(_collect_changing());

    if (changing.first->empty())
        return false;

    const std::tr1::shared_ptr<const PackageIDSequence> staying(_collect_staying(changing.first));

    for (PackageIDSequence::ConstIterator s(staying->begin()), s_end(staying->end()) ;
            s != s_end ; ++s)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        const std::tr1::shared_ptr<const ChangeByResolventSequence> dependent_upon(_dependent_upon(
                    *s, changing.first, changing.second));
        if (dependent_upon->empty())
            continue;

        Resolvent resolvent(*s, dt_install_to_slash);
        bool remove(_remove_if_dependent(*s));

        /* we've changed things if we've not already done anything for this
         * resolvent, but only if we're going to remove it rather than mark it
         * as broken */
        if (remove && _imp->resolutions_by_resolvent->end() == _imp->resolutions_by_resolvent->find(resolvent))
            changed = true;

        const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));
        const std::tr1::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_dependent(
                    resolution, *s, dependent_upon));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

        if ((! remove) && (! resolution->decision()))
            resolution->decision() = make_shared_ptr(new BreakDecision(
                        resolvent,
                        *s,
                        true));
    }

    return changed;
}

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_for_dependent(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & r) const
{
    return _imp->fns.get_constraints_for_dependent_fn()(resolution, id, r);
}

namespace
{
    const std::tr1::shared_ptr<const PackageID> dependent_checker_id(const std::tr1::shared_ptr<const PackageID> & i)
    {
        return i;
    }

    const std::tr1::shared_ptr<const PackageID> dependent_checker_id(const ChangeByResolvent & i)
    {
        return i.package_id();
    }

    template <typename C_>
    struct DependentChecker
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const C_> going_away;
        const std::tr1::shared_ptr<const C_> newly_available;
        const std::tr1::shared_ptr<C_> result;

        DependentChecker(
                const Environment * const e,
                const std::tr1::shared_ptr<const C_> & g,
                const std::tr1::shared_ptr<const C_> & n) :
            env(e),
            going_away(g),
            newly_available(n),
            result(new C_)
        {
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & s)
        {
            const std::tr1::shared_ptr<const SetSpecTree> set(env->set(s.spec()->name()));
            set->root()->accept(*this);
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & s)
        {
            for (typename C_::ConstIterator g(going_away->begin()), g_end(going_away->end()) ;
                    g != g_end ; ++g)
            {
                if (! match_package(*env, *s.spec(), *dependent_checker_id(*g), MatchPackageOptions()))
                    continue;

                bool any(false);
                for (typename C_::ConstIterator n(newly_available->begin()), n_end(newly_available->end()) ;
                        n != n_end ; ++n)
                {
                    if (match_package(*env, *s.spec(), *dependent_checker_id(*n), MatchPackageOptions()))
                    {
                        any = true;
                        break;
                    }
                }

                if (! any)
                    result->push_back(*g);
            }
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & s)
        {
            if (s.spec()->condition_met())
                std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()),
                        accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & s)
        {
            std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & s)
        {
            std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()),
                    accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }
    };

    const std::tr1::shared_ptr<const PackageID> get_change_by_resolvent_id(const ChangeByResolvent & r)
    {
        return r.package_id();
    }
}

const std::tr1::shared_ptr<const ChangeByResolventSequence>
Decider::_dependent_upon(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & going_away,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & staying) const
{
    DependentChecker<ChangeByResolventSequence> c(_imp->env, going_away, staying);
    if (id->dependencies_key())
        id->dependencies_key()->value()->root()->accept(c);
    else
    {
        if (id->build_dependencies_key())
            id->build_dependencies_key()->value()->root()->accept(c);
        if (id->run_dependencies_key())
            id->run_dependencies_key()->value()->root()->accept(c);
        if (id->post_dependencies_key())
            id->post_dependencies_key()->value()->root()->accept(c);
        if (id->suggested_dependencies_key())
            id->suggested_dependencies_key()->value()->root()->accept(c);
    }

    return c.result;
}

namespace
{
    struct ChangingCollector
    {
        std::tr1::shared_ptr<Resolution> current_resolution;

        std::tr1::shared_ptr<ChangeByResolventSequence> going_away;
        std::tr1::shared_ptr<ChangeByResolventSequence> newly_available;

        ChangingCollector() :
            going_away(new ChangeByResolventSequence),
            newly_available(new ChangeByResolventSequence)
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
    std::tr1::shared_ptr<const ChangeByResolventSequence>,
    std::tr1::shared_ptr<const ChangeByResolventSequence> >
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
        const std::tr1::shared_ptr<const PackageID> id;

        ChangeByResolventPackageIDIs(const std::tr1::shared_ptr<const PackageID> & i) :
            id(i)
        {
        }

        bool operator() (const ChangeByResolvent & r) const
        {
            return *r.package_id() == *id;
        }
    };
}

const std::tr1::shared_ptr<const PackageIDSequence>
Decider::_collect_staying(const std::tr1::shared_ptr<const ChangeByResolventSequence> & going_away) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> existing((*_imp->env)[selection::AllVersionsUnsorted(
                generator::All() | filter::InstalledAtRoot(FSEntry("/")))]);

    const std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
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
        const std::tr1::shared_ptr<const Resolution> & resolution,
        ChangesToMakeDecision & decision) const
{
    decision.set_destination(_make_destination_for(resolution, decision));
    decision.set_change_type(_make_change_type_for(resolution, decision));
}

const std::tr1::shared_ptr<Destination>
Decider::_make_destination_for(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    const std::tr1::shared_ptr<const Repository> repo(_find_repository_for(resolution, decision));
    if ((! repo->destination_interface()) ||
            (! repo->destination_interface()->is_suitable_destination_for(*decision.origin_id())))
        throw InternalError(PALUDIS_HERE, stringify(repo->name()) + " is not a suitable destination for "
                + stringify(*decision.origin_id()));

    return make_shared_ptr(new Destination(make_named_values<Destination>(
                    n::replacing() = _find_replacing(decision.origin_id(), repo),
                    n::repository() = repo->name()
                    )));
}

const ChangeType
Decider::_make_change_type_for(
        const std::tr1::shared_ptr<const Resolution> &,
        const ChangesToMakeDecision & decision) const
{
    if (decision.destination()->replacing()->empty())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> others((*_imp->env)[selection::SomeArbitraryVersion(
                    generator::Package(decision.origin_id()->name()) &
                    generator::InRepository(decision.destination()->repository())
                    )]);
        if (others->empty())
            return ct_new;
        else
            return ct_slot_new;
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

const std::tr1::shared_ptr<const Repository>
Decider::_find_repository_for(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const ChangesToMakeDecision & decision) const
{
    return _imp->fns.find_repository_for_fn()(resolution, decision);
}

FilteredGenerator
Decider::_make_destination_filtered_generator(const Generator & g,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    return _imp->fns.make_destination_filtered_generator_fn()(g, resolution);
}

FilteredGenerator
Decider::_make_origin_filtered_generator(const Generator & g,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    return _imp->fns.make_origin_filtered_generator_fn()(g, resolution);
}

const std::tr1::shared_ptr<const PackageIDSequence>
Decider::_find_replacing(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Repository> & repo) const
{
    Context context("When working out what is replaced by '" + stringify(*id) +
            "' when it is installed to '" + stringify(repo->name()) + "':");

    std::set<RepositoryName> repos;

    if (repo->installed_root_key())
    {
        for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->installed_root_key() &&
                    (*r)->installed_root_key()->value() == repo->installed_root_key()->value())
                repos.insert((*r)->name());
    }
    else
        repos.insert(repo->name());

    std::tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    for (std::set<RepositoryName>::const_iterator r(repos.begin()),
            r_end(repos.end()) ;
            r != r_end ; ++r)
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsUnsorted(
                    generator::Package(id->name()) & generator::InRepository(*r))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->version() == id->version() || _same_slot(*i, id))
                result->push_back(*i);
        }
    }

    return result;
}

bool
Decider::_same_slot(const std::tr1::shared_ptr<const PackageID> & a,
        const std::tr1::shared_ptr<const PackageID> & b) const
{
    if (a->slot_key())
        return b->slot_key() && a->slot_key()->value() == b->slot_key()->value();
    else
        return ! b->slot_key();
}

const std::tr1::shared_ptr<Resolution>
Decider::_create_resolution_for_resolvent(const Resolvent & r) const
{
    return make_shared_ptr(new Resolution(make_named_values<Resolution>(
                    n::constraints() = _initial_constraints_for(r),
                    n::decision() = make_null_shared_ptr(),
                    n::resolvent() = r
                    )));
}

const std::tr1::shared_ptr<Resolution>
Decider::_resolution_for_resolvent(const Resolvent & r, const Tribool if_not_exist)
{
    ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->find(r));
    if (_imp->resolutions_by_resolvent->end() == i)
    {
        if (if_not_exist.is_true())
        {
            std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_resolvent(r));
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

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_target(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const PackageOrBlockDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    if (spec.if_package())
    {
        const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            n::destination_type() = resolution->resolvent().destination_type(),
                            n::nothing_is_fine_too() = false,
                            n::reason() = reason,
                            n::spec() = spec,
                            n::untaken() = false,
                            n::use_existing() = _imp->fns.get_use_existing_fn()(resolution, *spec.if_package(), reason)
                            ))));
        return result;
    }
    else if (spec.if_block())
        return _make_constraints_from_blocker(resolution, *spec.if_block(), reason);
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_dependency(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const SanitisedDependency & dep,
        const std::tr1::shared_ptr<const Reason> & reason,
        const SpecInterest interest) const
{
    if (dep.spec().if_package())
    {
        const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
        result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                            n::destination_type() = resolution->resolvent().destination_type(),
                            n::nothing_is_fine_too() = false,
                            n::reason() = reason,
                            n::spec() = *dep.spec().if_package(),
                            n::untaken() = si_untaken == interest,
                            n::use_existing() = _imp->fns.get_use_existing_fn()(resolution, *dep.spec().if_package(), reason)
                            ))));
        return result;
    }
    else if (dep.spec().if_block())
        return _make_constraints_from_blocker(resolution, *dep.spec().if_block(), reason);
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_blocker(
        const std::tr1::shared_ptr<const Resolution> &,
        const BlockDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    const std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);

    DestinationTypes destination_types(_get_destination_types_for_blocker(spec));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        if (destination_types[*t])
            result->push_back(make_shared_ptr(new Constraint(make_named_values<Constraint>(
                                n::destination_type() = *t,
                                n::nothing_is_fine_too() = true,
                                n::reason() = reason,
                                n::spec() = spec,
                                n::untaken() = false,
                                n::use_existing() = ue_if_possible
                                ))));

    return result;
}

void
Decider::_apply_resolution_constraint(
        const std::tr1::shared_ptr<Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
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
        const Constraint constraint;

        CheckConstraintVisitor(const Environment * const e, const Constraint & c) :
            env(e),
            constraint(c)
        {
        }

        bool ok(const std::tr1::shared_ptr<const PackageID> & chosen_id) const
        {
            if (constraint.spec().if_package())
            {
                if (! match_package(*env, *constraint.spec().if_package(), *chosen_id, MatchPackageOptions()))
                    return false;
            }
            else
            {
                if (match_package(*env, constraint.spec().if_block()->blocking(),
                            *chosen_id, MatchPackageOptions()))
                    return false;
            }

            return true;
        }

        bool visit(const ChangesToMakeDecision & decision) const
        {
            return ok(decision.origin_id());
        }

        bool visit(const ExistingNoChangeDecision & decision) const
        {
            return ok(decision.existing_id());
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
            return constraint.nothing_is_fine_too();
        }

        bool visit(const BreakDecision &) const
        {
            return true;
        }
    };

    struct CheckUseExistingVisitor
    {
        const std::tr1::shared_ptr<const Constraint> constraint;

        CheckUseExistingVisitor(const std::tr1::shared_ptr<const Constraint> & c) :
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
}

bool
Decider::_check_constraint(
        const std::tr1::shared_ptr<const Constraint> & constraint,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    if (! decision->accept_returning<bool>(CheckConstraintVisitor(_imp->env, *constraint)))
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
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    return _check_constraint(constraint, resolution->decision());
}

namespace
{
    struct WrongDecisionVisitor
    {
        std::tr1::function<void ()> restart;

        WrongDecisionVisitor(const std::tr1::function<void ()> & r) :
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
        const std::tr1::shared_ptr<Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    /* can we find a resolution that works for all our constraints? */
    std::tr1::shared_ptr<Resolution> adapted_resolution(make_shared_ptr(new Resolution(*resolution)));
    adapted_resolution->constraints()->add(constraint);

    const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(adapted_resolution));
    if (decision)
    {
        resolution->decision()->accept(WrongDecisionVisitor(std::tr1::bind(
                        &Decider::_suggest_restart_with, this, resolution, constraint, decision)));
        resolution->decision() = decision;
    }
    else
        resolution->decision() = _cannot_decide_for(adapted_resolution);
}

void
Decider::_suggest_restart_with(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    throw SuggestRestart(resolution->resolvent(), resolution->decision(), constraint, decision,
            _make_constraint_for_preloading(decision, constraint));
}

const std::tr1::shared_ptr<const Constraint>
Decider::_make_constraint_for_preloading(
        const std::tr1::shared_ptr<const Decision> &,
        const std::tr1::shared_ptr<const Constraint> & c) const
{
    const std::tr1::shared_ptr<Constraint> result(new Constraint(*c));

    const std::tr1::shared_ptr<PresetReason> reason(new PresetReason("restarted because of", c->reason()));
    result->reason() = reason;

    if (result->spec().if_package())
    {
        PackageDepSpec s(_make_spec_for_preloading(*result->spec().if_package()));
        result->spec().if_package() = make_shared_ptr(new PackageDepSpec(s));
    }
    else
    {
        PackageDepSpec s(_make_spec_for_preloading(result->spec().if_block()->blocking()));
        result->spec().if_block() = make_shared_ptr(new BlockDepSpec(
                    "!" + stringify(s),
                    s,
                    result->spec().if_block()->strong()));
    }

    return result;
}

const PackageDepSpec
Decider::_make_spec_for_preloading(const PackageDepSpec & spec) const
{
    PartiallyMadePackageDepSpec result(spec);

    /* we don't want to copy use deps from the constraint, since things like
     * [foo?] start to get weird when there's no longer an associated ID. */
    result.clear_additional_requirements();

    return result;
}

void
Decider::_decide(const std::tr1::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(resolution->resolvent()) + "':");

    _copy_other_destination_constraints(resolution);

    std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(resolution));
    if (decision)
        resolution->decision() = decision;
    else
        resolution->decision() = _cannot_decide_for(resolution);
}

void
Decider::_copy_other_destination_constraints(const std::tr1::shared_ptr<Resolution> & resolution)
{
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
    {
        if (*t == resolution->resolvent().destination_type())
            continue;

        Resolvent copy_from_resolvent(resolution->resolvent());
        copy_from_resolvent.destination_type() = *t;

        const std::tr1::shared_ptr<Resolution> copy_from_resolution(
                _resolution_for_resolvent(copy_from_resolvent, indeterminate));
        if (! copy_from_resolution)
            continue;

        for (Constraints::ConstIterator c(copy_from_resolution->constraints()->begin()),
                c_end(copy_from_resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_other_destination(
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
        const std::tr1::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & decision) const
        {
            if (decision.taken())
                return decision.existing_id();
            else
                return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            if (decision.taken())
                return decision.origin_id();
            else
                return make_null_shared_ptr();
        }

        const std::tr1::shared_ptr<const PackageID> visit(const BreakDecision &) const
        {
            return make_null_shared_ptr();
        }
    };
}

void
Decider::_add_dependencies_if_necessary(
        const std::tr1::shared_ptr<Resolution> & our_resolution)
{
    const std::tr1::shared_ptr<const PackageID> package_id(
            our_resolution->decision()->accept_returning<std::tr1::shared_ptr<const PackageID> >(
                DependenciesNecessityVisitor()));
    if (! package_id)
        return;

    Context context("When adding dependencies for '" + stringify(our_resolution->resolvent()) + "' with '"
            + stringify(*package_id) + "':");

    const std::tr1::shared_ptr<SanitisedDependencies> deps(new SanitisedDependencies);
    deps->populate(_imp->env, *this, our_resolution, package_id);

    for (SanitisedDependencies::ConstIterator s(deps->begin()), s_end(deps->end()) ;
            s != s_end ; ++s)
    {
        Context context_2("When handling dependency '" + stringify(s->spec()) + "':");

        SpecInterest interest(_interest_in_spec(our_resolution, *s));

        switch (interest)
        {
            case si_ignore:
                continue;

            case si_untaken:
            case si_take:
            case last_si:
                break;
        }

        const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(
                    package_id, our_resolution->resolvent(), *s, _already_met(*s)));

        std::tr1::shared_ptr<const Resolvents> resolvents;

        if (s->spec().if_package())
            resolvents = _get_resolvents_for(*s->spec().if_package(), reason);
        else
            resolvents = _get_resolvents_for_blocker(*s->spec().if_block());

        if (resolvents->empty())
        {
            if (s->spec().if_package())
                resolvents = _get_error_resolvents_for(*s->spec().if_package(), reason);
            else
            {
                /* blocking on something that doesn't exist is fine */
            }
        }

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(*r, true));
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(our_resolution, *s, reason, interest));

            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                _apply_resolution_constraint(dep_resolution, *c);
        }
    }
}

SpecInterest
Decider::_interest_in_spec(const std::tr1::shared_ptr<const Resolution> & resolution, const SanitisedDependency & dep) const
{
    return _imp->fns.interest_in_spec_fn()(resolution, dep);
}

const std::tr1::shared_ptr<Constraints>
Decider::_initial_constraints_for(const Resolvent & r) const
{
    return _imp->fns.get_initial_constraints_for_fn()(r);
}

std::pair<AnyChildScore, OperatorScore>
Decider::find_any_score(
        const std::tr1::shared_ptr<const Resolution> & our_resolution,
        const std::tr1::shared_ptr<const PackageID> & our_id,
        const SanitisedDependency & dep) const
{
    Context context("When working out whether we'd like '" + stringify(dep.spec()) + "' because of '"
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
        for (VersionRequirements::ConstIterator v(spec.version_requirements_ptr()->begin()),
                v_end(spec.version_requirements_ptr()->end()) ;
                v != v_end ; ++v)
        {
            OperatorScore local_score(os_worse_than_worst);

            switch (v->version_operator().value())
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
        Tribool prefer_or_avoid(_imp->fns.prefer_or_avoid_fn()(*spec.package_ptr()));
        if (prefer_or_avoid.is_true())
            return std::make_pair(is_block ? acs_avoid : acs_prefer, operator_bias);
        else if (prefer_or_avoid.is_false())
            return std::make_pair(is_block ? acs_prefer : acs_avoid, operator_bias);
    }

    /* best: blocker that doesn't match anything */
    if (is_block)
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements)
                        | filter::SupportsAction<InstallAction>() | filter::NotMasked()
                    )]);
        if (ids->empty())
            return std::make_pair(acs_vacuous_blocker, operator_bias);
    }

    /* next: already installed */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        if (! installed_ids->empty() ^ is_block)
            return std::make_pair(acs_already_installed, operator_bias);
    }

    /* next: already installed, except with the wrong options */
    if (! is_block && spec.additional_requirements_ptr())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        if (! installed_ids->empty())
            return std::make_pair(acs_wrong_options_installed, operator_bias);
    }

    const std::tr1::shared_ptr<DependencyReason> reason(new DependencyReason(
                our_id, our_resolution->resolvent(), dep, _already_met(dep)));
    const std::tr1::shared_ptr<const Resolvents> resolvents(_get_resolvents_for(spec, reason));

    /* next: will already be installing */
    if (! is_block)
    {
        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            ResolutionsByResolvent::ConstIterator i(_imp->resolutions_by_resolvent->find(*r));
            if (i != _imp->resolutions_by_resolvent->end())
                return std::make_pair(acs_will_be_installing, operator_bias);
        }
    }

    /* next: could install */
    if (! is_block)
    {
        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            const std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_resolvent(*r));
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_dependency(
                        our_resolution, dep, reason, si_take));
            for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                    c != c_end ; ++c)
                resolution->constraints()->add(*c);
            const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(resolution));
            if (decision)
                return std::make_pair(acs_could_install, operator_bias);
        }
    }

    /* next: blocks installed package */
    if (is_block)
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        if (! installed_ids->empty())
            return std::make_pair(acs_blocks_installed, operator_bias);
    }

    /* next: exists */
    if (! is_block)
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements)
                    )]);
        if (! ids->empty())
            return std::make_pair(acs_exists, operator_bias);
    }

    /* yay, people are depping upon packages that don't exist again. I SMELL A LESSPIPE. */
    return std::make_pair(acs_hate_hate_hate, operator_bias);
}

namespace
{
    struct SlotNameFinder
    {
        std::tr1::shared_ptr<SlotName> visit(const SlotExactRequirement & s)
        {
            return make_shared_ptr(new SlotName(s.slot()));
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyUnlockedRequirement &)
        {
            return make_null_shared_ptr();
        }

        std::tr1::shared_ptr<SlotName> visit(const SlotAnyLockedRequirement &)
        {
            return make_null_shared_ptr();
        }
    };
}

const std::tr1::shared_ptr<const Resolvents>
Decider::_get_resolvents_for_blocker(const BlockDepSpec & spec) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::tr1::shared_ptr<SlotName> exact_slot;
    if (spec.blocking().slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.blocking().slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
    }

    DestinationTypes destination_types(_get_destination_types_for_blocker(spec));
    std::tr1::shared_ptr<Resolvents> result(new Resolvents);
    if (exact_slot)
    {
        for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
            if (destination_types[*t])
                result->push_back(Resolvent(spec.blocking(), *exact_slot, *t));
    }
    else
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionInEachSlot(
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

const DestinationTypes
Decider::_get_destination_types_for_blocker(const BlockDepSpec &) const
{
    return DestinationTypes() + dt_install_to_slash;
}

const std::tr1::shared_ptr<const Resolvents>
Decider::_get_resolvents_for(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "':");

    std::tr1::shared_ptr<SlotName> exact_slot;

    if (spec.slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
    }

    return _imp->fns.get_resolvents_for_fn()(spec, exact_slot, reason);
}

const DestinationTypes
Decider::_get_destination_types_for(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    return _imp->fns.get_destination_types_for_fn()(spec, id, reason);
}

const std::tr1::shared_ptr<const Resolvents>
Decider::_get_error_resolvents_for(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const Reason> & reason) const
{
    Context context("When finding slots for '" + stringify(spec) + "', which can't be found the normal way:");

    std::tr1::shared_ptr<Resolvents> result(new Resolvents);
    DestinationTypes destination_types(_get_destination_types_for(spec, make_null_shared_ptr(), reason));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        if (destination_types[*t])
            result->push_back(Resolvent(spec, true, *t));
    return result;
}

const std::tr1::shared_ptr<Decision>
Decider::_try_to_find_decision_for(
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageID> existing_id(_find_existing_id_for(resolution));
    std::pair<const std::tr1::shared_ptr<const PackageID>, bool> installable_id_best(_find_installable_id_for(resolution));
    const std::tr1::shared_ptr<const PackageID> installable_id(installable_id_best.first);
    bool best(installable_id_best.second);

    if (resolution->constraints()->nothing_is_fine_too())
    {
        const std::tr1::shared_ptr<const PackageIDSequence> existing_resolvent_ids(_installed_ids(resolution));
        if (existing_resolvent_ids->empty())
        {
            /* nothing existing, but nothing's ok */
            return make_shared_ptr(new NothingNoChangeDecision(
                        resolution->resolvent(),
                        ! resolution->constraints()->all_untaken()
                        ));
        }
    }

    if (installable_id && ! existing_id)
    {
        /* there's nothing suitable existing. */
        return make_shared_ptr(new ChangesToMakeDecision(
                    resolution->resolvent(),
                    installable_id,
                    best,
                    last_ct,
                    ! resolution->constraints()->all_untaken(),
                    make_null_shared_ptr(),
                    std::tr1::bind(&Decider::_fixup_changes_to_make_decision, this, resolution, std::tr1::placeholders::_1)
                    ));
    }
    else if (existing_id && ! installable_id)
    {
        /* there's nothing installable. this may or may not be ok. */
        bool is_transient(existing_id->behaviours_key() && existing_id->behaviours_key()->value()->end() !=
                existing_id->behaviours_key()->value()->find("transient"));

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

        return make_shared_ptr(new ExistingNoChangeDecision(
                    resolution->resolvent(),
                    existing_id,
                    true,
                    true,
                    is_transient,
                    ! resolution->constraints()->all_untaken()
                    ));
    }
    else if ((! existing_id) && (! installable_id))
    {
        /* we can't stick with our existing id, if there is one, and we can't
         * fix it by installing things. this might be an error, or we might be
         * able to remove things. */
        if (resolution->constraints()->nothing_is_fine_too() && _installed_but_allowed_to_remove(resolution))
            return make_shared_ptr(new RemoveDecision(
                        resolution->resolvent(),
                        _installed_ids(resolution),
                        ! resolution->constraints()->all_untaken()
                        ));
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
            if (existing_id->choices_key() && installable_id->choices_key())
            {
                std::set<ChoiceNameWithPrefix> i_common, u_common;
                for (Choices::ConstIterator k(installable_id->choices_key()->value()->begin()),
                        k_end(installable_id->choices_key()->value()->end()) ;
                        k != k_end ; ++k)
                {
                    if (! (*k)->consider_added_or_changed())
                        continue;

                    for (Choice::ConstIterator i((*k)->begin()), i_end((*k)->end()) ;
                            i != i_end ; ++i)
                        if ((*i)->explicitly_listed())
                            i_common.insert((*i)->name_with_prefix());
                }

                for (Choices::ConstIterator k(existing_id->choices_key()->value()->begin()),
                        k_end(existing_id->choices_key()->value()->end()) ;
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
                if (installable_id->choices_key()->value()->find_by_name_with_prefix(*f)->enabled() !=
                        existing_id->choices_key()->value()->find_by_name_with_prefix(*f)->enabled())
                {
                    is_same = false;
                    break;
                }
        }

        bool is_transient(existing_id->behaviours_key() && existing_id->behaviours_key()->value()->end() !=
                existing_id->behaviours_key()->value()->find("transient"));

        /* we've got existing and installable. do we have any reason not to pick the existing id? */
        const std::tr1::shared_ptr<Decision> existing(new ExistingNoChangeDecision(
                    resolution->resolvent(),
                    existing_id,
                    is_same,
                    is_same_version,
                    is_transient,
                    ! resolution->constraints()->all_untaken()
                    ));
        const std::tr1::shared_ptr<Decision> changes_to_make(new ChangesToMakeDecision(
                    resolution->resolvent(),
                    installable_id,
                    best,
                    last_ct,
                    ! resolution->constraints()->all_untaken(),
                    make_null_shared_ptr(),
                    std::tr1::bind(&Decider::_fixup_changes_to_make_decision, this, resolution, std::tr1::placeholders::_1)
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

const std::tr1::shared_ptr<Decision>
Decider::_cannot_decide_for(
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<UnsuitableCandidates> unsuitable_candidates(new UnsuitableCandidates);

    const std::tr1::shared_ptr<const PackageID> existing_id(_find_existing_id_for(resolution));
    if (existing_id)
        unsuitable_candidates->push_back(_make_unsuitable_candidate(resolution, existing_id, true));

    const std::tr1::shared_ptr<const PackageIDSequence> installable_ids(_find_installable_id_candidates_for(resolution, true));
    for (PackageIDSequence::ConstIterator i(installable_ids->begin()), i_end(installable_ids->end()) ;
            i != i_end ; ++i)
        unsuitable_candidates->push_back(_make_unsuitable_candidate(resolution, *i, false));

    return make_shared_ptr(new UnableToMakeDecision(
                resolution->resolvent(),
                unsuitable_candidates,
                ! resolution->constraints()->all_untaken()
                ));
}

UnsuitableCandidate
Decider::_make_unsuitable_candidate(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageID> & id,
        const bool existing) const
{
    return make_named_values<UnsuitableCandidate>(
            n::package_id() = id,
            n::unmet_constraints() = _get_unmatching_constraints(resolution, id, existing)
            );
}

const std::tr1::shared_ptr<const PackageID>
Decider::_find_existing_id_for(const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids(_installed_ids(resolution));
    return _find_id_for_from(resolution, ids).first;
}

bool
Decider::_installed_but_allowed_to_remove(const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids(_installed_ids(resolution));
    if (ids->empty())
        return false;

    return ids->end() == std::find_if(ids->begin(), ids->end(),
            std::tr1::bind(std::logical_not<bool>(), std::tr1::bind(&Decider::_allowed_to_remove,
                    this, resolution, std::tr1::placeholders::_1)));
}

bool
Decider::_allowed_to_remove(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageID> & id) const
{
    return id->supports_action(SupportsActionTest<UninstallAction>()) && _imp->fns.allowed_to_remove_fn()(resolution, id);
}

bool
Decider::_remove_if_dependent(const std::tr1::shared_ptr<const PackageID> & id) const
{
    return _imp->fns.remove_if_dependent_fn()(id);
}

const std::tr1::shared_ptr<const PackageIDSequence>
Decider::_installed_ids(const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    return (*_imp->env)[selection::AllVersionsSorted(
            _make_destination_filtered_generator(generator::Package(resolution->resolvent().package()), resolution) |
            make_slot_filter(resolution->resolvent())
            )];
}

const std::tr1::shared_ptr<const PackageIDSequence>
Decider::_find_installable_id_candidates_for(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const bool include_errors) const
{
    return (*_imp->env)[selection::AllVersionsSorted(
            _make_origin_filtered_generator(generator::Package(resolution->resolvent().package()), resolution) |
            make_slot_filter(resolution->resolvent()) |
            filter::SupportsAction<InstallAction>() |
            ((! include_errors) ? Filter(filter::NotMasked()) : Filter(filter::All()))
            )];
}

const std::pair<const std::tr1::shared_ptr<const PackageID>, bool>
Decider::_find_installable_id_for(const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    return _find_id_for_from(resolution, _find_installable_id_candidates_for(resolution, false));
}

const std::pair<const std::tr1::shared_ptr<const PackageID>, bool>
Decider::_find_id_for_from(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageIDSequence> & ids) const
{
    std::tr1::shared_ptr<const PackageID> best_version;
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
                ok = ok && match_package(*_imp->env, *(*c)->spec().if_package(), **i, MatchPackageOptions());
            else
                ok = ok && ! match_package(*_imp->env, (*c)->spec().if_block()->blocking(), **i, MatchPackageOptions());

            if (! ok)
                break;
        }

        if (ok)
            return std::make_pair(*i, (*i)->version() == best_version->version());
    }

    return std::make_pair(make_null_shared_ptr(), false);
}

const std::tr1::shared_ptr<const Constraints>
Decider::_get_unmatching_constraints(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageID> & id,
        const bool existing) const
{
    const std::tr1::shared_ptr<Constraints> result(new Constraints);

    for (Constraints::ConstIterator c(resolution->constraints()->begin()),
            c_end(resolution->constraints()->end()) ;
            c != c_end ; ++c)
    {
        std::tr1::shared_ptr<Decision> decision;

        if (existing)
        {
            bool is_transient(id->behaviours_key() && id->behaviours_key()->value()->end() !=
                    id->behaviours_key()->value()->find("transient"));
            decision.reset(new ExistingNoChangeDecision(
                        resolution->resolvent(),
                        id,
                        true,
                        true,
                        is_transient,
                        ! (*c)->untaken()
                        ));
        }
        else
            decision.reset(new ChangesToMakeDecision(
                        resolution->resolvent(),
                        id,
                        false,
                        last_ct,
                        ! (*c)->untaken(),
                        make_null_shared_ptr(),
                        std::tr1::function<void (const ChangesToMakeDecision &)>()
                        ));
        if (! _check_constraint(*c, decision))
            result->add(*c);
    }

    return result;
}

const std::tr1::shared_ptr<const RewrittenSpec>
Decider::rewrite_if_special(
        const PackageOrBlockDepSpec & spec,
        const std::tr1::shared_ptr<const Resolvent> & maybe_from) const
{
    return _imp->rewriter.rewrite_if_special(spec, maybe_from);
}

void
Decider::add_target_with_reason(const PackageOrBlockDepSpec & spec, const std::tr1::shared_ptr<const Reason> & reason)
{
    Context context("When adding target '" + stringify(spec) + "':");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const RewrittenSpec> if_rewritten(rewrite_if_special(spec, make_null_shared_ptr()));
    if (if_rewritten)
    {
        for (Sequence<PackageOrBlockDepSpec>::ConstIterator i(if_rewritten->specs()->begin()), i_end(if_rewritten->specs()->end()) ;
                i != i_end ; ++i)
            add_target_with_reason(*i, reason);
    }
    else
    {
        PackageDepSpec base_spec(spec.if_package() ? *spec.if_package() : spec.if_block()->blocking());
        std::tr1::shared_ptr<const Resolvents> resolvents(_get_resolvents_for(base_spec, reason));
        if (resolvents->empty())
        {
            if (spec.if_package())
                resolvents = _get_error_resolvents_for(*spec.if_package(), reason);
            else
            {
                /* blocking on something that doesn't exist is fine */
            }
        }

        for (Resolvents::ConstIterator r(resolvents->begin()), r_end(resolvents->end()) ;
                r != r_end ; ++r)
        {
            Context context_2("When adding constraints from target '" + stringify(spec) + "' to resolvent '"
                    + stringify(*r) + "':");

            const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_resolvent(*r, true));
            const std::tr1::shared_ptr<ConstraintSequence> constraints(_make_constraints_from_target(dep_resolution, spec, reason));

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

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Collecting"));

    const std::tr1::shared_ptr<const PackageIDSet> have_now(_collect_installed());
    const std::tr1::shared_ptr<PackageIDSequence> have_now_seq(new PackageIDSequence);
    std::copy(have_now->begin(), have_now->end(), have_now_seq->back_inserter());

    const std::tr1::shared_ptr<const PackageIDSet> world(_collect_world(have_now));
    const std::tr1::shared_ptr<const PackageIDSet> world_plus_deps(_accumulate_deps_and_provides(world, have_now_seq, true));

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Calculating Unused"));

    const std::tr1::shared_ptr<PackageIDSet> unused(new PackageIDSet);
    std::set_difference(have_now->begin(), have_now->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(), PackageIDSetComparator());

    for (PackageIDSet::ConstIterator i(unused->begin()), i_end(unused->end()) ;
            i != i_end ; ++i)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if ((*i)->behaviours_key() && (*i)->behaviours_key()->value()->end() !=
                (*i)->behaviours_key()->value()->find("used"))
            continue;

        Resolvent resolvent(*i, dt_install_to_slash);
        const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        const std::tr1::shared_ptr<const ChangeByResolventSequence> used_to_use(new ChangeByResolventSequence);
        const std::tr1::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_purge(resolution, *i, used_to_use));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

        _decide(resolution);
    }
}

const std::tr1::shared_ptr<const PackageIDSet>
Decider::_collect_world(
        const std::tr1::shared_ptr<const PackageIDSet> & from) const
{
    const std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);
    const std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(SetName("world")));

    for (PackageIDSet::ConstIterator i(from->begin()), i_end(from->end()) ;
            i != i_end ; ++i)
        if (match_package_in_set(*_imp->env, *set, **i, MatchPackageOptions()))
            result->insert(*i);

    return result;
}

void
Decider::resolve()
{
    while (true)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Deciding"));
        _resolve_decide_with_dependencies();

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
Decider::_already_met(const SanitisedDependency & dep) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::AllVersionsUnsorted(
                generator::Matches(dep.spec().if_package() ?
                    *dep.spec().if_package() :
                    dep.spec().if_block()->blocking(),
                    MatchPackageOptions()) |
                filter::InstalledAtRoot(FSEntry("/")))]);
    if (installed_ids->empty())
        return dep.spec().if_block();
    else
    {
        if (dep.spec().if_block())
            return false;

        if (installed_ids->end() == std::find_if(installed_ids->begin(), installed_ids->end(),
                    std::tr1::bind(&Decider::_can_use, this, std::tr1::placeholders::_1)))
            return false;

        return true;
    }
}

bool
Decider::_can_use(
        const std::tr1::shared_ptr<const PackageID> & id) const
{
    return _imp->fns.can_use_fn()(id);
}

namespace
{
    struct ConfirmVisitor
    {
        const Environment * const env;
        const ResolverFunctions & fns;
        const std::tr1::shared_ptr<const Resolution> resolution;

        ConfirmVisitor(
                const Environment * const e,
                const ResolverFunctions & f,
                const std::tr1::shared_ptr<const Resolution> & r) :
            env(e),
            fns(f),
            resolution(r)
        {
        }

        void visit(ChangesToMakeDecision & changes_to_make_decision) const
        {
            if (! changes_to_make_decision.best())
            {
                const std::tr1::shared_ptr<RequiredConfirmation> c(new NotBestConfirmation);
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }

            if (ct_downgrade == changes_to_make_decision.change_type())
            {
                const std::tr1::shared_ptr<DowngradeConfirmation> c(new DowngradeConfirmation);
                if (! fns.confirm_fn()(resolution, c))
                    changes_to_make_decision.add_required_confirmation(c);
            }
        }

        void visit(BreakDecision & break_decision) const
        {
            const std::tr1::shared_ptr<BreakConfirmation> c(new BreakConfirmation);
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
                if (match_package_in_set(*env, *env->set(SetName("system")), **i, MatchPackageOptions()))
                    is_system = true;

            if (is_system)
            {
                const std::tr1::shared_ptr<RemoveSystemPackageConfirmation> c(new RemoveSystemPackageConfirmation);
                if (! fns.confirm_fn()(resolution, c))
                    remove_decision.add_required_confirmation(c);
            }
        }
    };
}

void
Decider::_confirm(
        const std::tr1::shared_ptr<const Resolution> & resolution)
{
    resolution->decision()->accept(ConfirmVisitor(_imp->env, _imp->fns, resolution));
}

bool
Decider::_resolve_purges()
{
    Context context("When finding things to purge:");

    const std::pair<
        std::tr1::shared_ptr<const ChangeByResolventSequence>,
        std::tr1::shared_ptr<const ChangeByResolventSequence> > going_away_newly_available(_collect_changing());

    const std::tr1::shared_ptr<PackageIDSet> going_away(new PackageIDSet);
    std::transform(going_away_newly_available.first->begin(), going_away_newly_available.first->end(),
            going_away->inserter(), get_change_by_resolvent_id);

    const std::tr1::shared_ptr<PackageIDSet> newly_available(new PackageIDSet);
    std::transform(going_away_newly_available.second->begin(), going_away_newly_available.second->end(),
            newly_available->inserter(), get_change_by_resolvent_id);

    const std::tr1::shared_ptr<const PackageIDSet> have_now(_collect_installed());

    const std::tr1::shared_ptr<PackageIDSet> have_now_minus_going_away(new PackageIDSet);
    std::set_difference(have_now->begin(), have_now->end(),
            going_away->begin(), going_away->end(), have_now_minus_going_away->inserter(), PackageIDSetComparator());

    const std::tr1::shared_ptr<PackageIDSet> will_eventually_have_set(new PackageIDSet);
    std::copy(have_now_minus_going_away->begin(), have_now_minus_going_away->end(), will_eventually_have_set->inserter());
    std::copy(newly_available->begin(), newly_available->end(), will_eventually_have_set->inserter());

    const std::tr1::shared_ptr<PackageIDSequence> will_eventually_have(new PackageIDSequence);
    std::copy(will_eventually_have_set->begin(), will_eventually_have_set->end(), will_eventually_have->back_inserter());

    const std::tr1::shared_ptr<const PackageIDSet> used_originally(_accumulate_deps_and_provides(going_away, will_eventually_have, false));
    const std::tr1::shared_ptr<const PackageIDSet> used_afterwards(_accumulate_deps_and_provides(newly_available, will_eventually_have, false));

    const std::tr1::shared_ptr<PackageIDSet> used_originally_and_not_going_away(new PackageIDSet);
    std::set_difference(used_originally->begin(), used_originally->end(),
            going_away->begin(), going_away->end(), used_originally_and_not_going_away->inserter(), PackageIDSetComparator());

    const std::tr1::shared_ptr<PackageIDSet> newly_unused(new PackageIDSet);
    std::set_difference(used_originally_and_not_going_away->begin(), used_originally_and_not_going_away->end(),
            used_afterwards->begin(), used_afterwards->end(), newly_unused->inserter(), PackageIDSetComparator());

    if (newly_unused->empty())
        return false;

    const std::tr1::shared_ptr<PackageIDSequence> newly_unused_seq(new PackageIDSequence);
    std::copy(newly_unused->begin(), newly_unused->end(), newly_unused_seq->back_inserter());

    const std::tr1::shared_ptr<PackageIDSet> used_by_unchanging(new PackageIDSet);
    for (PackageIDSet::ConstIterator u(have_now_minus_going_away->begin()), u_end(have_now_minus_going_away->end()) ;
            u != u_end ; ++u)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        const std::tr1::shared_ptr<const PackageIDSet> used(_collect_depped_upon(*u, newly_unused_seq));
        std::copy(used->begin(), used->end(), used_by_unchanging->inserter());
    }

    const std::tr1::shared_ptr<PackageIDSet> newly_really_unused(new PackageIDSet);
    std::set_difference(newly_unused->begin(), newly_unused->end(),
            used_by_unchanging->begin(), used_by_unchanging->end(), newly_really_unused->inserter(), PackageIDSetComparator());

    const std::tr1::shared_ptr<const SetSpecTree> world(_imp->env->set(SetName("world")));

    bool changed(false);
    for (PackageIDSet::ConstIterator i(newly_really_unused->begin()), i_end(newly_really_unused->end()) ;
            i != i_end ; ++i)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if ((*i)->behaviours_key() && (*i)->behaviours_key()->value()->end() !=
                (*i)->behaviours_key()->value()->find("used"))
            continue;

        /* to catch packages being purged that are also in world and not used
         * by anything else */
        if (match_package_in_set(*_imp->env, *world, **i, MatchPackageOptions()))
            continue;

        const std::tr1::shared_ptr<ChangeByResolventSequence> used_to_use(new ChangeByResolventSequence);
        const std::tr1::shared_ptr<PackageIDSequence> star_i_set(new PackageIDSequence);
        star_i_set->push_back(*i);
        for (ChangeByResolventSequence::ConstIterator g(going_away_newly_available.first->begin()), g_end(going_away_newly_available.first->end()) ;
                g != g_end ; ++g)
            if (! _collect_depped_upon(g->package_id(), star_i_set)->empty())
                used_to_use->push_back(*g);

        Resolvent resolvent(*i, dt_install_to_slash);
        const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_resolvent(resolvent, true));

        if (resolution->decision())
            continue;

        const std::tr1::shared_ptr<const ConstraintSequence> constraints(_make_constraints_for_purge(resolution, *i, used_to_use));
        for (ConstraintSequence::ConstIterator c(constraints->begin()), c_end(constraints->end()) ;
                c != c_end ; ++c)
            _apply_resolution_constraint(resolution, *c);

        _decide(resolution);

        if (resolution->decision()->taken())
            changed = true;
    }

    return changed;
}

const std::tr1::shared_ptr<const PackageIDSet>
Decider::_collect_installed() const
{
    const std::tr1::shared_ptr<const PackageIDSequence> q((*_imp->env)[selection::AllVersionsUnsorted(
                generator::All() | filter::InstalledAtRoot(FSEntry("/")))]);
    const std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);

    std::copy(q->begin(), q->end(), result->inserter());
    return result;
}

const std::tr1::shared_ptr<const PackageIDSet>
Decider::_accumulate_deps_and_provides(
        const std::tr1::shared_ptr<const PackageIDSet> & start,
        const std::tr1::shared_ptr<const PackageIDSequence> & will_eventually_have,
        const bool recurse) const
{
    const std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet), done(new PackageIDSet);
    std::copy(start->begin(), start->end(), result->inserter());

    while (result->size() > done->size())
    {
        const std::tr1::shared_ptr<PackageIDSet> more(new PackageIDSet);
        std::set_difference(result->begin(), result->end(), done->begin(), done->end(), more->inserter(), PackageIDSetComparator());

        for (PackageIDSet::ConstIterator i(more->begin()), i_end(more->end()) ;
                i != i_end ; ++i)
        {
            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            done->insert(*i);

            const std::tr1::shared_ptr<const PackageIDSet> depped_upon(_collect_depped_upon(*i, will_eventually_have));
            std::copy(depped_upon->begin(), depped_upon->end(), result->inserter());

            const std::tr1::shared_ptr<const PackageIDSet> provided(_collect_provided(*i));
            std::copy(provided->begin(), provided->end(), result->inserter());
        }

        if (! recurse)
            break;
    }

    return result;
}

const std::tr1::shared_ptr<const PackageIDSet>
Decider::_collect_depped_upon(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const PackageIDSequence> & candidates) const
{
    DependentChecker<PackageIDSequence> c(_imp->env, candidates, make_shared_ptr(new PackageIDSequence));
    if (id->dependencies_key())
        id->dependencies_key()->value()->root()->accept(c);
    else
    {
        if (id->build_dependencies_key())
            id->build_dependencies_key()->value()->root()->accept(c);
        if (id->run_dependencies_key())
            id->run_dependencies_key()->value()->root()->accept(c);
        if (id->post_dependencies_key())
            id->post_dependencies_key()->value()->root()->accept(c);
        if (id->suggested_dependencies_key())
            id->suggested_dependencies_key()->value()->root()->accept(c);
    }

    const std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);
    std::copy(c.result->begin(), c.result->end(), result->inserter());
    return result;
}

const std::tr1::shared_ptr<const PackageIDSet>
Decider::_collect_provided(
        const std::tr1::shared_ptr<const PackageID> & id) const
{
    const std::tr1::shared_ptr<PackageIDSet> result(new PackageIDSet);

    if (id->provide_key())
    {
        DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(_imp->env);
        id->provide_key()->value()->root()->accept(f);

        for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator v(f.begin()), v_end(f.end()) ;
                v != v_end ; ++v)
        {
            const std::tr1::shared_ptr<const PackageIDSequence> virtuals((*_imp->env)[selection::AllVersionsUnsorted(
                        generator::Matches(**v, MatchPackageOptions()))]);
            std::copy(virtuals->begin(), virtuals->end(), result->inserter());
        }
    }

    return result;
}

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_for_purge(
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const ChangeByResolventSequence> & r) const
{
    return _imp->fns.get_constraints_for_purge_fn()(resolution, id, r);
}

namespace
{
    struct ConstraintFromOtherDestinationVisitor
    {
        const DestinationType destination_type;
        const std::tr1::shared_ptr<const Constraint> from_constraint;
        const Resolvent resolvent;

        ConstraintFromOtherDestinationVisitor(
                const DestinationType t,
                const std::tr1::shared_ptr<const Constraint> f,
                const Resolvent & r) :
            destination_type(t),
            from_constraint(f),
            resolvent(r)
        {
        }

        const std::tr1::shared_ptr<ConstraintSequence> visit(const LikeOtherDestinationTypeReason &) const
        {
            std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
            return result;
        }

        const std::tr1::shared_ptr<ConstraintSequence> visit(const Reason &) const
        {
            std::tr1::shared_ptr<ConstraintSequence> result(new ConstraintSequence);
            result->push_back(make_shared_copy(make_named_values<Constraint>(
                            n::destination_type() = destination_type,
                            n::nothing_is_fine_too() = true,
                            n::reason() = make_shared_ptr(new LikeOtherDestinationTypeReason(
                                    resolvent,
                                    from_constraint->reason()
                                    )),
                            n::spec() = from_constraint->spec(),
                            n::untaken() = from_constraint->untaken(),
                            n::use_existing() = ue_if_possible
                            )));
            return result;
        }
    };
}

const std::tr1::shared_ptr<ConstraintSequence>
Decider::_make_constraints_from_other_destination(
        const std::tr1::shared_ptr<const Resolution> & new_resolution,
        const std::tr1::shared_ptr<const Resolution> & from_resolution,
        const std::tr1::shared_ptr<const Constraint> & from_constraint) const
{
    return from_constraint->reason()->accept_returning<std::tr1::shared_ptr<ConstraintSequence> >(
            ConstraintFromOtherDestinationVisitor(new_resolution->resolvent().destination_type(),
                from_constraint, from_resolution->resolvent()));
}

