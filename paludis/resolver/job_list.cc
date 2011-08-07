/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/serialise-impl.hh>
#include <vector>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <typename Job_>
    struct Imp<JobList<Job_> >
    {
        std::vector<std::shared_ptr<Job_> > list;
    };

    template <typename Job_>
    struct WrappedForwardIteratorTraits<JobListConstIteratorTag<Job_> >
    {
        typedef typename std::vector<std::shared_ptr<Job_> >::const_iterator UnderlyingIterator;
    };
}

template <typename Job_>
JobList<Job_>::JobList() :
    _imp()
{
}

template <typename Job_>
JobList<Job_>::~JobList()
{
}

template <typename Job_>
JobNumber
JobList<Job_>::append(const std::shared_ptr<Job_> & i)
{
    typename std::vector<std::shared_ptr<Job_> >::const_iterator p(_imp->list.insert(_imp->list.end(), i));
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
typename JobList<Job_>::ConstIterator
JobList<Job_>::fetch(const JobNumber n) const
{
    if (n < 0 || n >= JobNumber(_imp->list.size()))
        throw InternalError(PALUDIS_HERE, "n is " + stringify(n) + " but size is " + stringify(_imp->list.size()));
    return ConstIterator(_imp->list.begin() + n);
}

template <typename Job_>
JobNumber
JobList<Job_>::number(const ConstIterator & i) const
{
    return i.template underlying_iterator<typename WrappedForwardIteratorTraits<ConstIteratorTag>::UnderlyingIterator>() - _imp->list.begin();
}

template <typename Job_>
const std::shared_ptr<JobList<Job_> >
JobList<Job_>::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobList");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::shared_ptr<JobList> result(std::make_shared<JobList>());
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->append(vv.member<std::shared_ptr<Job_> >(stringify(n)));
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

namespace paludis
{
    template class WrappedForwardIterator<JobListConstIteratorTag<PretendJob>, const std::shared_ptr<PretendJob> >;
    template class WrappedForwardIterator<JobListConstIteratorTag<ExecuteJob>, const std::shared_ptr<ExecuteJob> >;

    namespace resolver {
        template class JobList<PretendJob>;
        template class JobList<ExecuteJob>;
    }
}
