/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/lineariser.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/nag.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/strongly_connected_component.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/work_lists.hh>
#include <paludis/resolver/work_list.hh>
#include <paludis/resolver/work_item.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/lineariser_notes.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

typedef std::tr1::unordered_map<Resolvent, std::tr1::shared_ptr<const ChangeOrRemoveDecision>, Hash<Resolvent> > ChangeOrRemoveResolvents;

namespace paludis
{
    template <>
    struct Implementation<Lineariser>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<Resolved> resolved;
        ChangeOrRemoveResolvents change_or_remove_resolvents;

        Implementation(
                const Environment * const e,
                const std::tr1::shared_ptr<Resolved> & r) :
            env(e),
            resolved(r)
        {
        }
    };
}

Lineariser::Lineariser(
        const Environment * const e,
        const std::tr1::shared_ptr<Resolved> & r) :
    PrivateImplementationPattern<Lineariser>(new Implementation<Lineariser>(e, r))
{
}

Lineariser::~Lineariser()
{
}

namespace
{
    typedef std::tr1::unordered_set<Resolvent, Hash<Resolvent> > ResolventsSet;

    struct DecisionDispatcher
    {
        const std::tr1::shared_ptr<Resolved> resolved;
        ResolventsSet & ignore_dependencies_from_resolvents;
        ChangeOrRemoveResolvents & change_or_remove_resolvents;
        const Resolvent resolvent;
        const std::tr1::shared_ptr<const Decision> decision;

        DecisionDispatcher(
                const std::tr1::shared_ptr<Resolved> & r,
                ResolventsSet & i,
                ChangeOrRemoveResolvents & c,
                const Resolvent & v,
                const std::tr1::shared_ptr<const Decision> & d) :
            resolved(r),
            ignore_dependencies_from_resolvents(i),
            change_or_remove_resolvents(c),
            resolvent(v),
            decision(d)
        {
        }

        bool visit(const UnableToMakeDecision &)
        {
            if (decision->taken())
                resolved->taken_unable_to_make_decisions()->cast_push_back(decision);
            else
                resolved->untaken_unable_to_make_decisions()->cast_push_back(decision);

            ignore_dependencies_from_resolvents.insert(resolvent);

            return false;
        }

        bool visit(const NothingNoChangeDecision &)
        {
            resolved->nag()->add_node(resolvent);
            return true;
        }

        bool visit(const ExistingNoChangeDecision &)
        {
            resolved->nag()->add_node(resolvent);
            return true;
        }

        bool visit(const ChangesToMakeDecision &)
        {
            if (decision->taken())
            {
                resolved->nag()->add_node(resolvent);
                change_or_remove_resolvents.insert(std::make_pair(resolvent,
                            std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));
                return true;
            }
            else
            {
                resolved->untaken_change_or_remove_decisions()->cast_push_back(decision);
                return false;
            }
        }

        bool visit(const RemoveDecision &)
        {
            if (decision->taken())
            {
                resolved->nag()->add_node(resolvent);
                change_or_remove_resolvents.insert(std::make_pair(resolvent,
                            std::tr1::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));
                return true;
            }
            else
                throw InternalError(PALUDIS_HERE, "untaken RemoveDecision");
        }
    };

    struct LabelsClassifier
    {
        bool build;
        bool run;
        bool post;

        LabelsClassifier() :
            build(false),
            run(false),
            post(false)
        {
        }

        void visit(const DependenciesBuildLabel &)
        {
            build = true;
        }

        void visit(const DependenciesInstallLabel &)
        {
            build = true;
        }

        void visit(const DependenciesFetchLabel &)
        {
            build = true;
        }

        void visit(const DependenciesRunLabel &)
        {
            run = true;
        }

        void visit(const DependenciesTestLabel &)
        {
            build = true;
        }

        void visit(const DependenciesPostLabel &)
        {
            post = true;
        }

        void visit(const DependenciesSuggestionLabel &)
        {
            post = true;
        }

        void visit(const DependenciesRecommendationLabel &)
        {
            post = true;
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            build = true;
        }
    };

