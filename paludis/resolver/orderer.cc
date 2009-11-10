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

#include <paludis/resolver/orderer.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/decider.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/resolver.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/match_package.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/notifier_callback.hh>
#include <iostream>
#include <algorithm>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Orderer>
    {
        const Environment * const env;
        const std::tr1::shared_ptr<const Decider> decider;

        const std::tr1::shared_ptr<Resolutions> all_resolutions;
        const std::tr1::shared_ptr<Resolutions> ordered_resolutions;

        Implementation(const Environment * const e,
                const std::tr1::shared_ptr<const Decider> & d,
                const ResolverLists & l) :
            env(e),
            decider(d),
            all_resolutions(l.all()),
            ordered_resolutions(l.ordered())
        {
        }
    };
}

Orderer::Orderer(const Environment * const e, const std::tr1::shared_ptr<const Decider> & d,
        const ResolverLists & l) :
    PrivateImplementationPattern<Orderer>(new Implementation<Orderer>(e, d, l))
{
}

Orderer::~Orderer()
{
}

void
Orderer::_resolve_arrows()
{
    Context context("When creating arrows for order resolution:");

    for (Resolutions::ConstIterator i(_imp->all_resolutions->begin()),
            i_end(_imp->all_resolutions->end()) ;
            i != i_end ; ++i)
        for (Constraints::ConstIterator c((*i)->constraints()->begin()),
                c_end((*i)->constraints()->end()) ;
                c != c_end ; ++c)
            _resolve_arrow((*i)->resolvent(), _imp->decider->resolution_for_resolvent((*i)->resolvent()), *c);
}

namespace
{
    struct GetDependencyReason
    {
        const DependencyReason * visit(const DependencyReason & r) const
        {
            return &r;
        }

        const DependencyReason * visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<const DependencyReason *>(*this);
        }

        const DependencyReason * visit(const TargetReason &) const
        {
            return 0;
        }

        const DependencyReason * visit(const PresetReason &) const
        {
            return 0;
        }
    };

    struct ArrowInfo
    {
        bool causes_pre_arrow;
        bool ignorable;

        ArrowInfo(const DependencyReason & reason) :
            causes_pre_arrow(false),
            ignorable(true)
        {
            std::for_each(indirect_iterator(reason.sanitised_dependency().active_dependency_labels()->begin()),
                    indirect_iterator(reason.sanitised_dependency().active_dependency_labels()->end()), accept_visitor(*this));
        }

        void visit(const DependenciesBuildLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependenciesTestLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependenciesRunLabel &)
        {
            causes_pre_arrow = true;
        }

        void visit(const DependenciesPostLabel &)
        {
        }

        void visit(const DependenciesInstallLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependenciesCompileAgainstLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependenciesFetchLabel &)
        {
            causes_pre_arrow = true;
            ignorable = false;
        }

        void visit(const DependenciesRecommendationLabel &)
        {
        }

        void visit(const DependenciesSuggestionLabel &)
        {
        }
    };
}

void
Orderer::_resolve_arrow(
        const Resolvent & resolvent,
        const std::tr1::shared_ptr<Resolution> &,
        const std::tr1::shared_ptr<const Constraint> & constraint)
{
    GetDependencyReason gdr;
    const DependencyReason * if_dependency_reason(constraint->reason()->accept_returning<const DependencyReason *>(gdr));
    if (! if_dependency_reason)
        return;

    const Resolvent from_resolvent(if_dependency_reason->from_resolvent());
    const std::tr1::shared_ptr<Resolution> resolution(_imp->decider->resolution_for_resolvent(from_resolvent));

    /* deps between binaries don't count */
    if (resolvent.destination_type() == dt_create_binary &&
            if_dependency_reason->from_resolvent().destination_type() == dt_create_binary)
        return;

    if (constraint->spec().if_block())
    {
        if (constraint->spec().if_block()->strong())
        {
            resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                value_for<n::comes_after>(resolvent),
                                value_for<n::ignorable_pass>(0),
                                value_for<n::reason>(constraint->reason())
                                ))));
        }
    }
    else
    {
        ArrowInfo a(*if_dependency_reason);
        if (a.causes_pre_arrow)
        {
            int ignorable_pass(0);
            if (_already_met(if_dependency_reason->sanitised_dependency()))
                ignorable_pass = 1;
            else if (a.ignorable)
                ignorable_pass = 2;

            resolution->arrows()->push_back(make_shared_ptr(new Arrow(make_named_values<Arrow>(
                                value_for<n::comes_after>(resolvent),
                                value_for<n::ignorable_pass>(ignorable_pass),
                                value_for<n::reason>(constraint->reason())
                                ))));
        }
    }
}

bool
Orderer::_already_met(const SanitisedDependency & dep) const
{
    if (dep.spec().if_package())
    {
        std::tr1::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::SomeArbitraryVersion(
                    generator::Matches(*dep.spec().if_package(), MatchPackageOptions()) |
                    filter::InstalledAtRoot(FSEntry("/")))]);
        return ! ids->empty();
    }
    else if (dep.spec().if_block())
    {
        /* it's imposing an arrow, so it's a strong block */
        return false;
    }
    else
        throw InternalError(PALUDIS_HERE, "resolver bug: huh? it's not a block and it's not a package");
}

