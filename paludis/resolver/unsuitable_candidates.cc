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

#include <paludis/resolver/unsuitable_candidates.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

template class Sequence<UnsuitableCandidate>;
template class WrappedForwardIterator<UnsuitableCandidates::ConstIteratorTag, const UnsuitableCandidate>;

UnsuitableCandidate
UnsuitableCandidate::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "UnsuitableCandidate");
    return make_named_values<UnsuitableCandidate>(
            n::package_id() = v.member<std::shared_ptr<const PackageID> >("package_id"),
            n::unmet_constraints() = v.member<std::shared_ptr<Constraints> >("unmet_constraints")
            );
}

void
UnsuitableCandidate::serialise(Serialiser & s) const
{
    s.object("UnsuitableCandidate")
        .member(SerialiserFlags<serialise::might_be_null>(), "package_id", package_id())
        .member(SerialiserFlags<>(), "unmet_constraints", *unmet_constraints())
        ;
}

