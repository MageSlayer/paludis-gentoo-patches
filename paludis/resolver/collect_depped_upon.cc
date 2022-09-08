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
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <paludis/spec_tree.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/match_package.hh>
#include <paludis/version_spec.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/slot.hh>

#include <algorithm>
#include <initializer_list>
#include <sstream>

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

    template <typename R_>
    struct ResultValueMaker;

    template <>
    struct ResultValueMaker<std::shared_ptr<const PackageID> >
    {
        static const std::shared_ptr<const PackageID> create(
                const std::shared_ptr<const PackageID> & i,
                const std::shared_ptr<const DependenciesLabelSequence> &)
        {
            return i;
        }
    };

    template <>
    struct ResultValueMaker<DependentPackageID>
    {
        static DependentPackageID create(
                const ChangeByResolvent & r,
                const std::shared_ptr<const DependenciesLabelSequence> & s)
        {
            std::stringstream adl;
            for (const auto & label : *s)
                adl << (adl.str().empty() ? "" : ", ") << stringify(*label);

            return make_named_values<DependentPackageID>(
                    n::active_dependency_labels_as_string() = adl.str(),
                    n::package_id() = r.package_id(),
                    n::resolvent() = r.resolvent()
                    );
        }
    };

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

    template <typename C_, typename R_>
    struct DependentChecker
    {
        const Environment * const env;
        const std::shared_ptr<const PackageID> id_for_specs;
        const std::shared_ptr<const C_> going_away;
        const std::shared_ptr<const C_> newly_available;
        const std::shared_ptr<const PackageIDSequence> not_changing_slots;
        const std::shared_ptr<R_> result;

        std::list<std::shared_ptr<const DependenciesLabelSequence> > labels_stack;

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
            result(std::make_shared<R_>())
        {
            labels_stack.push_front(std::make_shared<DependenciesLabelSequence>());
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & s)
        {
            const std::shared_ptr<const SetSpecTree> set(env->set(s.spec()->name()));
            set->top()->accept(*this);
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & s)
        {
            for (const auto & removing : *going_away)
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
                        part_spec.slot_requirement(std::make_shared<ELikeSlotExactPartialRequirement>(best_eventual_id->slot_key()->parse_value().parallel_value(), nullptr));
                        spec = std::make_shared<PackageDepSpec>(part_spec);
                    }
                }

                if (! match_package(*env, *spec, dependent_checker_id(removing), id_for_specs, { }))
                    continue;

                bool any(false);
                for (const auto & installing : *newly_available)
                {
                    if (match_package(*env, *spec, dependent_checker_id(installing), id_for_specs, { }))
                    {
                        any = true;
                        break;
                    }
                }

                if (! any)
                    result->push_back(ResultValueMaker<typename R_::value_type>::create(removing, *labels_stack.begin()));
            }
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & s)
        {
            if (s.spec()->condition_met(env, id_for_specs))
            {
                labels_stack.push_front(*labels_stack.begin());
                std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()), accept_visitor(*this));
                labels_stack.pop_front();
            }
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & s)
        {
            labels_stack.push_front(*labels_stack.begin());
            std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()), accept_visitor(*this));
            labels_stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & s)
        {
            labels_stack.push_front(*labels_stack.begin());
            std::for_each(indirect_iterator(s.begin()), indirect_iterator(s.end()), accept_visitor(*this));
            labels_stack.pop_front();
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            std::shared_ptr<DependenciesLabelSequence> labels(std::make_shared<DependenciesLabelSequence>());
            std::copy(node.spec()->begin(), node.spec()->end(), labels->back_inserter());
            *labels_stack.begin() = labels;
        }
    };
}

const std::shared_ptr<const DependentPackageIDSequence>
paludis::resolver::dependent_upon(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const ChangeByResolventSequence> & going_away,
        const std::shared_ptr<const ChangeByResolventSequence> & staying,
        const std::shared_ptr<const PackageIDSequence> & not_changing_slots)
{
    DependentChecker<ChangeByResolventSequence, DependentPackageIDSequence> c(env, id, going_away, staying, not_changing_slots);
    if (id->dependencies_key())
    {
        c.labels_stack.push_front(id->dependencies_key()->initial_labels());
        id->dependencies_key()->parse_value()->top()->accept(c);
        c.labels_stack.pop_front();
    }
    else
    {
        for (auto& fn : { &PackageID::build_dependencies_key, &PackageID::run_dependencies_key, &PackageID::post_dependencies_key })
        {
            auto key(((*id).*fn)());
            if (key)
            {
                c.labels_stack.push_front(key->initial_labels());
                key->parse_value()->top()->accept(c);
                c.labels_stack.pop_front();
            }
        }
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
    DependentChecker<PackageIDSequence, PackageIDSequence> c(env, id, candidates, std::make_shared<PackageIDSequence>(), not_changing_slots);
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

    for (const auto & id : *installed_ids)
    {
        DependentChecker<PackageIDSequence, PackageIDSequence> c(env, id, going_away_as_ids,
                std::make_shared<PackageIDSequence>(), std::make_shared<PackageIDSequence>());

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
        }

        if (! c.result->empty())
            result->insert(id);
    }

    return result;
}

void
DependentPackageID::serialise(Serialiser & s) const
{
    s.object("DependentPackageID")
        .member(SerialiserFlags<>(), "active_dependency_labels_as_string", active_dependency_labels_as_string())
        .member(SerialiserFlags<serialise::might_be_null>(), "package_id", package_id())
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        ;
}

const DependentPackageID
DependentPackageID::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "DependentPackageID");
    return make_named_values<DependentPackageID>(
            n::active_dependency_labels_as_string() = v.member<std::string>("active_dependency_labels_as_string"),
            n::package_id() = v.member<std::shared_ptr<const PackageID> >("package_id"),
            n::resolvent() = v.member<Resolvent>("resolvent")
            );
}

namespace paludis
{
    template class Sequence<DependentPackageID>;
    template class WrappedForwardIterator<Sequence<DependentPackageID>::ConstIteratorTag, const DependentPackageID>;
}

