/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/nag.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/strongly_connected_component.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_requirements.hh>
#include <paludis/resolver/destination.hh>
#include <paludis/resolver/orderer_notes.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/labels_classifier.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/package_id.hh>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

typedef std::unordered_map<NAGIndex, std::shared_ptr<const ChangeOrRemoveDecision>, Hash<NAGIndex> > ChangeOrRemoveIndices;
typedef std::unordered_map<NAGIndex, JobNumber, Hash<NAGIndex> > ChangeOrRemoveJobNumbers;
typedef std::unordered_map<Resolvent, JobNumber, Hash<Resolvent> > FetchJobNumbers;

namespace paludis
{
    template <>
    struct Imp<Orderer>
    {
        const Environment * const env;
        const ResolverFunctions fns;
        const std::shared_ptr<Resolved> resolved;
        ChangeOrRemoveIndices change_or_remove_indices;
        FetchJobNumbers fetch_job_numbers;
        ChangeOrRemoveJobNumbers change_or_remove_job_numbers;

        Imp(
                const Environment * const e,
                const ResolverFunctions & f,
                const std::shared_ptr<Resolved> & r) :
            env(e),
            fns(f),
            resolved(r)
        {
        }
    };
}

Orderer::Orderer(
        const Environment * const e,
        const ResolverFunctions & f,
        const std::shared_ptr<Resolved> & r) :
    _imp(e, f, r)
{
}

Orderer::~Orderer()
{
}

namespace
{
    typedef std::unordered_set<Resolvent, Hash<Resolvent> > ResolventsSet;

    struct DecisionDispatcher
    {
        const std::shared_ptr<Resolved> resolved;
        ResolventsSet & ignore_dependencies_from_resolvents;
        ChangeOrRemoveIndices & change_or_remove_indices;
        const Resolvent resolvent;
        const std::shared_ptr<const Decision> decision;

        DecisionDispatcher(
                const std::shared_ptr<Resolved> & r,
                ResolventsSet & i,
                ChangeOrRemoveIndices & c,
                const Resolvent & v,
                const std::shared_ptr<const Decision> & d) :
            resolved(r),
            ignore_dependencies_from_resolvents(i),
            change_or_remove_indices(c),
            resolvent(v),
            decision(d)
        {
        }

        bool visit(const UnableToMakeDecision &)
        {
            if (decision->taken())
                resolved->taken_unable_to_make_decisions()->push_back(
                        std::static_pointer_cast<const UnableToMakeDecision>(decision));
            else
                resolved->untaken_unable_to_make_decisions()->push_back(
                        std::static_pointer_cast<const UnableToMakeDecision>(decision));

            ignore_dependencies_from_resolvents.insert(resolvent);

            return false;
        }

        bool visit(const NothingNoChangeDecision &)
        {
            resolved->nag()->add_node(make_named_values<NAGIndex>(
                        n::resolvent() = resolvent,
                        n::role() = nir_done
                        ));
            return true;
        }

        bool visit(const ExistingNoChangeDecision &)
        {
            resolved->nag()->add_node(make_named_values<NAGIndex>(
                        n::resolvent() = resolvent,
                        n::role() = nir_done
                        ));
            return true;
        }

        bool visit(const ChangesToMakeDecision &)
        {
            if (decision->taken())
            {
                NAGIndex fetched_index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_fetched
                            ));
                resolved->nag()->add_node(fetched_index);
                change_or_remove_indices.insert(std::make_pair(fetched_index,
                            std::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));

