/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly <pioto@pioto.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

/** \file
 * Implementation of aa_visitor.hh
 *
 * \ingroup grpaavisitor
 */

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template<>
    struct Implementation<AAVisitor>
    {
        std::list<std::string> aa;
    };
}

AAVisitor::AAVisitor() :
    PrivateImplementationPattern<AAVisitor>(new Implementation<AAVisitor>)
{
}

AAVisitor::~AAVisitor()
{
}

void
AAVisitor::visit_sequence(const AllDepSpec &,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator e)
{
    std::for_each(cur, e, accept_visitor(*this));
}

void
AAVisitor::visit_sequence(const UseDepSpec &,
        URISpecTree::ConstSequenceIterator cur,
        URISpecTree::ConstSequenceIterator e)
{
    std::for_each(cur, e, accept_visitor(*this));
}

void
AAVisitor::visit_leaf(const URIDepSpec & p)
{
    _imp->aa.push_back(p.filename());
}

AAVisitor::Iterator
AAVisitor::begin() const
{
    return Iterator(_imp->aa.begin());
}

AAVisitor::Iterator
AAVisitor::end() const
{
    return Iterator(_imp->aa.end());
}

