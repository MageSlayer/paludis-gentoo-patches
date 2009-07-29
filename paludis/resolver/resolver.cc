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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/log.hh>
#include <paludis/util/join.hh>
#include <paludis/environment.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/spec_tree.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/action.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_requirements.hh>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>

using namespace paludis;
using namespace paludis::resolver;

typedef std::map<QPN_S, std::tr1::shared_ptr<Resolution> > ResolutionsByQPN_SMap;
typedef std::map<QPN_S, std::tr1::shared_ptr<Constraints> > InitialConstraints;
typedef std::list<std::tr1::shared_ptr<const Resolution> > OrderedResolutionsList;

namespace paludis
{
    template <>
    struct Implementation<Resolver>
    {
        const Environment * const env;
        ResolutionsByQPN_SMap resolutions_by_qpn_s;
        InitialConstraints initial_constraints;
        OrderedResolutionsList ordered_resolutions;

        Implementation(const Environment * const e) :
            env(e)
        {
        }
    };
}

Resolver::Resolver(const Environment * const e) :
    PrivateImplementationPattern<Resolver>(new Implementation<Resolver>(e))
{
}

Resolver::~Resolver()
{
}

void
Resolver::resolve()
{
    Context context("When finding an appropriate resolution:");

    _resolve_dependencies();
    _resolve_arrows();
    _resolve_order();
}

void
Resolver::_resolve_dependencies()
{
    Context context("When resolving dependencies recursively:");

    bool done(false);
    while (! done)
    {
        done = true;

        for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                i != i_end ; ++i)
        {
            if (i->second->decision())
                continue;

            _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

            done = false;
            _decide(i->first, i->second);

            _add_dependencies(i->first, i->second);
        }
    }
}

void
Resolver::add_target(const PackageDepSpec & spec, const UseInstalled use_installed)
{
    Context context("When adding target '" + stringify(spec) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    std::list<QPN_S> qpn_s_s;
    _find_qpn_s_s_for_spec(spec, std::back_inserter(qpn_s_s));
    if (qpn_s_s.empty())
        throw InternalError(PALUDIS_HERE, "not implemented: no slot for " + stringify(spec));

    for (std::list<QPN_S>::const_iterator qpn_s(qpn_s_s.begin()), qpn_s_end(qpn_s_s.end()) ;
            qpn_s != qpn_s_end ; ++qpn_s)
    {
        Context context_2("When adding constraints from target '" + stringify(spec) + "' to qpn:s '"
                + stringify(*qpn_s) + "':");

        const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
        const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_target(spec, use_installed));

        _apply_resolution_constraint(*qpn_s, dep_resolution, constraint);
    }
}

void
Resolver::add_set(const SetName & set_name, const UseInstalled use_installed)
{
    Context context("When adding set target '" + stringify(set_name) + "':");
    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());

    const std::tr1::shared_ptr<const SetSpecTree> set(_imp->env->set(set_name));
    if (! set)
        throw InternalError(PALUDIS_HERE, "todo");

    DepSpecFlattener<SetSpecTree, PackageDepSpec> flattener(_imp->env);
    set->root()->accept(flattener);

    for (DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIterator s(flattener.begin()), s_end(flattener.end()) ;
            s != s_end ; ++s)
        add_target(**s, use_installed);
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

template <typename I_>
void
Resolver::_find_qpn_s_s_for_spec(const PackageDepSpec & spec, I_ iter) const
{
    std::tr1::shared_ptr<SlotName> exact_slot;

    if (spec.slot_requirement_ptr())
    {
        SlotNameFinder f;
        exact_slot = spec.slot_requirement_ptr()->accept_returning<std::tr1::shared_ptr<SlotName> >(f);
    }

    if (exact_slot)
        *iter++ = make_named_values<QPN_S>(
                value_for<n::package>(_package_from_spec(spec)),
                value_for<n::slot_name_or_null>(exact_slot)
                );
    else
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked())]);

        if (! ids->empty())
            *iter++ = qpn_s_from_id(*ids->begin());
        else
        {
            const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                        generator::Matches(spec, MatchPackageOptions() + mpo_ignore_additional_requirements) |
                        filter::SupportsAction<InstalledAction>())]);

            if (! installed_ids->empty())
                *iter++ = qpn_s_from_id(*installed_ids->begin());
        }
    }
}

