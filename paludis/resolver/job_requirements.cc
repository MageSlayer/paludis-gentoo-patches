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

#include <paludis/resolver/job_requirements.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>
#include <istream>
#include <ostream>

using namespace paludis;
using namespace paludis::resolver;

#include <paludis/resolver/job_requirements-se.cc>

const JobRequirement
JobRequirement::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobRequirement");
    return make_named_values<JobRequirement>(
            n::job_number() = v.member<JobNumber>("job_number"),
            n::required_if() = v.member<JobRequirementIfs>("required_if")
            );
}

void
JobRequirement::serialise(Serialiser & s) const
{
    s.object("JobRequirement")
        .member(SerialiserFlags<>(), "job_number", job_number())
        .member(SerialiserFlags<>(), "required_if", required_if())
        ;
}

template class Sequence<JobRequirement>;
template class WrappedForwardIterator<Sequence<JobRequirement>::ConstIteratorTag, const JobRequirement>;

