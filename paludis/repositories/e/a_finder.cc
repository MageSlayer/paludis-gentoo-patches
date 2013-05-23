/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/a_finder.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

AFinder::AFinder(const Environment * const e, const std::shared_ptr<const PackageID> & i) :
    env(e),
    id(i)
{
    _labels.push_back(0);
}

void
AFinder::visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
{
    _specs.push_back(std::make_pair(node.spec().get(), *_labels.begin()));
}

void
AFinder::visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
{
    *_labels.begin() = node.spec().get();
}

void
AFinder::visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
{
    _labels.push_front(*_labels.begin());
    std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    _labels.pop_front();
}

void
AFinder::visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
{
    if (node.spec()->condition_met(env, id))
    {
        _labels.push_front(*_labels.begin());
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        _labels.pop_front();
    }
}

AFinder::ConstIterator
AFinder::begin()
{
    return _specs.begin();
}

AFinder::ConstIterator
AFinder::end() const
{
    return _specs.end();
}