QualifiedPackageName
Resolver::_package_from_spec(const PackageDepSpec & spec) const
{
    if (! spec.package_ptr())
        throw InternalError(PALUDIS_HERE, "not implemented");

    return *spec.package_ptr();
}

const std::tr1::shared_ptr<Resolution>
Resolver::_create_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    return make_shared_ptr(new Resolution(make_named_values<Resolution>(
                    value_for<n::already_ordered>(false),
                    value_for<n::arrows>(make_shared_ptr(new ArrowSequence)),
                    value_for<n::constraints>(_initial_constraints_for(qpn_s)),
                    value_for<n::decision>(make_null_shared_ptr())
                    )));
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s, const bool create)
{
    ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
    {
        if (create)
            i = _imp->resolutions_by_qpn_s.insert(std::make_pair(qpn_s, _create_resolution_for_qpn_s(qpn_s))).first;
        else
            throw InternalError(PALUDIS_HERE, "doesn't exist");
    }

    return i->second;
}

const std::tr1::shared_ptr<Resolution>
Resolver::_resolution_for_qpn_s(const QPN_S & qpn_s) const
{
    ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(qpn_s));
    if (_imp->resolutions_by_qpn_s.end() == i)
        throw InternalError(PALUDIS_HERE, "doesn't exist");

    return i->second;
}

QPN_S
Resolver::qpn_s_from_id(const std::tr1::shared_ptr<const PackageID> & id) const
{
    return make_named_values<QPN_S>(
            value_for<n::package>(id->name()),
            value_for<n::slot_name_or_null>(id->slot_key() ? make_shared_ptr(new SlotName(id->slot_key()->value())) : make_null_shared_ptr())
            );
}

const std::tr1::shared_ptr<Constraint>
Resolver::_make_constraint_from_target(const PackageDepSpec & spec, const UseInstalled use_installed) const
{
    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    // value_for<n::desire_strength>(ds_requirement),
                    value_for<n::reason>(make_shared_ptr(new TargetReason)),
                    value_for<n::spec>(spec),
                    value_for<n::use_installed>(use_installed)
                    )));
}

const std::tr1::shared_ptr<Constraint>
Resolver::_make_constraint_from_dependency(const QPN_S & qpn_s, const SanitisedDependency & dep) const
{
    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    // value_for<n::desire_strength>(_desire_strength_from_sanitised_dependency(qpn_s, dep)),
                    value_for<n::reason>(make_shared_ptr(new DependencyReason(qpn_s, dep))),
                    value_for<n::spec>(dep.spec()),
                    value_for<n::use_installed>(ui_if_same)
                    )));
}

void
Resolver::_apply_resolution_constraint(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    if (resolution->decision())
        _verify_new_constraint(qpn_s, resolution, constraint);

    resolution->constraints()->add(constraint);
}

void
Resolver::_verify_new_constraint(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & r,
        const std::tr1::shared_ptr<const Constraint> & c)
{
    if (! _constraint_matches(c, r->decision()))
        _made_wrong_decision(qpn_s, r, c);
}