    struct EdgesFromReasonVisitor
    {
        const std::tr1::shared_ptr<NAG> nag;
        const ResolventsSet & ignore_dependencies_from_resolvents;
        const Resolvent resolvent;

        EdgesFromReasonVisitor(
                const std::tr1::shared_ptr<NAG> & n,
                const ResolventsSet & i,
                const Resolvent & v) :
            nag(n),
            ignore_dependencies_from_resolvents(i),
            resolvent(v)
        {
        }

        void visit(const DependencyReason & r)
        {
            /* we may be constrained by a dep from a package that was changed
             * from a non error decision to an unable to make decision */
            if (ignore_dependencies_from_resolvents.end() != ignore_dependencies_from_resolvents.find(r.from_resolvent()))
                return;

            /* what sort of dep are we? */
            LabelsClassifier classifier;
            for (DependenciesLabelSequence::ConstIterator l(r.sanitised_dependency().active_dependency_labels()->begin()),
                    l_end(r.sanitised_dependency().active_dependency_labels()->end()) ;
                    l != l_end ; ++l)
                (*l)->accept(classifier);

            if (classifier.build || classifier.run)
            {
                bool arrow(true);
                if (r.sanitised_dependency().spec().if_block())
                    if (! r.sanitised_dependency().spec().if_block()->strong())
                        arrow = false;

                if (arrow)
                    nag->add_edge(r.from_resolvent(), resolvent, make_named_values<NAGEdgeProperties>(
                                n::build() = classifier.build,
                                n::build_all_met() = r.already_met() || ! classifier.build,
                                n::run() = classifier.run,
                                n::run_all_met() = r.already_met() || ! classifier.run
                                ));
            }
            else if (classifier.post)
            {
                /* we won't add a backwards edge, since most post deps dep upon
                 * the thing requiring them anyway */
                // nag->add_edge(resolvent, r.from_resolvent());
            }
            else
                throw InternalError(PALUDIS_HERE, "No classification");
        }

        void visit(const SetReason & r)
        {
            if (r.reason_for_set())
                r.reason_for_set()->accept(*this);
        }

        void visit(const PresetReason &)
        {
        }

        void visit(const DependentReason &)
        {
        }

        void visit(const TargetReason &)
        {
        }
    };

    bool no_build_dependencies(
            const Set<Resolvent> & resolvents,
            const NAG & nag)
    {
        for (Set<Resolvent>::ConstIterator r(resolvents.begin()), r_end(resolvents.end()) ;
                r != r_end ; ++r)
            for (NAG::EdgesFromConstIterator e(nag.begin_edges_from(*r)), e_end(nag.end_edges_from(*r)) ;
                    e != e_end ; ++e)
                if (e->second.build())
                    return false;

        return true;
    }
}

