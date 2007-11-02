/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/hashed_containers.hh>

#include <list>
#include <algorithm>

using namespace paludis;

#include <paludis/package_id-se.cc>

template class Sequence<tr1::shared_ptr<const PackageID> >;
template class WrappedForwardIterator<Sequence<tr1::shared_ptr<const PackageID> >::ConstIteratorTag,
         const tr1::shared_ptr<const PackageID> >;
template class WrappedOutputIterator<Sequence<tr1::shared_ptr<const PackageID> >::InserterTag,
         tr1::shared_ptr<const PackageID> >;

template class Set<tr1::shared_ptr<const PackageID>, PackageIDSetComparator>;
template class WrappedForwardIterator<Set<tr1::shared_ptr<const PackageID> >::ConstIteratorTag,
         const tr1::shared_ptr<const PackageID> >;
template class WrappedOutputIterator<Set<tr1::shared_ptr<const PackageID> >::InserterTag,
         tr1::shared_ptr<const PackageID> >;

template class WrappedForwardIterator<PackageID::MetadataConstIteratorTag, tr1::shared_ptr<const MetadataKey> >;
template class WrappedForwardIterator<PackageID::MasksConstIteratorTag, tr1::shared_ptr<const Mask> >;

namespace paludis
{
    template <>
    struct Implementation<PackageID>
    {
        mutable std::list<tr1::shared_ptr<const MetadataKey> > keys;
        mutable std::list<tr1::shared_ptr<const Mask> > masks;
    };
}

PackageID::PackageID() :
    PrivateImplementationPattern<PackageID>(new Implementation<PackageID>)
{
}

PackageID::~PackageID()
{
}

void
PackageID::add_metadata_key(const tr1::shared_ptr<const MetadataKey> & k) const
{
    using namespace tr1::placeholders;

    if (_imp->keys.end() != std::find_if(_imp->keys.begin(), _imp->keys.end(),
                tr1::bind(std::equal_to<std::string>(), k->raw_name(), tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1))))
        throw ConfigurationError("Tried to add duplicate key '" + k->raw_name() + "' to ID '" + stringify(*this) + "'");

    _imp->keys.push_back(k);
}

PackageID::MetadataConstIterator
PackageID::begin_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.begin());
}

PackageID::MetadataConstIterator
PackageID::end_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.end());
}

void
PackageID::add_mask(const tr1::shared_ptr<const Mask> & k) const
{
    _imp->masks.push_back(k);
}

PackageID::MasksConstIterator
PackageID::begin_masks() const
{
    need_masks_added();
    return MasksConstIterator(_imp->masks.begin());
}

PackageID::MasksConstIterator
PackageID::end_masks() const
{
    need_masks_added();
    return MasksConstIterator(_imp->masks.end());
}

bool
PackageID::masked() const
{
    return begin_masks() != end_masks();
}

void
PackageID::invalidate_masks() const
{
    _imp->masks.clear();
}

PackageID::MetadataConstIterator
PackageID::find_metadata(const std::string & s) const
{
    using namespace tr1::placeholders;

    need_keys_added();
    return std::find_if(begin_metadata(), end_metadata(),
            tr1::bind(std::equal_to<std::string>(), s, tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1)));
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageID & i)
{
    s << i.canonical_form(idcf_full);
    return s;
}

bool
PackageIDSetComparator::operator() (const tr1::shared_ptr<const PackageID> & a,
        const tr1::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    if (a->repository()->name().data() < b->repository()->name().data())
        return true;

    if (a->repository()->name().data() > b->repository()->name().data())
        return false;

    return a->arbitrary_less_than_comparison(*b);
}

bool
paludis::operator== (const PackageID & a, const PackageID & b)
{
    return (a.name() == b.name())
        && (a.version() == b.version())
        && (a.repository()->name() == b.repository()->name())
        && (! a.arbitrary_less_than_comparison(b))
        && (! b.arbitrary_less_than_comparison(a));
}

namespace paludis
{
    template <>
    struct Implementation<PackageIDComparator>
    {
        MakeHashedMap<RepositoryName, unsigned>::Type m;
    };
}

PackageIDComparator::PackageIDComparator(const PackageDatabase * const db) :
    PrivateImplementationPattern<PackageIDComparator>(new Implementation<PackageIDComparator>)
{
    unsigned c(0);
    for (PackageDatabase::RepositoryConstIterator r(db->begin_repositories()),
            r_end(db->end_repositories()) ; r != r_end ; ++r)
        _imp->m.insert(std::make_pair((*r)->name(), ++c));
}

PackageIDComparator::~PackageIDComparator()
{
}

bool
PackageIDComparator::operator() (const tr1::shared_ptr<const PackageID> & a,
        const tr1::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    MakeHashedMap<RepositoryName, unsigned>::Type::const_iterator
        ma(_imp->m.find(a->repository()->name())),
        mb(_imp->m.find(b->repository()->name()));

    if (ma == _imp->m.end() || mb == _imp->m.end())
        throw InternalError(PALUDIS_HERE, "Repository not in database");

    if (ma->second < mb->second)
        return true;
    if (ma->second > mb->second)
        return false;

    return a->arbitrary_less_than_comparison(*b);
}

