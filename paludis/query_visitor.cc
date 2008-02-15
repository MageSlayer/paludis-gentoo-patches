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

#include <paludis/query_visitor.hh>
#include <paludis/dep_list.hh>
#include <paludis/range_rewriter.hh>
#include <paludis/package_database.hh>
#include <paludis/query.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor-impl.hh>
#include <algorithm>
#include <set>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<QueryVisitor>
    {
        bool result;
        const DepList * const dep_list;
        tr1::shared_ptr<const DestinationsSet> destinations;
        const Environment * const environment;
        const tr1::shared_ptr<const PackageID> id;
        std::set<SetName> recursing_sets;

        Implementation(const DepList * const d, tr1::shared_ptr<const DestinationsSet> dd,
                const Environment * const e, const tr1::shared_ptr<const PackageID> & p) :
            result(true),
            dep_list(d),
            destinations(dd),
            environment(e),
            id(p)
        {
        }
    };
}

QueryVisitor::QueryVisitor(const DepList * const d, tr1::shared_ptr<const DestinationsSet> dd,
        const Environment * const e, const tr1::shared_ptr<const PackageID> & id) :
    PrivateImplementationPattern<QueryVisitor>(new Implementation<QueryVisitor>(d, dd, e, id))
{
}

QueryVisitor::~QueryVisitor()
{
}

bool
QueryVisitor::result() const
{
    return _imp->result;
}

void
QueryVisitor::visit_leaf(const PackageDepSpec & a)
{
    using namespace tr1::placeholders;

    /* a pda matches if we'll be installed by the time we reach the current point. This
     * means that merely being installed is not enough, if we'll have our version changed
     * by something in the merge list. */

    _imp->result = false;

    // TODO: check destinations
    tr1::shared_ptr<const PackageIDSequence> matches(_imp->environment->package_database()->query(
                query::InstalledAtRoot(_imp->environment->root()) & query::Matches(a), qo_whatever));

    if (indirect_iterator(matches->end()) != std::find_if(indirect_iterator(matches->begin()), indirect_iterator(matches->end()),
                tr1::bind(tr1::mem_fn(&DepList::replaced), _imp->dep_list, _1)))
    {
        _imp->result = true;
        return;
    }

    /* check the merge list for any new packages that match */
    if (_imp->dep_list->match_on_list(a))
    {
        _imp->result = true;
        return;
    }
}

void
QueryVisitor::visit_leaf(const NamedSetDepSpec & s)
{
    Context context("When expanding named set '" + stringify(s) + "':");

    tr1::shared_ptr<const SetSpecTree::ConstItem> set(_imp->environment->set(s.name()));

    if (! set)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Unknown set '" << s.name() << "'";
        _imp->result = false;
        return;
    }

    if (! _imp->recursing_sets.insert(s.name()).second)
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Recursively defined set '" << s.name() << "'";
        return;
    }

    set->accept(*this);

    _imp->recursing_sets.erase(s.name());
}

void
QueryVisitor::visit_sequence(const ConditionalDepSpec & a,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    /* for use? ( ) dep specs, return true if we're not enabled, so that
     * weird || ( ) cases work. */
    if (a.condition_met())
    {
        _imp->result = true;
        for ( ; cur != end ; ++cur)
        {
            cur->accept(*this);
            if (! _imp->result)
                return;
        }
    }
    else
        _imp->result = true;
}

void
QueryVisitor::visit_sequence(const AnyDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    /* empty || ( ) must resolve to true */
    _imp->result = true;

    RangeRewriter r;
    std::for_each(cur, end, accept_visitor(r));

    if (r.spec())
        visit_leaf(*r.spec());
    else
        for ( ; cur != end ; ++cur)
        {
            if (! is_viable_any_child(*cur))
                continue;

            cur->accept(*this);
            if (_imp->result)
                return;
        }
}

void
QueryVisitor::visit_leaf(const BlockDepSpec & a)
{
    visit_leaf(*a.blocked_spec());
    _imp->result = !_imp->result;
}

void
QueryVisitor::visit_sequence(const AllDepSpec &,
        DependencySpecTree::ConstSequenceIterator cur,
        DependencySpecTree::ConstSequenceIterator end)
{
    for ( ; cur != end ; ++cur)
    {
        cur->accept(*this);
        if (! _imp->result)
            return;
    }
}

void
QueryVisitor::visit_leaf(const DependencyLabelsDepSpec &)
{
    // XXX implement
}