void
Lineariser::resolve()
{
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Nodifying Decisions"));

    ResolventsSet ignore_dependencies_from_resolvents, ignore_edges_from_resolvents;
    for (ResolutionsByResolvent::ConstIterator r(_imp->resolved->resolutions_by_resolvent()->begin()),
            r_end(_imp->resolved->resolutions_by_resolvent()->end()) ;
            r != r_end ; ++r)
    {
        DecisionDispatcher decision_dispatcher(
                _imp->resolved,
                ignore_dependencies_from_resolvents,
                _imp->change_or_remove_resolvents,
                (*r)->resolvent(),
                (*r)->decision());
        if (! (*r)->decision()->accept_returning<bool>(decision_dispatcher))
            ignore_edges_from_resolvents.insert((*r)->resolvent());
    }

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Building NAG Edges"));

    for (ResolutionsByResolvent::ConstIterator r(_imp->resolved->resolutions_by_resolvent()->begin()),
            r_end(_imp->resolved->resolutions_by_resolvent()->end()) ;
            r != r_end ; ++r)
    {
        if (ignore_dependencies_from_resolvents.end() != ignore_edges_from_resolvents.find((*r)->resolvent()))
            continue;

        EdgesFromReasonVisitor edges_from_reason_visitor(_imp->resolved->nag(), ignore_dependencies_from_resolvents, (*r)->resolvent());
        for (Constraints::ConstIterator c((*r)->constraints()->begin()),
                c_end((*r)->constraints()->end()) ;
                c != c_end ; ++c)
            (*c)->reason()->accept(edges_from_reason_visitor);
    }

    _imp->resolved->nag()->verify_edges();

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Finding NAG SCCs"));
    const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> ssccs(_imp->resolved->nag()->sorted_strongly_connected_components());

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Linearising SCCs"));
    for (SortedStronglyConnectedComponents::ConstIterator scc(ssccs->begin()), scc_end(ssccs->end()) ;
            scc != scc_end ; ++scc)
    {
        /* some (or none, or all) of the nodes in our SCC are change or remove
         * nodes. this matters for cycle resolution. we identify them now, even
         * though our scc might just contain a single install, rather than
         * adding in extra useless code for the special easy case. */
        typedef std::tr1::unordered_set<Resolvent, Hash<Resolvent> > ChangesInSCC;
        ChangesInSCC changes_in_scc;

        for (Set<Resolvent>::ConstIterator r(scc->nodes()->begin()), r_end(scc->nodes()->end()) ;
                r != r_end ; ++r)
            if (_imp->change_or_remove_resolvents.end() != _imp->change_or_remove_resolvents.find(*r))
                changes_in_scc.insert(*r);

        if (changes_in_scc.empty())
        {
            /* two or more installed packages are codependent, but we don't have
             * to care */
        }
        else if (changes_in_scc.size() == 1)
        {
            /* there's only one real package in the component, so there's no
             * need to try anything clever */
            schedule(_imp->change_or_remove_resolvents.find(*changes_in_scc.begin())->second,
                    make_shared_copy(make_named_values<LineariserNotes>(
                            n::cycle_breaking() = ""
                            )));
        }
        else
        {
            /* whoop de doo. what do our SCCs look like if we only count change
             * or remove nodes? */
            NAG scc_nag;
            for (ChangesInSCC::const_iterator r(changes_in_scc.begin()), r_end(changes_in_scc.end()) ;
                    r != r_end ; ++r)
            {
                scc_nag.add_node(*r);
                /* we only need edges inside our SCC, and only those to other
                 * change or remove nodes */
                for (NAG::EdgesFromConstIterator e(_imp->resolved->nag()->begin_edges_from(*r)), e_end(_imp->resolved->nag()->end_edges_from(*r)) ;
                        e != e_end ; ++e)
                    if (changes_in_scc.end() != changes_in_scc.find(e->first))
                        scc_nag.add_edge(*r, e->first, e->second);
            }

            scc_nag.verify_edges();

            /* now we try again, hopefully with lots of small SCCs now */
            const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs(scc_nag.sorted_strongly_connected_components());
            linearise_sub_ssccs(scc_nag, *scc, sub_ssccs, true);
        }
    }
}

namespace
{
    std::string nice_resolvent(const Resolvent & r)
    {
        return stringify(r.package()) + stringify(r.slot());
    }
}

