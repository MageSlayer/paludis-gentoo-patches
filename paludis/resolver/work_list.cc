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

#include <paludis/resolver/work_list.hh>
#include <paludis/resolver/work_item.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/serialise-impl.hh>
#include <vector>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename WorkItem_>
    struct Implementation<WorkList<WorkItem_> >
    {
        std::vector<std::tr1::shared_ptr<WorkItem_> > list;
    };

#ifdef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename WorkItem_>
    struct WrappedForwardIteratorTraits<WorkListConstIteratorTag<WorkItem_> >
    {
        typedef typename std::vector<std::tr1::shared_ptr<WorkItem_> >::const_iterator UnderlyingIterator;
    };
}

template <typename WorkItem_>
WorkList<WorkItem_>::WorkList() :
    PrivateImplementationPattern<WorkList<WorkItem_> >(new Implementation<WorkList<WorkItem_> >)
{
}

template <typename WorkItem_>
WorkList<WorkItem_>::~WorkList()
{
}

template <typename WorkItem_>
WorkListIndex
WorkList<WorkItem_>::append(const std::tr1::shared_ptr<WorkItem_> & i)
{
    typename std::vector<std::tr1::shared_ptr<WorkItem_> >::const_iterator p(_imp->list.insert(_imp->list.end(), i));
    return p - _imp->list.begin();
}

template <typename WorkItem_>
int
WorkList<WorkItem_>::length() const
{
    return _imp->list.size();
}

template <typename WorkItem_>
typename WorkList<WorkItem_>::ConstIterator
WorkList<WorkItem_>::begin() const
{
    return ConstIterator(_imp->list.begin());
}

template <typename WorkItem_>
typename WorkList<WorkItem_>::ConstIterator
WorkList<WorkItem_>::end() const
{
    return ConstIterator(_imp->list.end());
}

template <typename WorkItem_>
const std::tr1::shared_ptr<WorkList<WorkItem_> >
WorkList<WorkItem_>::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "WorkList");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<WorkList> result(new WorkList);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->append(vv.member<std::tr1::shared_ptr<WorkItem_> >(stringify(n)));
    return result;
}

template <typename WorkItem_>
void
WorkList<WorkItem_>::serialise(Serialiser & s) const
{
    s.object("WorkList")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->list)
        ;
}

template class WorkList<PretendWorkItem>;
template class WrappedForwardIterator<WorkListConstIteratorTag<PretendWorkItem>, const std::tr1::shared_ptr<PretendWorkItem> >;

template class WorkList<ExecuteWorkItem>;
template class WrappedForwardIterator<WorkListConstIteratorTag<ExecuteWorkItem>, const std::tr1::shared_ptr<ExecuteWorkItem> >;

