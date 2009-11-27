/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <list>
#include <algorithm>

using namespace paludis;

#include <paludis/package_id-se.cc>

namespace paludis
{
    template <>
    struct Implementation<PackageID>
    {
        mutable std::list<std::tr1::shared_ptr<const Mask> > masks;
        mutable std::list<std::tr1::shared_ptr<const OverriddenMask> > overridden_masks;
    };

    template <>
    struct WrappedForwardIteratorTraits<PackageID::MasksConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const Mask> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<PackageID::OverriddenMasksConstIteratorTag>
    {
        typedef std::list<std::tr1::shared_ptr<const OverriddenMask> >::const_iterator UnderlyingIterator;
    };
}

PackageID::PackageID() :
    PrivateImplementationPattern<PackageID>(new Implementation<PackageID>),
    _imp(PrivateImplementationPattern<PackageID>::_imp)
{
}

PackageID::~PackageID()
{
}

void
PackageID::add_mask(const std::tr1::shared_ptr<const Mask> & k) const
{
    _imp->masks.push_back(k);
}

void
PackageID::add_overridden_mask(const std::tr1::shared_ptr<const OverriddenMask> & k) const
{
    _imp->overridden_masks.push_back(k);
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

PackageID::OverriddenMasksConstIterator
PackageID::begin_overridden_masks() const
{
    need_masks_added();
    return OverriddenMasksConstIterator(_imp->overridden_masks.begin());
}

PackageID::OverriddenMasksConstIterator
PackageID::end_overridden_masks() const
{
    need_masks_added();
    return OverriddenMasksConstIterator(_imp->overridden_masks.end());
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
    _imp->overridden_masks.clear();
}

std::ostream &
paludis::operator<< (std::ostream & s, const PackageID & i)
{
    s << i.canonical_form(idcf_full);
    return s;
}

bool
PackageIDSetComparator::operator() (const std::tr1::shared_ptr<const PackageID> & a,
        const std::tr1::shared_ptr<const PackageID> & b) const
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
        std::tr1::unordered_map<RepositoryName, unsigned, Hash<RepositoryName> > m;
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

PackageIDComparator::PackageIDComparator(const PackageIDComparator & other) :
    PrivateImplementationPattern<PackageIDComparator>(new Implementation<PackageIDComparator>)
{
    _imp->m = other._imp->m;
}

PackageIDComparator::~PackageIDComparator()
{
}

bool
PackageIDComparator::operator() (const std::tr1::shared_ptr<const PackageID> & a,
        const std::tr1::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    std::tr1::unordered_map<RepositoryName, unsigned, Hash<RepositoryName> >::const_iterator
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

std::size_t
PackageID::hash() const
{
    return
        (Hash<QualifiedPackageName>()(name()) << 0) ^
        (Hash<VersionSpec>()(version()) << 5) ^
        (Hash<RepositoryName>()(repository()->name()) << 9) ^
        (extra_hash_value() << 13);
}

void
PackageID::can_drop_in_memory_cache() const
{
}

template class Sequence<std::tr1::shared_ptr<const PackageID> >;
template class WrappedForwardIterator<Sequence<std::tr1::shared_ptr<const PackageID> >::ConstIteratorTag,
         const std::tr1::shared_ptr<const PackageID> >;
template class WrappedForwardIterator<Sequence<std::tr1::shared_ptr<const PackageID> >::ReverseConstIteratorTag,
         const std::tr1::shared_ptr<const PackageID> >;
template class WrappedOutputIterator<Sequence<std::tr1::shared_ptr<const PackageID> >::InserterTag,
         std::tr1::shared_ptr<const PackageID> >;

template class Set<std::tr1::shared_ptr<const PackageID>, PackageIDSetComparator>;
template class WrappedForwardIterator<Set<std::tr1::shared_ptr<const PackageID>, PackageIDSetComparator>::ConstIteratorTag,
         const std::tr1::shared_ptr<const PackageID> >;
template class WrappedOutputIterator<Set<std::tr1::shared_ptr<const PackageID>, PackageIDSetComparator>::InserterTag,
         std::tr1::shared_ptr<const PackageID> >;

template class WrappedForwardIterator<PackageID::MasksConstIteratorTag, const std::tr1::shared_ptr<const Mask> >;
template class WrappedForwardIterator<PackageID::OverriddenMasksConstIteratorTag, const std::tr1::shared_ptr<const OverriddenMask> >;

