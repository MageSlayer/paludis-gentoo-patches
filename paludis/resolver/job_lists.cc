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

#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
JobLists::serialise(Serialiser & s) const
{
    s.object("JobLists")
        .member(SerialiserFlags<serialise::might_be_null>(), "execute_job_list", execute_job_list())
        .member(SerialiserFlags<serialise::might_be_null>(), "pretend_job_list", pretend_job_list())
        ;
}

const std::shared_ptr<JobLists>
JobLists::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobLists");

    return make_shared_copy(make_named_values<JobLists>(
            n::execute_job_list() = v.member<std::shared_ptr<JobList<ExecuteJob> > >("execute_job_list"),
            n::pretend_job_list() = v.member<std::shared_ptr<JobList<PretendJob> > >("pretend_job_list")
            ));
}

