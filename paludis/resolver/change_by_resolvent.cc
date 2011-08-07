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

#include <paludis/resolver/change_by_resolvent.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
ChangeByResolvent::serialise(Serialiser & s) const
{
    s.object("ChangeByResolvent")
        .member(SerialiserFlags<serialise::might_be_null>(), "package_id", package_id())
        .member(SerialiserFlags<>(), "resolvent", resolvent())
        ;
}

const ChangeByResolvent
ChangeByResolvent::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ChangeByResolvent");
    return make_named_values<ChangeByResolvent>(
            n::package_id() = v.member<std::shared_ptr<const PackageID> >("package_id"),
            n::resolvent() = v.member<Resolvent>("resolvent")
            );
}

namespace paludis
{
    template class Sequence<ChangeByResolvent>;
    template class WrappedForwardIterator<Sequence<ChangeByResolvent>::ConstIteratorTag, const ChangeByResolvent>;
}