                NAGIndex done_index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));
                resolved->nag()->add_node(done_index);
                change_or_remove_indices.insert(std::make_pair(done_index,
                            std::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));

                resolved->nag()->add_edge(done_index, fetched_index,
                        make_named_values<NAGEdgeProperties>(
                            n::always() = false,
                            n::build() = true,
                            n::build_all_met() = false,
                            n::run() = false,
                            n::run_all_met() = true
                            ));

                return true;
            }
            else
            {
                resolved->untaken_change_or_remove_decisions()->push_back(
                        std::static_pointer_cast<const ChangesToMakeDecision>(decision));
                return false;
            }
        }

        bool visit(const RemoveDecision &)
        {
            if (decision->taken())
            {
                NAGIndex index(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));
                resolved->nag()->add_node(index);
                change_or_remove_indices.insert(std::make_pair(index,
                            std::static_pointer_cast<const ChangeOrRemoveDecision>(decision)));
                return true;
            }
            else
            {
                resolved->untaken_change_or_remove_decisions()->push_back(
                        std::static_pointer_cast<const ChangeOrRemoveDecision>(decision));
                return false;
            }
        }

        bool visit(const BreakDecision & d)
        {
            if (d.required_confirmations_if_any())
                resolved->taken_unconfirmed_decisions()->push_back(
                        std::static_pointer_cast<const BreakDecision>(decision));
            return false;
        }
    };

    struct EdgesFromReasonVisitor
    {
        const Environment * const env;
        const std::shared_ptr<NAG> nag;
        const ResolventsSet & ignore_dependencies_from_resolvents;
        const Resolvent resolvent;
        const std::function<NAGIndexRole (const Resolvent &)> role_for_fetching;

        EdgesFromReasonVisitor(
                const Environment * const e,
                const std::shared_ptr<NAG> & n,
                const ResolventsSet & i,
                const Resolvent & v,
            const std::function<NAGIndexRole (const Resolvent &)> & f) :
            env(e),
            nag(n),
            ignore_dependencies_from_resolvents(i),
            resolvent(v),
            role_for_fetching(f)
        {
        }

        void visit(const DependencyReason & r)
        {
            /* we may be constrained by a dep from a package that was changed
             * from a non error decision to an unable to make decision */
            if (ignore_dependencies_from_resolvents.end() != ignore_dependencies_from_resolvents.find(r.from_resolvent()))
                return;

            /* what sort of dep are we? */
            LabelsClassifier classifier(env, r.from_id());
            for (DependenciesLabelSequence::ConstIterator l(r.sanitised_dependency().active_dependency_labels()->begin()),
                    l_end(r.sanitised_dependency().active_dependency_labels()->end()) ;
                    l != l_end ; ++l)
                (*l)->accept(classifier);

            if (classifier.includes_buildish || classifier.includes_non_post_runish)
            {
                bool normal(true);
                if (r.sanitised_dependency().spec().if_block())
                    switch (r.sanitised_dependency().spec().if_block()->block_kind())
                    {
                        case bk_weak:
                        case bk_uninstall_blocked_after:
                            normal = false;
                            break;

                        case bk_strong:
                        case bk_manual:
                        case bk_upgrade_blocked_before:
                        case bk_uninstall_blocked_before:
                            break;

                        case last_bk:
                            break;
                    }

                NAGIndex from(make_named_values<NAGIndex>(
                            n::resolvent() = r.from_resolvent(),
                            n::role() = classifier.includes_fetch ? role_for_fetching(r.from_resolvent()) : nir_done
                            ));

                NAGIndex to(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));

                if (normal)
                {
                    /* we might have added in binary creation later, so make from be
                     * our binary creation node rather than ourself, if applicable */
                    if (from.resolvent().destination_type() != dt_create_binary)
                    {
                        NAGIndex from_bin(from);
                        from_bin.resolvent().destination_type() = dt_create_binary;
                        if (nag->end_nodes() != nag->find_node(from_bin))
                            from = from_bin;
                    }

                    nag->add_edge(from, to,
                            make_named_values<NAGEdgeProperties>(
                                n::always() = false,
                                n::build() = classifier.includes_buildish,
                                n::build_all_met() = r.already_met().is_true() || ! classifier.includes_buildish,
                                n::run() = classifier.includes_non_post_runish,
                                n::run_all_met() = r.already_met().is_true() || ! classifier.includes_non_post_runish
                                ));
                }
                else
                {
                    nag->add_edge(to, from,
                            make_named_values<NAGEdgeProperties>(
                                n::always() = false,
                                n::build() = false,
                                n::build_all_met() = true,
                                n::run() = false,
                                n::run_all_met() = true
                                ));
                }
            }
            else if (classifier.includes_postish)
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

        void visit(const LikeOtherDestinationTypeReason & r)
        {
            if (r.reason_for_other())
                r.reason_for_other()->accept(*this);
        }

        void visit(const PresetReason &)
        {
        }

        void visit(const ViaBinaryReason &)
        {
            /* we do clever binary arrows later, to get binaries that just
             * happen to work anyway right too */
        }

        void visit(const DependentReason & r)
        {
            /* we may be constrained by a dep from a package that was changed
             * from a non error decision to an unable to make decision */
            if (ignore_dependencies_from_resolvents.end() != ignore_dependencies_from_resolvents.find(
                        r.id_and_resolvent_being_removed().resolvent()))
                return;

            NAGIndex from(make_named_values<NAGIndex>(
                        n::resolvent() = r.id_and_resolvent_being_removed().resolvent(),
                        n::role() = nir_done
                        ));

            NAGIndex to(make_named_values<NAGIndex>(
                        n::resolvent() = resolvent,
                        n::role() = nir_done
                        ));

            nag->add_edge(from, to,
                    make_named_values<NAGEdgeProperties>(
                        n::always() = false,
                        n::build() = false,
                        n::build_all_met() = true,
                        n::run() = false,
                        n::run_all_met() = true
                        ));
        }

        void visit(const TargetReason &)
        {
        }

        void visit(const WasUsedByReason & r)
        {
            for (ChangeByResolventSequence::ConstIterator i(r.ids_and_resolvents_being_removed()->begin()),
                    i_end(r.ids_and_resolvents_being_removed()->end()) ;
                    i != i_end ; ++i)
            {
                NAGIndex to(make_named_values<NAGIndex>(
                            n::resolvent() = i->resolvent(),
                            n::role() = nir_done
                            ));

                NAGIndex from(make_named_values<NAGIndex>(
                            n::resolvent() = resolvent,
                            n::role() = nir_done
                            ));

                nag->add_edge(from, to,
                        make_named_values<NAGEdgeProperties>(
                            n::always() = false,
                            n::build() = false,
                            n::build_all_met() = true,
                            n::run() = false,
                            n::run_all_met() = true
                            ));
            }
        }
    };

    bool no_build_dependencies(
            const Set<NAGIndex> & indices,
            const NAG & nag)
    {
        for (Set<NAGIndex>::ConstIterator r(indices.begin()), r_end(indices.end()) ;
                r != r_end ; ++r)
            for (NAG::EdgesFromConstIterator e(nag.begin_edges_from(*r)), e_end(nag.end_edges_from(*r)) ;
                    e != e_end ; ++e)
                if (e->second.build())
                    return false;

        return true;
    }
}

