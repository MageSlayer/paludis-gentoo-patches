/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>

using namespace paludis;
using namespace paludis::resolver;

void
Arrow::serialise(Serialiser & s) const
{
    s.object("Arrow")
        .member(SerialiserFlags<>(), "comes_after", comes_after())
        .member(SerialiserFlags<>(), "ignorable_pass", ignorable_pass())
        .member(SerialiserFlags<serialise::might_be_null>(), "reason", reason())
        ;
}

const std::tr1::shared_ptr<Arrow>
Arrow::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Arrow");
    return make_shared_ptr(new Arrow(make_named_values<Arrow>(
                    value_for<n::comes_after>(v.member<Resolvent>("comes_after")),
                    value_for<n::ignorable_pass>(v.member<bool>("ignorable_pass")),
                    value_for<n::reason>(v.member<std::tr1::shared_ptr<const Reason> >("reason"))
                    )));
}

template class Sequence<std::tr1::shared_ptr<Arrow> >;
template class WrappedForwardIterator<ArrowSequence::ConstIteratorTag, const std::tr1::shared_ptr<Arrow> >;

