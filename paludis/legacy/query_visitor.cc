/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/legacy/query_visitor.hh>
#include <paludis/legacy/dep_list.hh>
#include <paludis/legacy/range_rewriter.hh>
#include <paludis/package_database.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <functional>
#include <algorithm>
#include <set>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<QueryVisitor>
    {
        bool result;
        const DepList * const dep_list;
        std::shared_ptr<const DestinationsSet> destinations;
        const Environment * const environment;
        const std::shared_ptr<const PackageID> id;
        std::set<SetName> recursing_sets;

        Imp(const DepList * const d, std::shared_ptr<const DestinationsSet> dd,
                const Environment * const e, const std::shared_ptr<const PackageID> & p) :
            result(true),
            dep_list(d),
            destinations(dd),
            environment(e),
            id(p)
        {
        }
    };
}

QueryVisitor::QueryVisitor(const DepList * const d, const std::shared_ptr<const DestinationsSet> & dd,
        const Environment * const e, const std::shared_ptr<const PackageID> & id) :
    Pimp<QueryVisitor>(d, dd, e, id)
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
QueryVisitor::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
{
    using namespace std::placeholders;

    /* a pda matches if we'll be installed by the time we reach the current point. This
     * means that merely being installed is not enough, if we'll have our version changed
     * by something in the merge list. */

    _imp->result = false;

    // TODO: check destinations
    std::shared_ptr<const PackageIDSequence> matches((*_imp->environment)[selection::AllVersionsUnsorted(
                generator::Matches(*node.spec(), _imp->dep_list->options()->match_package_options()) |
                filter::InstalledAtRoot(_imp->environment->preferred_root_key()->value()))]);

    if (indirect_iterator(matches->end()) != std::find_if(indirect_iterator(matches->begin()), indirect_iterator(matches->end()),
                std::bind(std::logical_not<bool>(), std::bind(std::mem_fn(&DepList::replaced), _imp->dep_list, _1))))
    {
        _imp->result = true;
        return;
    }

    /* check the merge list for any new packages that match */
    if (_imp->dep_list->match_on_list(*node.spec()))
    {
        _imp->result = true;
        return;
    }
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
{
    Context context("When expanding named set '" + stringify(*node.spec()) + "':");

    std::shared_ptr<const SetSpecTree> set(_imp->environment->set(node.spec()->name()));

    if (! set)
    {
        Log::get_instance()->message("dep_list.query_visitor.unknown_set", ll_warning, lc_context) << "Unknown set '" << node.spec()->name() << "'";
        _imp->result = false;
        return;
    }

    if (! _imp->recursing_sets.insert(node.spec()->name()).second)
    {
        Log::get_instance()->message("dep_list.query_visitor.recursive_set", ll_warning, lc_context)
            << "Recursively defined set '" << node.spec()->name() << "'";
        return;
    }

    set->top()->accept(*this);

    _imp->recursing_sets.erase(node.spec()->name());
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    /* for use? ( ) dep specs, return true if we're not enabled, so that
     * weird || ( ) cases work. */
    if (node.spec()->condition_met(_imp->environment, _imp->id))
    {
        _imp->result = true;
        for (DependencySpecTree::NodeType<AnyDepSpec>::Type::ConstIterator c(node.begin()), c_end(node.end()) ;
                c != c_end ; ++c)
        {
            (*c)->accept(*this);
            if (! _imp->result)
                return;
        }
    }
    else
        _imp->result = true;
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
{
    /* empty || ( ) must resolve to true */
    _imp->result = true;

    RangeRewriter r;
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(r));

    if (r.spec())
    {
        DependencySpecTree tree(std::make_shared<AllDepSpec>());
        tree.top()->append(r.spec());
        tree.top()->accept(*this);
    }
    else
        for (DependencySpecTree::NodeType<AnyDepSpec>::Type::ConstIterator c(node.begin()), c_end(node.end()) ;
                c != c_end ; ++c)
        {
            if (! is_viable_any_child(_imp->environment, _imp->id, **c))
                continue;

            (*c)->accept(*this);
            if (_imp->result)
                return;
        }
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type & node)
{
    DependencySpecTree tree(std::make_shared<AllDepSpec>());
    tree.top()->append(std::static_pointer_cast<const PackageDepSpec>(node.spec()->blocking().clone()));
    tree.top()->accept(*this);
    _imp->result = !_imp->result;
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
{
    for (DependencySpecTree::NodeType<AnyDepSpec>::Type::ConstIterator c(node.begin()), c_end(node.end()) ;
            c != c_end ; ++c)
    {
        (*c)->accept(*this);
        if (! _imp->result)
            return;
    }
}

void
QueryVisitor::visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
{
    // XXX implement
}