Tribool
Orderer::_order_early(const NAGIndex & i) const
{
    return _imp->fns.order_early_fn()(*_imp->resolved->resolutions_by_resolvent()->find(i.resolvent()));
}

void
Orderer::resolve()
{
    Context context("When resolving ordering:");

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Nodifying Decisions"));

    ResolventsSet ignore_dependencies_from_resolvents, ignore_edges_from_resolvents;
    for (ResolutionsByResolvent::ConstIterator r(_imp->resolved->resolutions_by_resolvent()->begin()),
            r_end(_imp->resolved->resolutions_by_resolvent()->end()) ;
            r != r_end ; ++r)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        DecisionDispatcher decision_dispatcher(
                _imp->resolved,
                ignore_dependencies_from_resolvents,
                _imp->change_or_remove_indices,
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
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (ignore_edges_from_resolvents.end() != ignore_edges_from_resolvents.find((*r)->resolvent()))
            continue;

        _add_binary_cleverness(*r);

        EdgesFromReasonVisitor edges_from_reason_visitor(_imp->env, _imp->resolved->nag(), ignore_dependencies_from_resolvents, (*r)->resolvent(),
                std::bind(&Orderer::_role_for_fetching, this, std::placeholders::_1));
        for (Constraints::ConstIterator c((*r)->constraints()->begin()),
                c_end((*r)->constraints()->end()) ;
                c != c_end ; ++c)
            (*c)->reason()->accept(edges_from_reason_visitor);
    }

    _imp->resolved->nag()->verify_edges();

    const std::function<Tribool (const NAGIndex &)> order_early_fn(std::bind(&Orderer::_order_early, this, std::placeholders::_1));

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Finding NAG SCCs"));
    const std::shared_ptr<const SortedStronglyConnectedComponents> ssccs(
            _imp->resolved->nag()->sorted_strongly_connected_components(order_early_fn));

    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStageEvent("Ordering SCCs"));
    for (SortedStronglyConnectedComponents::ConstIterator scc(ssccs->begin()), scc_end(ssccs->end()) ;
            scc != scc_end ; ++scc)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        /* some (or none, or all) of the nodes in our SCC are change or remove
         * nodes. this matters for cycle resolution. we identify them now, even
         * though our scc might just contain a single install, rather than
         * adding in extra useless code for the special easy case. */
        typedef std::unordered_set<NAGIndex, Hash<NAGIndex> > ChangesInSCC;
        ChangesInSCC changes_in_scc;

        for (Set<NAGIndex>::ConstIterator r(scc->nodes()->begin()), r_end(scc->nodes()->end()) ;
                r != r_end ; ++r)
            if (_imp->change_or_remove_indices.end() != _imp->change_or_remove_indices.find(*r))
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
            _check_self_deps_and_schedule(*changes_in_scc.begin(),
                    _imp->change_or_remove_indices.find(*changes_in_scc.begin())->second,
                    make_shared_copy(make_named_values<OrdererNotes>(
                            n::cycle_breaking() = ""
                            )));
        }
        else
        {
            Context sub_context("When considering only changes:");

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
            const std::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs(scc_nag.sorted_strongly_connected_components(order_early_fn));
            _order_sub_ssccs(scc_nag, *scc, sub_ssccs, true, order_early_fn);
        }
    }
}

