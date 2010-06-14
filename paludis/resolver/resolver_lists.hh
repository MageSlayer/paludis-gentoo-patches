/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_LISTS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_LISTS_HH 1

#include <paludis/resolver/resolver_lists-fwd.hh>
#include <paludis/resolver/job_id-fwd.hh>
#include <paludis/resolver/jobs-fwd.hh>
#include <paludis/resolver/resolutions_by_resolvent-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct job_ids_needing_confirmation_name> job_ids_needing_confirmation;
        typedef Name<struct jobs_name> jobs;
        typedef Name<struct resolutions_by_resolvent_name> resolutions_by_resolvent;
        typedef Name<struct taken_error_job_ids_name> taken_error_job_ids;
        typedef Name<struct taken_job_ids_name> taken_job_ids;
        typedef Name<struct untaken_error_job_ids_name> untaken_error_job_ids;
        typedef Name<struct untaken_job_ids_name> untaken_job_ids;
    }

    namespace resolver
    {
        struct ResolverLists
        {
            NamedValue<n::job_ids_needing_confirmation, std::tr1::shared_ptr<JobIDSequence> > job_ids_needing_confirmation;
            NamedValue<n::jobs, std::tr1::shared_ptr<Jobs> > jobs;
            NamedValue<n::resolutions_by_resolvent, std::tr1::shared_ptr<ResolutionsByResolvent> > resolutions_by_resolvent;
            NamedValue<n::taken_error_job_ids, std::tr1::shared_ptr<JobIDSequence> > taken_error_job_ids;
            NamedValue<n::taken_job_ids, std::tr1::shared_ptr<JobIDSequence> > taken_job_ids;
            NamedValue<n::untaken_error_job_ids, std::tr1::shared_ptr<JobIDSequence> > untaken_error_job_ids;
            NamedValue<n::untaken_job_ids, std::tr1::shared_ptr<JobIDSequence> > untaken_job_ids;

            static const ResolverLists deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