bool
Resolver::_constraint_matches(
        const std::tr1::shared_ptr<const Constraint> & c,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    Context context("When working out whether '" + stringify(*decision) + "' is matched by '" + stringify(*c) + "':");

    bool ok(true);

    switch (c->use_installed())
    {
        case ui_never:
            ok = ok && ! decision->is_installed();
            break;

        case ui_only_if_transient:
            ok = ok && ((! decision->is_installed()) || (decision->is_transient()));
            break;

        case ui_if_possible:
            break;

        case ui_if_same:
            if (decision->is_installed())
                ok = ok && (decision->is_same() || decision->is_transient());
            break;

        case ui_if_same_version:
            if (decision->is_installed())
                ok = ok && (decision->is_same_version() || decision->is_transient());
            break;

        case last_ui:
            break;
    }

    ok = ok && match_package(*_imp->env, c->spec(), *decision->package_id(), MatchPackageOptions());

    return ok;
}

void
Resolver::_decide(const QPN_S & qpn_s, const std::tr1::shared_ptr<Resolution> & resolution)
{
    Context context("When deciding upon an origin ID to use for '" + stringify(qpn_s) + "' with '"
            + stringify(*resolution) + "':");

    std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, resolution));
    if (decision)
        resolution->decision() = decision;
    else
        _unable_to_decide(qpn_s, resolution);
}

const std::tr1::shared_ptr<Decision>
Resolver::_try_to_find_decision_for(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    std::tr1::shared_ptr<const PackageID> id;

    if (((id = _best_installed_id_for(qpn_s, resolution))))
        return _decision_from_package_id(qpn_s, id);

    if (((id = _best_installable_id_for(qpn_s, resolution))))
        return _decision_from_package_id(qpn_s, id);

    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_installed_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                _make_slot_filter(qpn_s) |
                filter::SupportsAction<InstalledAction>())]);

    return _best_id_from(ids, qpn_s, resolution);
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_installable_id_for(const QPN_S & qpn_s, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                _make_slot_filter(qpn_s) |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked())]);

    return _best_id_from(ids, qpn_s, resolution);
}

const std::tr1::shared_ptr<const PackageID>
Resolver::_best_id_from(
        const std::tr1::shared_ptr<const PackageIDSequence> & ids,
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
            i != i_end ; ++i)
    {
        bool ok(true);

        for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                c_end(resolution->constraints()->end()) ;
                c != c_end ; ++c)
        {
            if (! _constraint_matches(*c, _decision_from_package_id(qpn_s, *i)))
            {
                ok = false;
                break;
            }
        }

        if (ok)
            return *i;
    }

    return make_null_shared_ptr();
}

Filter
Resolver::_make_slot_filter(const QPN_S & qpn_s) const
{
    if (qpn_s.slot_name_or_null())
        return filter::Slot(*qpn_s.slot_name_or_null());
    else
        return filter::NoSlot();
}

void
Resolver::_add_dependencies(const QPN_S & our_qpn_s, const std::tr1::shared_ptr<Resolution> & our_resolution)
{
    Context context("When adding dependencies for '" + stringify(our_qpn_s) + "' with '"
            + stringify(*our_resolution) + "':");

    const std::tr1::shared_ptr<SanitisedDependencies> deps(new SanitisedDependencies);
    deps->populate(*this, our_resolution->decision()->package_id());

    for (SanitisedDependencies::ConstIterator s(deps->begin()), s_end(deps->end()) ;
            s != s_end ; ++s)
    {
        Context context_2("When handling dependency '" + stringify(*s) + "':");

        std::list<QPN_S> qpn_s_s;

        if (! _care_about_dependency_spec(our_qpn_s, our_resolution, *s))
            continue;

        _find_qpn_s_s_for_spec(s->spec(), std::back_inserter(qpn_s_s));
        if (qpn_s_s.empty())
        {
            if (our_resolution->decision()->is_installed())
                Log::get_instance()->message("resolver.cannot_find_installed_dep", ll_warning, lc_context)
                    << "Installed package '" << *our_resolution->decision() << "' dependency '" << *s << " cannot be met";
            else
                throw InternalError(PALUDIS_HERE, "not implemented: no slot for " + stringify(s->spec()));
        }

        for (std::list<QPN_S>::const_iterator qpn_s(qpn_s_s.begin()), qpn_s_end(qpn_s_s.end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> dep_resolution(_resolution_for_qpn_s(*qpn_s, true));
            const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_dependency(our_qpn_s, *s));

            _apply_resolution_constraint(*qpn_s, dep_resolution, constraint);
        }
    }
}