void
Orderer::_add_binary_cleverness(const std::shared_ptr<const Resolution> & resolution)
{
    if (resolution->resolvent().destination_type() != dt_create_binary)
        return;

    const ChangesToMakeDecision * changes_decision(visitor_cast<const ChangesToMakeDecision>(*resolution->decision()));
    if (! changes_decision)
        return;

    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
    {
        if (*t == dt_create_binary)
            continue;

        Resolvent non_binary_resolvent(resolution->resolvent());
        non_binary_resolvent.destination_type() = *t;

        ResolutionsByResolvent::ConstIterator non_binary_resolution(_imp->resolved->resolutions_by_resolvent()->find(non_binary_resolvent));
        if (_imp->resolved->resolutions_by_resolvent()->end() == non_binary_resolution)
            continue;

        ChangesToMakeDecision * non_binary_changes_decision(visitor_cast<ChangesToMakeDecision>(*(*non_binary_resolution)->decision()));
        if (! non_binary_changes_decision)
            continue;

        if (changes_decision->origin_id() != non_binary_changes_decision->origin_id())
            throw InternalError(PALUDIS_HERE, "chose different origin ids: " + stringify(*changes_decision->origin_id()) + " vs "
                    + stringify(*non_binary_changes_decision->origin_id()));
        non_binary_changes_decision->set_via_new_binary_in(changes_decision->destination()->repository());

        NAGIndex from(make_named_values<NAGIndex>(
                    n::resolvent() = non_binary_resolvent,
                    n::role() = nir_fetched /* can't fetch until our origin exists */
                    ));

        NAGIndex to(make_named_values<NAGIndex>(
                    n::resolvent() = resolution->resolvent(),
                    n::role() = nir_done
                    ));

        _imp->resolved->nag()->add_edge(from, to,
                make_named_values<NAGEdgeProperties>(
                    n::always() = true,
                    n::build() = true,
                    n::build_all_met() = false,
                    n::run() = false,
                    n::run_all_met() = true
                    ));
    }
}

