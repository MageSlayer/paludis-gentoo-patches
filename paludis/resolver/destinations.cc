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

#include <paludis/resolver/destinations.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/package_id.hh>
#include <ostream>
#include <sstream>
#include <vector>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Destinations>
    {
        std::vector<std::tr1::shared_ptr<const Destination> > destinations;

        Implementation() :
            destinations(static_cast<int>(last_dt), make_null_shared_ptr())
        {
        }
    };
}

Destinations::Destinations() :
    PrivateImplementationPattern<Destinations>(new Implementation<Destinations>)
{
}

Destinations::~Destinations()
{
}

const std::tr1::shared_ptr<const Destination>
Destinations::by_type(const DestinationType t) const
{
    return _imp->destinations.at(static_cast<int>(t));
}

void
Destinations::set_destination_type(const DestinationType t, const std::tr1::shared_ptr<const Destination> & d)
{
    _imp->destinations.at(t) = d;
}

void
Destinations::serialise(Serialiser & s) const
{
    SerialiserObjectWriter w(s.object("Destinations"));
    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        w.member(SerialiserFlags<serialise::might_be_null>(), stringify(*t), by_type(*t));
}

void
Destination::serialise(Serialiser & s) const
{
    s.object("Destination")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "replacing", replacing())
        .member(SerialiserFlags<>(), "repository", stringify(repository()))
        ;
}

const std::tr1::shared_ptr<Destinations>
Destinations::deserialise(Deserialisation & d)
{
    const std::tr1::shared_ptr<Destinations> result(new Destinations);
    Deserialisator v(d, "Destinations");

    for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
        result->set_destination_type(*t, v.member<std::tr1::shared_ptr<Destination> >(stringify(*t)));

    return result;
}

const std::tr1::shared_ptr<Destination>
Destination::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Destination");

    std::tr1::shared_ptr<PackageIDSequence> replacing(new PackageIDSequence);
    Deserialisator vv(*v.find_remove_member("replacing"), "c");
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        replacing->push_back(vv.member<std::tr1::shared_ptr<const PackageID> >(stringify(n)));

    return make_shared_ptr(new Destination(make_named_values<Destination>(
                    value_for<n::replacing>(replacing),
                    value_for<n::repository>(RepositoryName(v.member<std::string>("repository")))
                    )));
}

template class PrivateImplementationPattern<Destinations>;

