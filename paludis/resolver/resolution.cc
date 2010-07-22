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

#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/serialise-impl.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

void
Resolution::serialise(Serialiser & s) const
{
    s.object("Resolution")
        .member(SerialiserFlags<>(), "constraints", *constraints())
        .member(SerialiserFlags<serialise::might_be_null>(), "decision", decision())
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        ;
}

const std::shared_ptr<Resolution>
Resolution::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolution");

    return make_shared_ptr(new Resolution(make_named_values<Resolution>(
                    n::constraints() = v.member<std::shared_ptr<Constraints> >("constraints"),
                    n::decision() = v.member<std::shared_ptr<Decision> >("decision"),
                    n::resolvent() = v.member<Resolvent>("resolvent")
                    )));
}

