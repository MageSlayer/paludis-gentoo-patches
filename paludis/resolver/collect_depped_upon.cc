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

#include <paludis/resolver/collect_depped_upon.hh>
#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/version_spec.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    const std::shared_ptr<const PackageID> dependent_checker_id(const std::shared_ptr<const PackageID> & i)
    {
        return i;
    }

    const std::shared_ptr<const PackageID> dependent_checker_id(const ChangeByResolvent & i)
    {
        return i.package_id();
    }

    template <typename S_>
    const std::shared_ptr<const PackageID> best_eventual(
            const Environment * const env,
            const PackageDepSpec & spec,
            const std::shared_ptr<const PackageID> & from_id,
            const S_ & seq)
    {
        std::shared_ptr<const PackageID> result;

        for (auto i(seq->begin()), i_end(seq->end()) ;
                i != i_end ; ++i)
        {
            if ((! result) || dependent_checker_id(*i)->version() >= result->version())
                if (match_package(*env, spec, dependent_checker_id(*i), from_id, { }))
                    result = dependent_checker_id(*i);
        }

        return result;
    }

    template <typename C_>
    struct DependentChecker
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id_for_specs;
        const std::shared_ptr<const C_> going_away;
        const std::shared_ptr<const C_> newly_available;
        const std::shared_ptr<const PackageIDSequence> not_changing_slots;
        const std::shared_ptr<C_> result;

        DependentChecker(
                const Environment * const e,
                const std::shared_ptr<const PackageID> & i,
                const std::shared_ptr<const C_> & g,
                const std::shared_ptr<const C_> & n,
                const std::shared_ptr<const PackageIDSequence> & s) :
            env(e),
            id_for_specs(i),
            going_away(g),
            newly_available(n),
            not_changing_slots(s),
            result(std::make_shared<C_>())
        {
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & s)
        {
            const std::shared_ptr<const SetSpecTree> set(env->set(s.spec()->name()));
            set->top()->accept(*this);
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & s)
        {
            for (typename C_::ConstIterator g(going_away->begin()), g_end(going_away->end()) ;
                    g != g_end ; ++g)
            {
                auto spec(s.spec());

                if (s.spec()->slot_requirement_ptr() && visitor_cast<const SlotAnyUnlockedRequirement>(
                            *s.spec()->slot_requirement_ptr()))
                {
                    auto best_eventual_id(best_eventual(env, *s.spec(), id_for_specs, newly_available));
                    if (! best_eventual_id)
                        best_eventual_id = best_eventual(env, *s.spec(), id_for_specs, not_changing_slots);
                    if (best_eventual_id && best_eventual_id->slot_key())
                    {
                        PartiallyMadePackageDepSpec part_spec(*s.spec());
                        part_spec.slot_requirement(std::make_shared<ELikeSlotExactRequirement>(best_eventual_id->slot_key()->parse_value(), false));
                        spec = std::make_shared<PackageDepSpec>(part_spec);
                    }
                }

                if (! match_package(*env, *spec, dependent_checker_id(*g), id_for_specs, { }))
                    continue;

                bool any(false);
                for (typename C_::ConstIterator n(newly_available->begin()), n_end(newly_available->end()) ;
                        n != n_end ; ++n)
                {
                    if (match_package(*env, *spec, dependent_checker_id(*n), id_for_specs, { }))
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
            if (s.spec()->condition_met(env, id_for_specs))
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
}

const std::shared_ptr<const ChangeByResolventSequence>
paludis::resolver::dependent_upon(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & going_away,
        const std::shared_ptr<const ChangeByResolventSequence> & staying,
        const std::shared_ptr<const PackageIDSequence> & not_changing_slots)
{
    DependentChecker<ChangeByResolventSequence> c(env, id, going_away, staying, not_changing_slots);
    if (id->dependencies_key())
        id->dependencies_key()->parse_value()->top()->accept(c);
    else
    {
        if (id->build_dependencies_key())
            id->build_dependencies_key()->parse_value()->top()->accept(c);
        if (id->run_dependencies_key())
            id->run_dependencies_key()->parse_value()->top()->accept(c);
        if (id->post_dependencies_key())
            id->post_dependencies_key()->parse_value()->top()->accept(c);
        if (id->suggested_dependencies_key())
            id->suggested_dependencies_key()->parse_value()->top()->accept(c);
    }

    return c.result;
}

const std::shared_ptr<const PackageIDSet>
paludis::resolver::collect_depped_upon(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const PackageIDSequence> & candidates,
        const std::shared_ptr<const PackageIDSequence> & not_changing_slots)
{
    DependentChecker<PackageIDSequence> c(env, id, candidates, std::make_shared<PackageIDSequence>(), not_changing_slots);
    if (id->dependencies_key())
        id->dependencies_key()->parse_value()->top()->accept(c);
    else
    {
        if (id->build_dependencies_key())
            id->build_dependencies_key()->parse_value()->top()->accept(c);
        if (id->run_dependencies_key())
            id->run_dependencies_key()->parse_value()->top()->accept(c);
        if (id->post_dependencies_key())
            id->post_dependencies_key()->parse_value()->top()->accept(c);
        if (id->suggested_dependencies_key())
            id->suggested_dependencies_key()->parse_value()->top()->accept(c);
    }

    const std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    std::copy(c.result->begin(), c.result->end(), result->inserter());
    return result;
}

const std::shared_ptr<const PackageIDSet>
paludis::resolver::collect_dependents(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & going_away,
        const std::shared_ptr<const PackageIDSequence> & installed_ids)
{
    auto going_away_as_ids(std::make_shared<PackageIDSequence>());
    going_away_as_ids->push_back(going_away);

    auto result(std::make_shared<PackageIDSet>());

    for (auto i(installed_ids->begin()), i_end(installed_ids->end()) ;
            i != i_end ; ++i)
    {
        DependentChecker<PackageIDSequence> c(env, *i, going_away_as_ids,
                std::make_shared<PackageIDSequence>(), std::make_shared<PackageIDSequence>());

        if ((*i)->dependencies_key())
            (*i)->dependencies_key()->parse_value()->top()->accept(c);
        else
        {
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->parse_value()->top()->accept(c);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->parse_value()->top()->accept(c);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->parse_value()->top()->accept(c);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->parse_value()->top()->accept(c);
        }

        if (! c.result->empty())
            result->insert(*i);
    }

    return result;
}

