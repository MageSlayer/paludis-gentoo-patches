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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_LIST_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_LIST_HH 1

#include <paludis/resolver/job_list-fwd.hh>
#include <paludis/resolver/job-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        template <typename Job_>
        struct JobListConstIteratorTag;

        template <typename Job_>
        class PALUDIS_VISIBLE JobList :
            private Pimp<JobList<Job_> >
        {
            private:
                using Pimp<JobList<Job_> >::_imp;

            public:
                JobList();
                ~JobList();

                JobNumber append(const std::shared_ptr<Job_> &);

                int length() const PALUDIS_ATTRIBUTE((warn_unused_result));

                typedef JobListConstIteratorTag<Job_> ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::shared_ptr<Job_> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator fetch(const JobNumber) const PALUDIS_ATTRIBUTE((warn_unused_result));

                JobNumber number(const ConstIterator &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::shared_ptr<JobList<Job_> > deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                void serialise(Serialiser &) const;
        };

        extern template class JobList<PretendJob>;
        extern template class JobList<ExecuteJob>;
    }

    extern template class WrappedForwardIterator<resolver::JobListConstIteratorTag<resolver::PretendJob>,
           const std::shared_ptr<resolver::PretendJob> >;
    extern template class WrappedForwardIterator<resolver::JobListConstIteratorTag<resolver::ExecuteJob>,
           const std::shared_ptr<resolver::ExecuteJob> >;
}

#endif
