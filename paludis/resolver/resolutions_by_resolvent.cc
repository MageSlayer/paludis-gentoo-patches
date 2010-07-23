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

#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/serialise-impl.hh>
#include <list>
#include <unordered_map>

using namespace paludis;
using namespace paludis::resolver;

// we need iterators to remain valid after an insert, but we also need fast
// lookups.
typedef std::list<std::shared_ptr<Resolution> > ResolutionList;
typedef std::unordered_map<Resolvent, ResolutionList::const_iterator, Hash<Resolvent> > ResolutionListIndex;

namespace paludis
{
    template <>
    struct Imp<ResolutionsByResolvent>
    {
        ResolutionList resolution_list;
        ResolutionListIndex resolution_list_index;
    };

    template <>
    struct WrappedForwardIteratorTraits<ResolutionsByResolvent::ConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<Resolution> >::const_iterator UnderlyingIterator;
    };
}

ResolutionsByResolvent::ResolutionsByResolvent() :
    Pimp<ResolutionsByResolvent>()
{
}

ResolutionsByResolvent::~ResolutionsByResolvent()
{
}

ResolutionsByResolvent::ConstIterator
ResolutionsByResolvent::begin() const
{
    return ConstIterator(_imp->resolution_list.begin());
}

ResolutionsByResolvent::ConstIterator
ResolutionsByResolvent::end() const
{
    return ConstIterator(_imp->resolution_list.end());
}

ResolutionsByResolvent::ConstIterator
ResolutionsByResolvent::find(const Resolvent & r) const
{
    ResolutionListIndex::const_iterator x(_imp->resolution_list_index.find(r));
    if (x == _imp->resolution_list_index.end())
        return end();
    else
        return ConstIterator(x->second);
}

ResolutionsByResolvent::ConstIterator
ResolutionsByResolvent::insert_new(const std::shared_ptr<Resolution> & r)
{
    ResolutionList::iterator i(_imp->resolution_list.insert(_imp->resolution_list.end(), r));
    if (! _imp->resolution_list_index.insert(std::make_pair(r->resolvent(), i)).second)
    {
        _imp->resolution_list.erase(i);
        throw InternalError(PALUDIS_HERE, "duplicate resolution");
    }
    return ConstIterator(i);
}

void
ResolutionsByResolvent::serialise(Serialiser & s) const
{
    s.object("ResolutionsByResolvent")
        .member(SerialiserFlags<serialise::container>(), "items", _imp->resolution_list)
        ;
}

const std::shared_ptr<ResolutionsByResolvent>
ResolutionsByResolvent::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "ResolutionsByResolvent");
    Deserialisator vv(*v.find_remove_member("items"), "c");
    std::shared_ptr<ResolutionsByResolvent> result(new ResolutionsByResolvent);
    for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
        result->insert_new(vv.member<std::shared_ptr<Resolution> >(stringify(n)));
    return result;
}

template class WrappedForwardIterator<ResolutionsByResolvent::ConstIteratorTag, const std::shared_ptr<Resolution> >;

