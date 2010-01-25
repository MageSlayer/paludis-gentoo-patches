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

#include <paludis/resolver/job_id.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::operator== (const JobID & a, const JobID & b)
{
    return a.string_id() == b.string_id();
}

std::size_t
JobID::hash() const
{
    return Hash<std::string>()(string_id());
}

bool
JobID::operator< (const JobID & other) const
{
    return string_id() < other.string_id();
}

void
JobID::serialise(Serialiser & s) const
{
    s.object("JobID")
        .member(SerialiserFlags<>(), "string_id", string_id())
        ;
}

const JobID
JobID::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "JobID");
    return make_named_values<JobID>(
            value_for<n::string_id>(v.member<std::string>("string_id"))
            );
}

template class Sequence<JobID>;
template class WrappedForwardIterator<JobIDSequence::ConstIteratorTag, const JobID>;