void
Lineariser::linearise_sub_ssccs(
        const NAG & scc_nag,
        const StronglyConnectedComponent & top_scc,
        const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> & sub_ssccs,
        const bool can_recurse)
{
    for (SortedStronglyConnectedComponents::ConstIterator sub_scc(sub_ssccs->begin()), sub_scc_end(sub_ssccs->end()) ;
            sub_scc != sub_scc_end ; ++sub_scc)
    {
        if (sub_scc->nodes()->size() == 1)
        {
            /* yay. it's all on its own. */
            schedule(_imp->change_or_remove_resolvents.find(*sub_scc->nodes()->begin())->second,
                    make_shared_copy(make_named_values<LineariserNotes>(
                            n::cycle_breaking() = (can_recurse ?
                                "In dependency cycle with existing packages: " + join(scc_nag.begin_nodes(), scc_nag.end_nodes(), ", ", nice_resolvent) :
                                "In dependency cycle with: " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_resolvent))
                            )));
        }
        else if (no_build_dependencies(*sub_scc->nodes(), scc_nag))
        {
            /* what's that, timmy? we have directly codependent nodes?
             * well i'm jolly glad that's because they're run
             * dependency cycles which we can order however we like! */
            for (Set<Resolvent>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
                schedule(_imp->change_or_remove_resolvents.find(*r)->second,
                    make_shared_copy(make_named_values<LineariserNotes>(
                            n::cycle_breaking() = "In run dependency cycle with: " + join(
                                sub_scc->nodes()->begin(), sub_scc->nodes()->end(), ", ", nice_resolvent) + (can_recurse ?
                                " in dependency cycle with " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_resolvent) : "")
                            )));
        }
        else if (can_recurse)
        {
            /* no, at least one of the deps is a build dep. let's try
             * this whole mess again, except without any edges for
             * dependencies that're already met */
            NAG scc_nag_without_met_deps;
            for (Set<Resolvent>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
            {
                scc_nag_without_met_deps.add_node(*r);
                for (NAG::EdgesFromConstIterator e(scc_nag.begin_edges_from(*r)), e_end(scc_nag.end_edges_from(*r)) ;
                        e != e_end ; ++e)
                    if ((! e->second.build_all_met()) || (! e->second.run_all_met()))
                        scc_nag_without_met_deps.add_edge(*r, e->first, make_named_values<NAGEdgeProperties>(
                                    n::build() = e->second.build() && ! e->second.build_all_met(),
                                    n::build_all_met() = e->second.build_all_met(),
                                    n::run() = e->second.run() && ! e->second.run_all_met(),
                                    n::run_all_met() = e->second.run_all_met()
                                    ));
            }

            scc_nag_without_met_deps.verify_edges();

            const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs_without_met_deps(
                    scc_nag_without_met_deps.sorted_strongly_connected_components());
            linearise_sub_ssccs(scc_nag_without_met_deps, top_scc, sub_ssccs_without_met_deps, false);
        }
        else
        {
            /* all that effort was wasted. there's incest and there's nothing we
             * can do to fix it. */
            throw InternalError(PALUDIS_HERE, "circular dependencies we're not smart enough to solve yet: { "
                    + join(sub_scc->nodes()->begin(), sub_scc->nodes()->end(), ", ", nice_resolvent) + " }");
        }
    }
}

void
Lineariser::schedule(
        const std::tr1::shared_ptr<const ChangeOrRemoveDecision> & d,
        const std::tr1::shared_ptr<const LineariserNotes> & n)
{
    _imp->resolved->taken_change_or_remove_decisions()->push_back(d, n);
    if (d->required_confirmations_if_any())
        _imp->resolved->taken_unconfirmed_change_or_remove_decisions()->push_back(d);

    const ChangesToMakeDecision * const changes_to_make_decision(simple_visitor_cast<const ChangesToMakeDecision>(*d));
    const RemoveDecision * const remove_decision(simple_visitor_cast<const RemoveDecision>(*d));

    if (changes_to_make_decision)
    {
        _imp->resolved->work_lists()->pretend_work_list()->append(make_shared_ptr(new PretendWorkItem(
                        changes_to_make_decision->origin_id())));

        _imp->resolved->work_lists()->execute_work_list()->append(make_shared_ptr(new FetchWorkItem(
                        changes_to_make_decision->origin_id())));

        _imp->resolved->work_lists()->execute_work_list()->append(make_shared_ptr(new InstallWorkItem(
                        changes_to_make_decision->origin_id(),
                        changes_to_make_decision->destination()->repository(),
                        changes_to_make_decision->resolvent().destination_type(),
                        changes_to_make_decision->destination()->replacing()
                        )));
    }
    else if (remove_decision)
    {
        _imp->resolved->work_lists()->execute_work_list()->append(make_shared_ptr(new UninstallWorkItem(
                        remove_decision->ids()
                        )));
    }
    else
        throw InternalError(PALUDIS_HERE, "huh?");
}

