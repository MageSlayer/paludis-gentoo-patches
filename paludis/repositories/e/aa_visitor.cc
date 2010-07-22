/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <list>
#include <algorithm>

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

    template <>
    struct WrappedForwardIteratorTraits<AAVisitor::ConstIteratorTag>
    {
        typedef std::list<std::string>::const_iterator UnderlyingIterator;
    };
}

AAVisitor::AAVisitor() :
    PrivateImplementationPattern<AAVisitor>()
{
}

AAVisitor::~AAVisitor()
{
}

void
AAVisitor::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    _imp->aa.push_back(node.spec()->filename());
}


void
AAVisitor::visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type &)
{
}

void
AAVisitor::visit(const FetchableURISpecTree::BasicInnerNode & node)
{
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
}

AAVisitor::ConstIterator
AAVisitor::begin() const
{
    return ConstIterator(_imp->aa.begin());
}

AAVisitor::ConstIterator
AAVisitor::end() const
{
    return ConstIterator(_imp->aa.end());
}

template class WrappedForwardIterator<AAVisitor::ConstIteratorTag, const std::string>;

