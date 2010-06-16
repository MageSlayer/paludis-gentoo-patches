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

#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
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
    template <typename Job_>
    struct Implementation<JobList<Job_> >
    {
        std::vector<std::tr1::shared_ptr<Job_> > list;
    };

#ifdef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename Job_>
    struct WrappedForwardIteratorTraits<JobListConstIteratorTag<Job_> >
    {
        typedef typename std::vector<std::tr1::shared_ptr<Job_> >::const_iterator UnderlyingIterator;
    };
}

template <typename Job_>
JobList<Job_>::JobList() :
    PrivateImplementationPattern<JobList<Job_> >(new Implementation<JobList<Job_> >)
{
}

template <typename Job_>
JobList<Job_>::~JobList()
{
}

template <typename Job_>
JobListIndex
JobList<Job_>::append(const std::tr1::shared_ptr<Job_> & i)
{
    typename std::vector<std::tr1::shared_ptr<Job_> >::const_iterator p(_imp->list.insert(_imp->list.end(), i));
    return p - _imp->list.begin();
}

template <typename Job_>
int
JobList<Job_>::length() const
{
    return _imp->list.size();
}

template <typename Job_>
typename JobList<Job_>::ConstIterator
JobList<Job_>::begin() const
{
    return ConstIterator(_imp->list.begin());
}

template <typename Job_>
typename JobList<Job_>::ConstIterator
JobList<Job_>::end() const
{
    return ConstIterator(_imp->list.end());
}

template <typename Job_>
const std::tr1::shared_ptr<JobList<Job_> >
JobList<Job_>::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobList");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<JobList> result(new JobList);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->append(vv.member<std::tr1::shared_ptr<Job_> >(stringify(n)));
    return result;
}

template <typename Job_>
void
JobList<Job_>::serialise(Serialiser & s) const
{
    s.object("JobList")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->list)
        ;
}

template class JobList<PretendJob>;
template class WrappedForwardIterator<JobListConstIteratorTag<PretendJob>, const std::tr1::shared_ptr<PretendJob> >;

template class JobList<ExecuteJob>;
template class WrappedForwardIterator<JobListConstIteratorTag<ExecuteJob>, const std::tr1::shared_ptr<ExecuteJob> >;

