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

#include <paludis/resolver/destination.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

const std::tr1::shared_ptr<Destination>
Destination::deserialise(Deserialisation & d)

{
    Deserialisator v(d, "Destination");
    std::tr1::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
    Deserialisator vv(*v.find_remove_member("replacing"), "c");
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        replacing->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));


    return make_shared_ptr(new Destination(make_named_values<Destination>(
                    n::replacing() = replacing,
                    n::repository() = RepositoryName(v.member<std::string>("repository"))
                    )));
}

void
Destination::serialise(Serialiser & s) const
{
    s.object("Destination")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "replacing", replacing())
        .member(SerialiserFlags<>(), "repository", stringify(repository()))
        ;
}