namespace
{
    std::string nice_index(const NAGIndex & x)
    {
        std::string result(stringify(x.resolvent().package()) + stringify(x.resolvent().slot()));
        if (x.role() == nir_fetched)
            result = result + " (fetch)";
        return result;
    }
}

void
Orderer::_order_sub_ssccs(
        const NAG & scc_nag,
        const StronglyConnectedComponent & top_scc,
        const std::shared_ptr<const SortedStronglyConnectedComponents> & sub_ssccs,
        const bool can_recurse,
        const std::function<Tribool (const NAGIndex &)> & order_early_fn)
{
    Context context("When ordering SSCCs" + std::string(can_recurse ? " for the first time" : " for the second time") + ":");

    for (SortedStronglyConnectedComponents::ConstIterator sub_scc(sub_ssccs->begin()), sub_scc_end(sub_ssccs->end()) ;
            sub_scc != sub_scc_end ; ++sub_scc)
    {
        _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

        if (sub_scc->nodes()->size() == 1)
        {
            /* yay. it's all on its own. */
            _check_self_deps_and_schedule(*sub_scc->nodes()->begin(),
                    _imp->change_or_remove_indices.find(*sub_scc->nodes()->begin())->second,
                    make_shared_copy(make_named_values<OrdererNotes>(
                            n::cycle_breaking() = (can_recurse ?
                                "In dependency cycle with existing packages: " + join(scc_nag.begin_nodes(), scc_nag.end_nodes(), ", ", nice_index) :
                                "In dependency cycle with: " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index))
                            )));
        }
        else if (no_build_dependencies(*sub_scc->nodes(), scc_nag))
        {
            /* what's that, timmy? we have directly codependent nodes?
             * well i'm jolly glad that's because they're run
             * dependency cycles which we can order however we like! */
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
                _check_self_deps_and_schedule(*r,
                        _imp->change_or_remove_indices.find(*r)->second,
                        make_shared_copy(make_named_values<OrdererNotes>(
                                n::cycle_breaking() = "In run dependency cycle with: " + join(
                                    sub_scc->nodes()->begin(), sub_scc->nodes()->end(), ", ", nice_index) + (can_recurse ?
                                    " in dependency cycle with " + join(top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index) : "")
                                )));
        }
        else if (can_recurse)
        {
            /* no, at least one of the deps is a build dep. let's try
             * this whole mess again, except without any edges for
             * dependencies that're already met */
            NAG scc_nag_without_met_deps;
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
            {
                scc_nag_without_met_deps.add_node(*r);
                for (NAG::EdgesFromConstIterator e(scc_nag.begin_edges_from(*r)), e_end(scc_nag.end_edges_from(*r)) ;
                        e != e_end ; ++e)
                    if (sub_scc->nodes()->end() != sub_scc->nodes()->find(e->first))
                        if ((! e->second.build_all_met()) || (! e->second.run_all_met()))
                            scc_nag_without_met_deps.add_edge(*r, e->first, make_named_values<NAGEdgeProperties>(
                                        n::always() = e->second.always(),
                                        n::build() = e->second.build() && ! e->second.build_all_met(),
                                        n::build_all_met() = e->second.build_all_met(),
                                        n::run() = e->second.run() && ! e->second.run_all_met(),
                                        n::run_all_met() = e->second.run_all_met()
                                        ));
            }

            scc_nag_without_met_deps.verify_edges();

            const std::shared_ptr<const SortedStronglyConnectedComponents> sub_ssccs_without_met_deps(
                    scc_nag_without_met_deps.sorted_strongly_connected_components(order_early_fn));
            _order_sub_ssccs(scc_nag_without_met_deps, top_scc, sub_ssccs_without_met_deps, false, order_early_fn);
        }
        else
        {
            for (Set<NAGIndex>::ConstIterator r(sub_scc->nodes()->begin()), r_end(sub_scc->nodes()->end()) ;
                    r != r_end ; ++r)
            {
                if (r->role() == nir_fetched && sub_scc->nodes()->end() != std::find(sub_scc->nodes()->begin(),
                            sub_scc->nodes()->end(), make_named_values<NAGIndex>(
                                n::resolvent() = r->resolvent(),
                                n::role() = nir_done
                                )))
                    continue;

                _imp->resolved->taken_unorderable_decisions()->push_back(
                        _imp->change_or_remove_indices.find(*r)->second,
                        make_shared_copy(make_named_values<OrdererNotes>(
                                n::cycle_breaking() = "In unsolvable cycle with " + join(
                                    top_scc.nodes()->begin(), top_scc.nodes()->end(), ", ", nice_index))));
            }
        }
    }
}

