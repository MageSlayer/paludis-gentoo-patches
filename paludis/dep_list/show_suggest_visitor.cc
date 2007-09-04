/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_list/show_suggest_visitor.hh>
#include <paludis/dep_list/dep_list.hh>
#include <paludis/dep_list/condition_tracker.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ShowSuggestVisitor>
    {
        DepList * const dep_list;
        tr1::shared_ptr<const DestinationsSet> destinations;
        const Environment * const environment;
        const tr1::shared_ptr<const PackageID> id;
        bool dependency_tags;
        tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions;

        Implementation(DepList * const d, tr1::shared_ptr<const DestinationsSet> dd,
                const Environment * const e, const tr1::shared_ptr<const PackageID> & p, bool t) :
            dep_list(d),
            destinations(dd),
            environment(e),
            id(p),
            dependency_tags(t),
            conditions(tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >(
                           new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
                               tr1::shared_ptr<AllDepSpec>(new AllDepSpec))))
        {
        }
    };
}

ShowSuggestVisitor::ShowSuggestVisitor(DepList * const d, tr1::shared_ptr<const DestinationsSet> dd,
        const Environment * const e, const tr1::shared_ptr<const PackageID> & p, bool t) :
    PrivateImplementationPattern<ShowSuggestVisitor>(new Implementation<ShowSuggestVisitor>(d, dd, e, p, t))
{
}

ShowSuggestVisitor::~ShowSuggestVisitor()
{
}

void
ShowSuggestVisitor::visit_sequence(const UseDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    if ((_imp->id ? _imp->environment->query_use(a.flag(), *_imp->id) : false) ^ a.inverse())
        std::for_each(cur, end, accept_visitor(*this));
}

void
ShowSuggestVisitor::visit_sequence(const AnyDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    std::for_each(cur, end, accept_visitor(*this));
}

void
ShowSuggestVisitor::visit_leaf(const BlockDepSpec &)
{
}

void
ShowSuggestVisitor::visit_leaf(const PackageDepSpec & a)
{
    Context context("When adding suggested dep '" + stringify(a) + "':");

    Save<tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > > save_c(
        &_imp->conditions, _imp->dependency_tags ?
        ConditionTracker(_imp->conditions).add_condition(a) : _imp->conditions);

    tr1::shared_ptr<const PackageIDSequence> matches(_imp->environment->package_database()->query(
                query::SupportsAction<InstallAction>() &
                query::Matches(a), qo_order_by_version));
    if (matches->empty())
    {
        Log::get_instance()->message(ll_warning, lc_context, "Nothing found for '" + stringify(a) + "'");
        return;
    }

    for (PackageIDSequence::Iterator m(matches->begin()), m_end(matches->end()) ;
            m != m_end ; ++m)
    {
        if ((*m)->masked())
            continue;

        _imp->dep_list->add_suggested_package(*m, a, _imp->conditions, _imp->destinations);
        return;
    }

    Log::get_instance()->message(ll_warning, lc_context, "Nothing visible found for '" + stringify(a) + "'");
}