bool
Resolver::_care_about_dependency_spec(const QPN_S &,
        const std::tr1::shared_ptr<const Resolution> &, const SanitisedDependency &) const
{
    return true;
    // return _desire_strength_from_sanitised_dependency(qpn_s, dep) >= ds_recommendation;
}

void
Resolver::_resolve_arrows()
{
    Context context("When creating arrows for order resolution:");

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        for (Constraints::ConstIterator c(i->second->constraints()->begin()),
                c_end(i->second->constraints()->end()) ;
                c != c_end ; ++c)
        {
            if (! (*c)->reason()->if_dependency_reason())
                continue;

            const QPN_S from_qpns((*c)->reason()->if_dependency_reason()->qpn_s());
            const std::tr1::shared_ptr<Resolution> resolution(_resolution_for_qpn_s(from_qpns, false));

            if (_causes_pre_arrow(*(*c)->reason()->if_dependency_reason()))
                resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                    value_for<n::to_qpn_s>(i->first)
                                    ))));
        }
    }
}

namespace
{
    struct DependencyTypeLabelCausesPreArrow
    {
        bool visit(const DependencyBuildLabel &)
        {
            return true;
        }

        bool visit(const DependencyRunLabel &)
        {
            return true;
        }

        bool visit(const DependencyPostLabel &)
        {
            return false;
        }

        bool visit(const DependencyInstallLabel &)
        {
            return true;
        }

        bool visit(const DependencyCompileLabel &)
        {
            return true;
        }
    };
}

bool
Resolver::_causes_pre_arrow(const DependencyReason & reason) const
{
    const std::tr1::shared_ptr<const ActiveDependencyLabels> labels(
            reason.sanitised_dependency().active_dependency_labels());

    if (labels->type_labels()->empty())
        throw InternalError(PALUDIS_HERE, "why did that happen?");

    for (DependencyTypeLabelSequence::ConstIterator l(labels->type_labels()->begin()),
            l_end(labels->type_labels()->end()) ;
            l != l_end ; ++l)
    {
        DependencyTypeLabelCausesPreArrow v;
        if ((*l)->accept_returning<bool>(v))
            return true;
    }

    return false;
}

void
Resolver::_resolve_order()
{
    Context context("When finding an order for selected packages:");

    bool done(false);

    for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
        if (i->second->decision()->is_installed())
            i->second->already_ordered() = true;

    while (! done)
    {
        bool any(false);
        done = true;

        for (ResolutionsByQPN_SMap::iterator i(_imp->resolutions_by_qpn_s.begin()), i_end(_imp->resolutions_by_qpn_s.end()) ;
                i != i_end ; ++i)
        {
            if (i->second->already_ordered())
                continue;

            if (_can_order_now(i->first, i->second))
            {
                _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());
                _do_order(i->first, i->second);
                any = true;
            }
            else
                done = false;
        }

        if ((! done) && (! any))
            _unable_to_order_more();
    }
}

bool
Resolver::_can_order_now(const QPN_S &, const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
            a != a_end ; ++a)
    {
        const std::tr1::shared_ptr<const Resolution> dep_resolution(_resolution_for_qpn_s((*a)->to_qpn_s()));
        if (! dep_resolution->already_ordered())
            return false;
    }

    return true;
}

void
Resolver::_do_order(const QPN_S &, const std::tr1::shared_ptr<Resolution> & resolution)
{
    _imp->ordered_resolutions.push_back(resolution);
    resolution->already_ordered() = true;
}

