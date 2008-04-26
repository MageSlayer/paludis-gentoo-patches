/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/show_suggest_visitor.hh>
#include <paludis/dep_list.hh>
#include <paludis/condition_tracker.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <paludis/dep_label.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <tr1/functional>
#include <set>
#include <list>

using namespace paludis;

typedef std::list<std::tr1::shared_ptr<ActiveDependencyLabels> > LabelsStack;

namespace paludis
{
    template <>
    struct Implementation<ShowSuggestVisitor>
    {
        DepList * const dep_list;
        std::tr1::shared_ptr<const DestinationsSet> destinations;
        const Environment * const environment;
        const std::tr1::shared_ptr<const PackageID> id;
        bool dependency_tags;
        const bool only_if_suggested_label;
        std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions;
        std::set<SetName> recursing_sets;
        LabelsStack labels;

        Implementation(DepList * const d, std::tr1::shared_ptr<const DestinationsSet> dd,
                const Environment * const e, const std::tr1::shared_ptr<const PackageID> & p, bool t, bool l) :
            dep_list(d),
            destinations(dd),
            environment(e),
            id(p),
            dependency_tags(t),
            only_if_suggested_label(l),
            conditions(std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >(
                           new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                               std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec))))
        {
            labels.push_front(make_shared_ptr(new ActiveDependencyLabels(*make_shared_ptr(new DependencyLabelSequence))));
        }
    };
}

ShowSuggestVisitor::ShowSuggestVisitor(DepList * const d, std::tr1::shared_ptr<const DestinationsSet> dd,
        const Environment * const e, const std::tr1::shared_ptr<const PackageID> & p, bool t, bool l) :
    PrivateImplementationPattern<ShowSuggestVisitor>(new Implementation<ShowSuggestVisitor>(d, dd, e, p, t, l))
{
}

ShowSuggestVisitor::~ShowSuggestVisitor()
{
}

void
ShowSuggestVisitor::visit_sequence(const ConditionalDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    Save<std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    if (a.condition_met())
    {
        _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
        RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

        std::for_each(cur, end, accept_visitor(*this));
    }
}

void
ShowSuggestVisitor::visit_sequence(const AnyDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    Save<std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
    RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::for_each(cur, end, accept_visitor(*this));
}

void
ShowSuggestVisitor::visit_sequence(const AllDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    _imp->labels.push_front(make_shared_ptr(new ActiveDependencyLabels(**_imp->labels.begin())));
    RunOnDestruction restore_labels(std::tr1::bind(std::tr1::mem_fn(&LabelsStack::pop_front), &_imp->labels));

    std::for_each(cur, end, accept_visitor(*this));
}

void
ShowSuggestVisitor::visit_leaf(const BlockDepSpec &)
{
}

namespace
{
    struct SuggestActiveVisitor :
        ConstVisitor<DependencySuggestLabelVisitorTypes>
    {
        bool result;

        SuggestActiveVisitor() :
            result(false)
        {
        }

        void visit(const DependencyRecommendedLabel &)
        {
        }

        void visit(const DependencySuggestedLabel &)
        {
            result = true;
        }

        void visit(const DependencyRequiredLabel &)
        {
        }
    };
}

void
ShowSuggestVisitor::visit_leaf(const PackageDepSpec & a)
{
    Context context("When adding suggested dep '" + stringify(a) + "':");

    if (_imp->only_if_suggested_label)
    {
        SuggestActiveVisitor v;
        for (DependencySuggestLabelSequence::ConstIterator
                i((*_imp->labels.begin())->suggest_labels()->begin()),
                i_end((*_imp->labels.begin())->suggest_labels()->end()) ;
                i != i_end ; ++i)
            (*i)->accept(v);

        if (! v.result)
        {
            Log::get_instance()->message("dep_list.show_suggest_visitor.skipping_suggested", ll_debug, lc_context)
                << "Skipping dep '" << a << "' because no suggested label is active";
            return;
        }
    }

    Save<std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    std::tr1::shared_ptr<const PackageIDSequence> installed_matches(_imp->environment->package_database()->query(
                query::SupportsAction<InstalledAction>() &
                query::Matches(a), qo_order_by_version));
    if (! installed_matches->empty())
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.already_installed", ll_debug, lc_context)
            << "Suggestion '" << a << "' already matched by installed packages '"
            << join(indirect_iterator(installed_matches->begin()), indirect_iterator(installed_matches->end()), ", ")
            << "', not suggesting it";
        return;
    }

    std::tr1::shared_ptr<const PackageIDSequence> matches(_imp->environment->package_database()->query(
                query::SupportsAction<InstallAction>() &
                query::Matches(a), qo_order_by_version));
    if (matches->empty())
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.nothing_found", ll_warning, lc_context)
            << "Nothing found for '" << a << "'";
        return;
    }

    for (PackageIDSequence::ConstIterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        if ((*m)->masked())
            continue;

        _imp->dep_list->add_suggested_package(*m, a, _imp->conditions, _imp->destinations);
        return;
    }

    Log::get_instance()->message("dep_list.show_suggest_visitor.nothing_visible_found", ll_warning, lc_context)
        << "Nothing visible found for '" << a << "'";
}

void
ShowSuggestVisitor::visit_leaf(const DependencyLabelsDepSpec & spec)
{
    _imp->labels.begin()->reset(new ActiveDependencyLabels(**_imp->labels.begin(), spec));
}

void
ShowSuggestVisitor::visit_leaf(const NamedSetDepSpec & s)
{
    Context context("When expanding named set '" + stringify(s) + "':");

    std::tr1::shared_ptr<const SetSpecTree::ConstItem> set(_imp->environment->set(s.name()));

    if (! set)
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.unknown_set", ll_warning, lc_context)
            << "Unknown set '" << s.name() << "'";
        return;
    }

    if (! _imp->recursing_sets.insert(s.name()).second)
    {
        Log::get_instance()->message("dep_list.show_suggest_visitor.recursive_set", ll_warning, lc_context)
            << "Recursively defined set '" << s.name() << "'";
        return;
    }

    set->accept(*this);

    _imp->recursing_sets.erase(s.name());
}

