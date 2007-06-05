/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#include <paludis/dep_list/condition_tracker.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/dep_spec.hh>

using namespace paludis;
using namespace tr1::placeholders;

ConditionTracker::ConditionTracker(
            tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > conditions) :
    base(new ConstTreeSequence<DependencySpecTree, AllDepSpec>(
             tr1::shared_ptr<AllDepSpec>(new AllDepSpec))),
    adder(tr1::bind(&ConstTreeSequence<DependencySpecTree, AllDepSpec>::add, base, _1))
{
    conditions->accept(*this);
}

ConditionTracker::~ConditionTracker()
{
}

template <typename T_>
tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::do_add_sequence(const T_ & node)
{
    adder(tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, T_> >(
              new ConstTreeSequence<DependencySpecTree, T_>(
                  tr1::static_pointer_cast<T_>(node.clone()))));
    return base;
}

template <typename T_>
tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::do_add_leaf(const T_ & node)
{
    adder(tr1::shared_ptr<TreeLeaf<DependencySpecTree, T_> >(
              new TreeLeaf<DependencySpecTree, T_>(
                  tr1::static_pointer_cast<T_>(node.clone()))));
    return base;
}

tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::add_condition(const AnyDepSpec & any)
{
    return do_add_sequence(any);
}

tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::add_condition(const UseDepSpec & use)
{
    return do_add_sequence(use);
}

tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::add_condition(const PackageDepSpec & pkg)
{
    return do_add_leaf(pkg);
}

tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> >
ConditionTracker::add_condition(const BlockDepSpec & block)
{
    return do_add_leaf(block);
}

template <typename T_>
void
ConditionTracker::do_visit_sequence(const T_ & node,
           DependencySpecTree::ConstSequenceIterator begin,
           DependencySpecTree::ConstSequenceIterator end)
{
    tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, T_> > a(
        new ConstTreeSequence<DependencySpecTree, T_>(
            tr1::static_pointer_cast<T_>(node.clone())));
    adder(a);
    adder = tr1::bind(&ConstTreeSequence<DependencySpecTree, T_>::add, a, _1);
    if (begin != end)
    {
        begin->accept(*this);
        if (++begin != end)
            throw InternalError(PALUDIS_HERE, "ConditionTracker saw a sequence with more than one element");
    }
}

void
ConditionTracker::visit_sequence(const AnyDepSpec & node,
            DependencySpecTree::ConstSequenceIterator begin,
            DependencySpecTree::ConstSequenceIterator end)
{
    do_visit_sequence(node, begin, end);
}

void
ConditionTracker::visit_sequence(const UseDepSpec & node,
            DependencySpecTree::ConstSequenceIterator begin,
            DependencySpecTree::ConstSequenceIterator end)
{
    do_visit_sequence(node, begin, end);
}

void
ConditionTracker::visit_leaf(const PackageDepSpec &)
{
    throw InternalError(PALUDIS_HERE, "ConditionTracker saw a PackageDepSpec");
}

void
ConditionTracker::visit_leaf(const BlockDepSpec &)
{
    throw InternalError(PALUDIS_HERE, "ConditionTracker saw a BlockDepSpec");
}