namespace
{
    typedef std::unordered_set<NAGIndex, Hash<NAGIndex> > RecursedRequirements;

    void populate_requirements(
            const std::shared_ptr<const NAG> & nag,
            const ChangeOrRemoveJobNumbers & change_or_remove_job_numbers,
            const NAGIndex & index,
            const std::shared_ptr<JobRequirements> & requirements,
            const bool is_uninstall,
            const bool is_fetch,
            const bool recursing,
            RecursedRequirements & recursed)
    {
        JobRequirementIfs basic_required_ifs;
        if (is_fetch)
            basic_required_ifs += jri_fetching;

        if (! recursing)
            for (NAG::EdgesFromConstIterator e(nag->begin_edges_from(index)),
                    e_end(nag->end_edges_from(index)) ;
                    e != e_end ; ++e)
            {
                if ((! e->second.build_all_met()) || (! e->second.run_all_met()) || is_uninstall)
                {
                    ChangeOrRemoveJobNumbers::const_iterator n(change_or_remove_job_numbers.find(e->first));
                    if (n != change_or_remove_job_numbers.end())
                        requirements->push_back(make_named_values<JobRequirement>(
                                    n::job_number() = n->second,
                                    n::required_if() = basic_required_ifs + jri_require_for_satisfied
                                    ));
                }

                if (e->second.always())
                {
                    ChangeOrRemoveJobNumbers::const_iterator n(change_or_remove_job_numbers.find(e->first));
                    if (n != change_or_remove_job_numbers.end())
                        requirements->push_back(make_named_values<JobRequirement>(
                                    n::job_number() = n->second,
                                    n::required_if() = basic_required_ifs + jri_require_always
                                    ));
                }
            }

        if ((! is_uninstall) && recursed.insert(index).second)
            for (NAG::EdgesFromConstIterator e(nag->begin_edges_from(index)),
                    e_end(nag->end_edges_from(index)) ;
                    e != e_end ; ++e)
            {
                ChangeOrRemoveJobNumbers::const_iterator n(change_or_remove_job_numbers.find(e->first));
                if (n != change_or_remove_job_numbers.end())
                    requirements->push_back(make_named_values<JobRequirement>(
                                n::job_number() = n->second,
                                n::required_if() = basic_required_ifs + jri_require_for_independent
                                ));
                populate_requirements(nag, change_or_remove_job_numbers, e->first, requirements,
                        is_uninstall, is_fetch, true, recursed);
            }
    }
}