void
Orderer::_resolve_order()
{
    Context context("When finding an order for selected packages:");

    bool done(false);
    while (! done)
    {
        bool any(false);
        done = true;

        int ignore_pass(0);
        while (true)
        {
            for (Resolutions::ConstIterator i(_imp->decider->unordered_resolutions()->begin()),
                    i_end(_imp->decider->unordered_resolutions()->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->already_ordered())
                    continue;

                if (_can_order_now((*i)->resolvent(), *i, ignore_pass))
                {
                    if (0 != ignore_pass)
                        Log::get_instance()->message("resolver.cycle_breaking", ll_warning, lc_context)
                            << "Had to use cycle breaking with ignore pass " << ignore_pass
                            << " to order " << (*i)->resolvent() << " because of cycle "
                            << _find_cycle((*i)->resolvent(), false);

                    _imp->env->trigger_notifier_callback(NotifierCallbackResolverStepEvent());
                    _do_order((*i)->resolvent(), _imp->decider->resolution_for_resolvent((*i)->resolvent()));
                    any = true;

                    if (0 != ignore_pass)
                        break;
                }
                else
                    done = false;
            }

            if ((! done) && (! any))
            {
                if (ignore_pass >= 2)
                    _unable_to_order_more();
                else
                    ++ignore_pass;
            }
            else
                break;
        }
    }
}

bool
Orderer::_can_order_now(const Resolvent &, const std::tr1::shared_ptr<const Resolution> & resolution,
        const int ignorable_pass) const
{
    for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
            a != a_end ; ++a)
    {
        if (0 != (*a)->ignorable_pass())
            if ((*a)->ignorable_pass() <= ignorable_pass)
                continue;

        const std::tr1::shared_ptr<const Resolution> dep_resolution(
                _imp->decider->resolution_for_resolvent((*a)->comes_after()));
        if (! dep_resolution->already_ordered())
            return false;
    }

    return true;
}

void
Orderer::_do_order(const Resolvent &, const std::tr1::shared_ptr<Resolution> & resolution)
{
    _imp->ordered_resolutions->append(resolution);
    resolution->already_ordered() = true;
}

void
Orderer::_unable_to_order_more() const
{
    std::cout << "Unable to order any of the following:" << std::endl;

    for (Resolutions::ConstIterator i(_imp->decider->unordered_resolutions()->begin()),
            i_end(_imp->decider->unordered_resolutions()->end()) ;
            i != i_end ; ++i)
    {
        if ((*i)->already_ordered())
            continue;

        std::cout << "  * " << (*i)->resolvent() << " because of cycle "
            << _find_cycle((*i)->resolvent(), true)
            << std::endl;
    }

    throw InternalError(PALUDIS_HERE, "unimplemented: unfixable dep cycle");
}

namespace
{
    struct ReasonDescriber
    {
        const std::string visit(const DependencyReason & r) const
        {
            return " (" + r.sanitised_dependency().active_dependency_labels_as_string() + " " +
                stringify(r.sanitised_dependency().spec()) + ")";
        }

        const std::string visit(const TargetReason &) const
        {
            return "";
        }

        const std::string visit(const PresetReason &) const
        {
            return "";
        }

        const std::string visit(const SetReason &) const
        {
            return "";
        }
    };
}

const std::string
Orderer::_find_cycle(const Resolvent & start_resolvent, const int ignorable_pass) const
{
    std::stringstream result;

    std::set<Resolvent> seen;
    Resolvent current(start_resolvent);

    bool first(true);
    while (true)
    {
        if (! first)
            result << " -> ";
        first = false;

        result << current;

        if (! seen.insert(current).second)
            break;

        bool ok(false);
        const std::tr1::shared_ptr<const Resolution> resolution(_imp->decider->resolution_for_resolvent(current));
        for (ArrowSequence::ConstIterator a(resolution->arrows()->begin()), a_end(resolution->arrows()->end()) ;
                a != a_end ; ++a)
        {
            if (_can_order_now(current, resolution, ignorable_pass))
                continue;

            const std::tr1::shared_ptr<const Resolution> to_resolution(
                    _imp->decider->resolution_for_resolvent((*a)->comes_after()));
            if (to_resolution->already_ordered())
                continue;

            ok = true;
            result << (*a)->reason()->accept_returning<std::string>(ReasonDescriber());
            current = (*a)->comes_after();
            break;
        }

        if (! ok)
            throw InternalError(PALUDIS_HERE, "resolver bug: there's a cycle, but we don't know what it is");
    }

    return result.str();
}

const std::tr1::shared_ptr<const Resolutions>
Orderer::ordered_resolutions() const
{
    return _imp->ordered_resolutions;
}

void
Orderer::resolve()
{
    _resolve_arrows();
    _resolve_order();
}

