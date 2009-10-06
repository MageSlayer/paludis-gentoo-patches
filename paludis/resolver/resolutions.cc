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

#include <paludis/resolver/resolutions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<Resolutions>
    {
        Sequence<std::tr1::shared_ptr<const Resolution> > resolutions;
    };
}

Resolutions::Resolutions() :
    PrivateImplementationPattern<Resolutions>(new Implementation<Resolutions>)
{
}

Resolutions::~Resolutions()
{
}

void
Resolutions::append(const std::tr1::shared_ptr<const Resolution> & r)
{
    _imp->resolutions.push_back(r);
}

Resolutions::ConstIterator
Resolutions::begin() const
{
    return ConstIterator(_imp->resolutions.begin());
}

Resolutions::ConstIterator
Resolutions::end() const
{
    return ConstIterator(_imp->resolutions.end());
}

bool
Resolutions::empty() const
{
    return begin() == end();
}

void
Resolutions::serialise(Serialiser & s) const
{
    s.object("Resolutions")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->resolutions)
        ;
}

const std::tr1::shared_ptr<Resolutions>
Resolutions::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolutions");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::tr1::shared_ptr<Resolutions> result(new Resolutions);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->append(vv.member<std::tr1::shared_ptr<Resolution> >(stringify(n)));
    return result;
}

void
ResolutionLists::serialise(Serialiser & s) const
{
    s.object("ResolutionLists")
        .member(SerialiserFlags<serialise::might_be_null>(), "all", all())
        .member(SerialiserFlags<serialise::might_be_null>(), "errors", errors())
        .member(SerialiserFlags<serialise::might_be_null>(), "ordered", ordered())
        .member(SerialiserFlags<serialise::might_be_null>(), "untaken", untaken())
        ;
}

ResolutionLists
ResolutionLists::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ResolutionLists");
    return make_named_values<ResolutionLists>(
            value_for<n::all>(v.member<std::tr1::shared_ptr<Resolutions> >("all")),
            value_for<n::errors>(v.member<std::tr1::shared_ptr<Resolutions> >("errors")),
            value_for<n::ordered>(v.member<std::tr1::shared_ptr<Resolutions> >("ordered")),
            value_for<n::untaken>(v.member<std::tr1::shared_ptr<Resolutions> >("untaken"))
            );
}

template class PrivateImplementationPattern<Resolutions>;
template class WrappedForwardIterator<Resolutions::ConstIteratorTag, const std::tr1::shared_ptr<const Resolution> >;