const std::tr1::shared_ptr<Decision>
Resolver::_decision_from_package_id(const QPN_S & qpn_s, const std::tr1::shared_ptr<const PackageID> & id) const
{
    bool is_installed, is_transient, is_new, is_same, is_same_version;

    is_installed = id->supports_action(SupportsActionTest<InstalledAction>());
    is_transient = is_installed && id->transient_key() && id->transient_key()->value();

    std::tr1::shared_ptr<const PackageIDSequence> comparison_ids;

    if (is_installed)
        comparison_ids = ((*_imp->env)[selection::BestVersionOnly(
                    generator::Package(qpn_s.package()) |
                    _make_slot_filter(qpn_s) |
                    filter::SupportsAction<InstallAction>() |
                    filter::NotMasked())]);
    else
        comparison_ids = ((*_imp->env)[selection::BestVersionOnly(
                    generator::Package(qpn_s.package()) |
                    _make_slot_filter(qpn_s) |
                    filter::SupportsAction<InstalledAction>())]);

    if (comparison_ids->empty())
    {
        is_new = true;
        is_same = false;
        is_same_version = false;
    }
    else
    {
        is_new = false;
        is_same = ((*comparison_ids->begin())->version() == id->version());
        is_same_version = is_same;
    }

    return make_shared_ptr(new Decision(make_named_values<Decision>(
                    value_for<n::is_installed>(is_installed),
                    value_for<n::is_new>(is_new),
                    value_for<n::is_same>(is_same),
                    value_for<n::is_same_version>(is_same_version),
                    value_for<n::is_transient>(is_transient),
                    value_for<n::package_id>(id)
                    )));
}

void
Resolver::_unable_to_decide(
        const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution) const
{
    const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::Package(qpn_s.package()) |
                _make_slot_filter(qpn_s) |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked())]);

    if (ids->empty())
    {
        std::cout << "Unable to find anything at all unmasked and installable for " << qpn_s << std::endl;
    }
    else
    {
        std::cout << "Unable to find anything suitable for " << qpn_s << ":" << std::endl;
        for (PackageIDSequence::ReverseConstIterator i(ids->rbegin()), i_end(ids->rend()) ;
                i != i_end ; ++i)
        {
            std::cout << "  * " << **i << " doesn't match:" << std::endl;
            for (Constraints::ConstIterator c(resolution->constraints()->begin()),
                    c_end(resolution->constraints()->end()) ;
                    c != c_end ; ++c)
                if (! _constraint_matches(*c, _decision_from_package_id(qpn_s, *i)))
                    std::cout << "    * " << **c << std::endl;
        }
    }

    throw InternalError(PALUDIS_HERE, "not implemented");
}

void
Resolver::_unable_to_order_more() const
{
    std::cout << "Unable to order any of the follwoing:" << std::endl;

    for (ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.begin()),
            i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
    {
        if (i->second->already_ordered())
            continue;

        std::cout << "  * " << *i->second->decision() << std::endl;
    }

    throw InternalError(PALUDIS_HERE, "not implemented");
}

const std::tr1::shared_ptr<Constraints>
Resolver::_initial_constraints_for(const QPN_S & qpn_s) const
{
    InitialConstraints::const_iterator i(_imp->initial_constraints.find(qpn_s));
    if (i == _imp->initial_constraints.end())
        return make_shared_ptr(new Constraints);
    else
        return i->second;
}

void
Resolver::_made_wrong_decision(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    /* can we find a resolution that works for all our constraints? */
    std::tr1::shared_ptr<Resolution> adapted_resolution(make_shared_ptr(new Resolution(*resolution)));
    adapted_resolution->constraints()->add(constraint);

    const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(qpn_s, adapted_resolution));
    if (decision)
    {
        /* can we preload and restart? */
        InitialConstraints::const_iterator x(_imp->initial_constraints.find(qpn_s));
        if (x == _imp->initial_constraints.end())
        {
            /* we've not already locked this to something. yes! */
            _suggest_restart_with(qpn_s, resolution, constraint, decision);
        }
        else
        {
            /* we can restart if we've selected the same or a newer version
             * than before. but we don't support that yet. */
            throw InternalError(PALUDIS_HERE, "should have selected " + stringify(*decision));
        }
    }
    else
        _unable_to_decide(qpn_s, adapted_resolution);
}