void
Orderer::_check_self_deps_and_schedule(
        const NAGIndex & index,
        const std::shared_ptr<const ChangeOrRemoveDecision> & d,
        const std::shared_ptr<OrdererNotes> & n)
{
    /* do we dep directly upon ourself? */
    bool direct_self_dep(false), self_dep_is_met(true), self_dep_is_not_build(true);
    for (NAG::EdgesFromConstIterator e(_imp->resolved->nag()->begin_edges_from(index)),
            e_end(_imp->resolved->nag()->end_edges_from(index)) ;
            e != e_end ; ++e)
    {
        if (e->first == index)
        {
            direct_self_dep = true;
            self_dep_is_met = self_dep_is_met && e->second.build_all_met() && e->second.run_all_met();
            self_dep_is_not_build = self_dep_is_not_build && ! e->second.build();
        }
    }

    if (direct_self_dep)
    {
        if (! n->cycle_breaking().empty())
            n->cycle_breaking().append("; ");

        if (self_dep_is_met)
            n->cycle_breaking().append("Self dependent (already met)");
        else if (self_dep_is_not_build)
            n->cycle_breaking().append("Self dependent (runtime only)");
        else
            n->cycle_breaking().append("Self dependent (unsolvable)");
    }

    if (direct_self_dep && ! self_dep_is_met && ! self_dep_is_not_build)
    {
        _imp->resolved->taken_unorderable_decisions()->push_back(
                _imp->change_or_remove_indices.find(index)->second,
                n);
    }
    else
        _schedule(index, d, n);
}

namespace
{
    PackageDepSpec make_origin_spec(const ChangesToMakeDecision & changes_to_make_decision)
    {
        PartiallyMadePackageDepSpec result(changes_to_make_decision.origin_id()->uniquely_identifying_spec());

        if (changes_to_make_decision.if_via_new_binary_in())
            result.in_repository(*changes_to_make_decision.if_via_new_binary_in());

        return result;
    }

    struct ExtraScheduler
    {
        const std::shared_ptr<const Resolved> resolved;
        FetchJobNumbers & fetch_job_numbers;
        ChangeOrRemoveJobNumbers & change_or_remove_job_numbers;
        const NAGIndex index;

        ExtraScheduler(
                const std::shared_ptr<const Resolved> & r,
                FetchJobNumbers & f,
                ChangeOrRemoveJobNumbers & i,
                const NAGIndex & v) :
            resolved(r),
            fetch_job_numbers(f),
            change_or_remove_job_numbers(i),
            index(v)
        {
        }

        void visit(const ChangesToMakeDecision & changes_to_make_decision) const
        {
            switch (index.role())
            {
                case nir_done:
                    {
                        FetchJobNumbers::const_iterator fetch_job_n(fetch_job_numbers.find(index.resolvent()));
                        if (fetch_job_n == fetch_job_numbers.end())
                            throw InternalError(PALUDIS_HERE, "haven't scheduled the fetch for " + stringify(index.resolvent()) + " yet");

                        resolved->job_lists()->pretend_job_list()->append(std::make_shared<PretendJob>(
                                        changes_to_make_decision.origin_id()->uniquely_identifying_spec(),
                                        changes_to_make_decision.destination()->repository(),
                                        changes_to_make_decision.resolvent().destination_type()));

                        std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());
                        requirements->push_back(make_named_values<JobRequirement>(
                                    n::job_number() = fetch_job_n->second,
                                    n::required_if() = JobRequirementIfs() + jri_require_for_satisfied + jri_require_for_independent
                                        + jri_require_always + jri_fetching
                                    ));

                        RecursedRequirements recursed;
                        populate_requirements(
                                resolved->nag(),
                                change_or_remove_job_numbers,
                                index,
                                requirements,
                                false,
                                false,
                                false,
                                recursed
                                );

                        requirements = minimise_requirements(requirements);

                        const std::shared_ptr<Sequence<PackageDepSpec> > replacing(std::make_shared<Sequence<PackageDepSpec>>());
                        for (PackageIDSequence::ConstIterator i(changes_to_make_decision.destination()->replacing()->begin()),
                                i_end(changes_to_make_decision.destination()->replacing()->end()) ;
                                i != i_end ; ++i)
                            replacing->push_back((*i)->uniquely_identifying_spec());

                        JobNumber install_job_n(resolved->job_lists()->execute_job_list()->append(std::make_shared<InstallJob>(
                                            requirements,
                                            make_origin_spec(changes_to_make_decision),
                                            changes_to_make_decision.destination()->repository(),
                                            changes_to_make_decision.resolvent().destination_type(),
                                            replacing
                                            )));

                        change_or_remove_job_numbers.insert(std::make_pair(index, install_job_n));
                    }
                    return;

                case nir_fetched:
                    {
                        std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());

                        RecursedRequirements recursed;
                        populate_requirements(
                                resolved->nag(),
                                change_or_remove_job_numbers,
                                index,
                                requirements,
                                false,
                                true,
                                false,
                                recursed
                                );

                        requirements = minimise_requirements(requirements);

                        JobNumber fetch_job_n(resolved->job_lists()->execute_job_list()->append(std::make_shared<FetchJob>(
                                            requirements,
                                            make_origin_spec(changes_to_make_decision))));
                        fetch_job_numbers.insert(std::make_pair(index.resolvent(), fetch_job_n));
                    }
                    return;

