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

#include <paludis/resolver/arrow.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
Arrow::serialise(Serialiser & s) const
{
    s.object("Arrow")
        .member(SerialiserFlags<>(), "comes_after", comes_after())
        .member(SerialiserFlags<>(), "failure_kinds", failure_kinds())
        .member(SerialiserFlags<serialise::might_be_null>(), "maybe_reason", maybe_reason())
        ;
}

const Arrow
Arrow::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Arrow");
    return make_named_values<Arrow>(
            n::comes_after() = v.member<JobID>("comes_after"),
            n::failure_kinds() = v.member<FailureKinds>("failure_kinds"),
            n::maybe_reason() = v.member<std::tr1::shared_ptr<const Reason> >("maybe_reason")
            );
}

template class Sequence<Arrow>;
template class WrappedForwardIterator<ArrowSequence::ConstIteratorTag, const Arrow>;