void
Resolver::_suggest_restart_with(const QPN_S & qpn_s,
        const std::tr1::shared_ptr<const Resolution> & resolution,
        const std::tr1::shared_ptr<const Constraint> & constraint,
        const std::tr1::shared_ptr<const Decision> & decision) const
{
    std::cout << "Suggesting a restart: for " << qpn_s << ", we originally decided " << *resolution
        << ", but new constraint " << *constraint << " means we want " << *decision << " instead" << std::endl;

    // throw SuggestRestart(qpn_s, _make_constraint_for_preloading(decision));
    throw InternalError(PALUDIS_HERE, "need a restart");
}

const std::tr1::shared_ptr<const Constraint>
Resolver::_make_constraint_for_preloading(
        const std::tr1::shared_ptr<const Decision> & d) const
{
    return make_shared_ptr(new Constraint(make_named_values<Constraint>(
                    // value_for<n::desire_strength>(ds_none),
                    value_for<n::reason>(make_shared_ptr(new TargetReason)),
                    value_for<n::spec>(d->package_id()->uniquely_identifying_spec()),
                    value_for<n::use_installed>(ui_if_possible)
                    )));
}

void
Resolver::copy_initial_constraints_from(const Resolver & other)
{
    for (InitialConstraints::const_iterator i(other._imp->initial_constraints.begin()),
            i_end(other._imp->initial_constraints.end()) ;
            i != i_end ; ++i)
        _imp->initial_constraints.insert(*i);
}

void
Resolver::add_initial_constraint(const QPN_S & q, const std::tr1::shared_ptr<const Constraint> & c)
{
    InitialConstraints::iterator i(_imp->initial_constraints.find(q));
    if (i == _imp->initial_constraints.end())
        i = _imp->initial_constraints.insert(std::make_pair(q, make_shared_ptr(new Constraints))).first;
    i->second->add(c);
}

#if 0

namespace
{
    struct DesireStrengthFinder
    {
        DesireStrength visit(const DependencyRequiredLabel &) const
        {
            return ds_requirement;
        }

        DesireStrength visit(const DependencySuggestedLabel &) const
        {
            return ds_suggestion;
        }

        DesireStrength visit(const DependencyRecommendedLabel &) const
        {
            return ds_recommendation;
        }
    };
}

DesireStrength
Resolver::_desire_strength_from_sanitised_dependency(const QPN_S &, const SanitisedDependency & dep) const
{
    DesireStrength result(ds_none);

    if (dep.active_dependency_labels()->suggest_labels()->empty())
        result = ds_requirement;
    else
    {
        DesireStrengthFinder f;
        for (DependencySuggestLabelSequence::ConstIterator l(dep.active_dependency_labels()->suggest_labels()->begin()),
                l_end(dep.active_dependency_labels()->suggest_labels()->end()) ;
                l != l_end ; ++l)
            result = std::max(result, (*l)->accept_returning<DesireStrength>(f));
    }

    return result;
}
#endif

Resolver::ConstIterator
Resolver::begin() const
{
    return ConstIterator(_imp->ordered_resolutions.begin());
}

Resolver::ConstIterator
Resolver::end() const
{
    return ConstIterator(_imp->ordered_resolutions.end());
}

