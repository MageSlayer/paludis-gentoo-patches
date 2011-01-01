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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_LISTS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_JOB_LISTS_HH 1

#include <paludis/resolver/job_lists-fwd.hh>
#include <paludis/resolver/job_list-fwd.hh>
#include <paludis/resolver/job-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_execute_job_list> execute_job_list;
        typedef Name<struct name_pretend_job_list> pretend_job_list;
    }

    namespace resolver
    {
        struct JobLists
        {
            NamedValue<n::execute_job_list, std::shared_ptr<JobList<ExecuteJob> > > execute_job_list;
            NamedValue<n::pretend_job_list, std::shared_ptr<JobList<PretendJob> > > pretend_job_list;

            static const std::shared_ptr<JobLists> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
