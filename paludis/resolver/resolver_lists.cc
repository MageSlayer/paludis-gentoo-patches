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

#include <paludis/resolver/resolver_lists.hh>
#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/jobs.hh>
#include <paludis/resolver/job_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
ResolverLists::serialise(Serialiser & s) const
{
    s.object("ResolverLists")
        .member(SerialiserFlags<serialise::might_be_null>(), "all_resolutions", all_resolutions())
        .member(SerialiserFlags<serialise::might_be_null>(), "jobs", jobs())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "job_ids_needing_confirmation", job_ids_needing_confirmation())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "taken_error_job_ids", taken_error_job_ids())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "taken_job_ids", taken_job_ids())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "untaken_error_job_ids", untaken_error_job_ids())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "untaken_job_ids", untaken_job_ids())
        ;
}

const ResolverLists
ResolverLists::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ResolverLists");

    std::tr1::shared_ptr<JobIDSequence> job_ids_needing_confirmation(new JobIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("job_ids_needing_confirmation"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            job_ids_needing_confirmation->push_back(vv.member<JobID>(stringify(n)));
    }

    std::tr1::shared_ptr<JobIDSequence> taken_error_job_ids(new JobIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("taken_error_job_ids"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            taken_error_job_ids->push_back(vv.member<JobID>(stringify(n)));
    }

    std::tr1::shared_ptr<JobIDSequence> taken_job_ids(new JobIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("taken_job_ids"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            taken_job_ids->push_back(vv.member<JobID>(stringify(n)));
    }

    std::tr1::shared_ptr<JobIDSequence> untaken_error_job_ids(new JobIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("untaken_error_job_ids"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            untaken_error_job_ids->push_back(vv.member<JobID>(stringify(n)));
    }

    std::tr1::shared_ptr<JobIDSequence> untaken_job_ids(new JobIDSequence);
    {
        Deserialisator vv(*v.find_remove_member("untaken_job_ids"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            untaken_job_ids->push_back(vv.member<JobID>(stringify(n)));
    }

    return make_named_values<ResolverLists>(
            value_for<n::all_resolutions>(v.member<std::tr1::shared_ptr<Resolutions> >("all_resolutions")),
            value_for<n::job_ids_needing_confirmation>(job_ids_needing_confirmation),
            value_for<n::jobs>(v.member<std::tr1::shared_ptr<Jobs> >("jobs")),
            value_for<n::taken_error_job_ids>(taken_error_job_ids),
            value_for<n::taken_job_ids>(taken_job_ids),
            value_for<n::untaken_error_job_ids>(untaken_error_job_ids),
            value_for<n::untaken_job_ids>(untaken_job_ids)
            );
}