                case last_nir:
                    break;
            }

            throw InternalError(PALUDIS_HERE, "bad index.role");
        }

        void visit(const RemoveDecision & remove_decision) const
        {
            const std::shared_ptr<Sequence<PackageDepSpec> > removing(std::make_shared<Sequence<PackageDepSpec>>());
            for (PackageIDSequence::ConstIterator i(remove_decision.ids()->begin()),
                    i_end(remove_decision.ids()->end()) ;
                    i != i_end ; ++i)
                removing->push_back((*i)->uniquely_identifying_spec());

            std::shared_ptr<JobRequirements> requirements(std::make_shared<JobRequirements>());
            RecursedRequirements recursed;
            populate_requirements(
                    resolved->nag(),
                    change_or_remove_job_numbers,
                    index,
                    requirements,
                    true,
                    false,
                    false,
                    recursed
                    );

            requirements = minimise_requirements(requirements);

            JobNumber uninstall_job_n(resolved->job_lists()->execute_job_list()->append(std::make_shared<UninstallJob>(
                                requirements,
                                removing
                                )));

            change_or_remove_job_numbers.insert(std::make_pair(index, uninstall_job_n));
        }
    };
}

void
Orderer::_schedule(
        const NAGIndex & index,
        const std::shared_ptr<const ChangeOrRemoveDecision> & d,
        const std::shared_ptr<const OrdererNotes> & n)
{
    do
    {
        switch (index.role())
        {
            case nir_done:
                _imp->resolved->taken_change_or_remove_decisions()->push_back(d, n);
                if (d->required_confirmations_if_any())
                    _imp->resolved->taken_unconfirmed_decisions()->push_back(d);
                continue;

            case nir_fetched:
                continue;

            case last_nir:
                break;
        }

        throw InternalError(PALUDIS_HERE, "bad index.role");
    } while (false);

    d->accept(ExtraScheduler(_imp->resolved, _imp->fetch_job_numbers, _imp->change_or_remove_job_numbers, index));
}

namespace
{
    struct RoleForFetchingVisitor
    {
        NAGIndexRole visit(const ChangesToMakeDecision &) const
        {
            return nir_fetched;
        }

        NAGIndexRole visit(const ExistingNoChangeDecision &) const
        {
            return nir_done;
        }

        NAGIndexRole visit(const UnableToMakeDecision &) const
        {
            return nir_done;
        }

        NAGIndexRole visit(const BreakDecision &) const
        {
            return nir_done;
        }

        NAGIndexRole visit(const RemoveDecision &) const
        {
            return nir_done;
        }

        NAGIndexRole visit(const NothingNoChangeDecision &) const
        {
            return nir_done;
        }
    };
}

NAGIndexRole
Orderer::_role_for_fetching(
        const Resolvent & resolvent) const
{
    return (*_imp->resolved->resolutions_by_resolvent()->find(
                resolvent))->decision()->accept_returning<NAGIndexRole>(RoleForFetchingVisitor());
}