void
Resolver::dump(std::ostream & s) const
{
    s << "Initial Constraints:" << std::endl;
    for (InitialConstraints::const_iterator i(_imp->initial_constraints.begin()),
            i_end(_imp->initial_constraints.end()) ;
            i != i_end ; ++i)
        s << "  [*] " << std::left << std::setw(30) << i->first << " " <<
            join(indirect_iterator(i->second->begin()), indirect_iterator(i->second->end()), ", ") << std::endl;
    s << std::endl;

    s << "Resolutions by QPN:S:" << std::endl;
    for (ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.begin()),
            i_end(_imp->resolutions_by_qpn_s.end()) ;
            i != i_end ; ++i)
        s << "  [*] " << std::left << std::setw(30) << i->first << " " << *i->second << std::endl;
    s << std::endl;

    s << "Ordered Resolutions:" << std::endl;
    for (OrderedResolutionsList::const_iterator i(_imp->ordered_resolutions.begin()),
            i_end(_imp->ordered_resolutions.end()) ;
            i != i_end ; ++i)
        s << "  [*] " << **i << std::endl;
    s << std::endl;
}

int
Resolver::find_any_score(const QPN_S & our_qpn_s, const SanitisedDependency & dep) const
{
    Context context("When working out whether we'd like '" + stringify(dep) + "' because of '"
            + stringify(our_qpn_s) + "':");

    int operator_bias(0);
    if (dep.spec().version_requirements_ptr() && ! dep.spec().version_requirements_ptr()->empty())
    {
        int score(-1);
        for (VersionRequirements::ConstIterator v(dep.spec().version_requirements_ptr()->begin()),
                v_end(dep.spec().version_requirements_ptr()->end()) ;
                v != v_end ; ++v)
        {
            int local_score(0);

            switch (v->version_operator().value())
            {
                case vo_greater:
                case vo_greater_equal:
                    local_score = 9;
                    break;

                case vo_equal:
                case vo_tilde:
                case vo_nice_equal_star:
                case vo_stupid_equal_star:
                case vo_tilde_greater:
                    local_score = 2;
                    break;

                case vo_less_equal:
                case vo_less:
                    local_score = 1;
                    break;

                case last_vo:
                    local_score = 1;
                    break;
            }

            if (score == -1)
                score = local_score;
            else
                switch (dep.spec().version_requirements_mode())
                {
                    case vr_and:
                        score = std::min(score, local_score);
                        break;

                    case vr_or:
                        score = std::max(score, local_score);
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
        operator_bias = 9;
    }

    /* best: already installed */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> installed_ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(dep.spec(), MatchPackageOptions() + mpo_ignore_additional_requirements) |
                    filter::SupportsAction<InstalledAction>())]);
        if (! installed_ids->empty())
            return 40 + operator_bias;
    }

    std::list<QPN_S> qpn_s_s;
    _find_qpn_s_s_for_spec(dep.spec(), std::back_inserter(qpn_s_s));

    /* next: will already be installing */
    {
        for (std::list<QPN_S>::const_iterator qpn_s(qpn_s_s.begin()), qpn_s_end(qpn_s_s.end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            ResolutionsByQPN_SMap::const_iterator i(_imp->resolutions_by_qpn_s.find(*qpn_s));
            if (i != _imp->resolutions_by_qpn_s.end())
                return 30 + operator_bias;
        }
    }

    /* next: could install */
    {
        for (std::list<QPN_S>::const_iterator qpn_s(qpn_s_s.begin()), qpn_s_end(qpn_s_s.end()) ;
                qpn_s != qpn_s_end ; ++qpn_s)
        {
            const std::tr1::shared_ptr<Resolution> resolution(_create_resolution_for_qpn_s(*qpn_s));
            const std::tr1::shared_ptr<Constraint> constraint(_make_constraint_from_dependency(our_qpn_s, dep));
            resolution->constraints()->add(constraint);
            const std::tr1::shared_ptr<Decision> decision(_try_to_find_decision_for(*qpn_s, resolution));
            if (decision)
                return 20 + operator_bias;
        }
    }

    /* next: exists */
    {
        const std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::BestVersionOnly(
                    generator::Matches(dep.spec(), MatchPackageOptions() + mpo_ignore_additional_requirements)
                    )]);
        if (! ids->empty())
            return 10 + operator_bias;
    }

    /* yay, people are depping upon packages that don't exist again. I SMELL A LESSPIPE. */
    return 0;
}

