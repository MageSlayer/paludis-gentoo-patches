/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator-impl.hh>
#include <paludis/util/hashes.hh>

#include <functional>
#include <unordered_map>
#include <list>
#include <algorithm>

using namespace paludis;

#include <paludis/package_id-se.cc>

namespace paludis
{
    template <>
    struct Imp<PackageID>
    {
        mutable std::list<std::shared_ptr<const Mask> > masks;
        mutable std::list<std::shared_ptr<const OverriddenMask> > overridden_masks;
    };

    template <>
    struct WrappedForwardIteratorTraits<PackageID::MasksConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<const Mask> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<PackageID::OverriddenMasksConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<const OverriddenMask> >::const_iterator UnderlyingIterator;
    };
}

PackageID::PackageID() :
    _imp()
{
}

PackageID::~PackageID() = default;

void
PackageID::add_mask(const std::shared_ptr<const Mask> & k) const
{
    _imp->masks.push_back(k);
}

void
PackageID::add_overridden_mask(const std::shared_ptr<const OverriddenMask> & k) const
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

std::ostream &
paludis::operator<< (std::ostream & s, const PackageID & i)
{
    s << i.canonical_form(idcf_full);
    return s;
}

bool
PackageIDSetComparator::operator() (const std::shared_ptr<const PackageID> & a,
        const std::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    if (a->repository_name().value() < b->repository_name().value())
        return true;

    if (a->repository_name().value() > b->repository_name().value())
        return false;

    return a->arbitrary_less_than_comparison(*b);
}

bool
paludis::operator== (const PackageID & a, const PackageID & b)
{
    return (a.name() == b.name())
        && (a.version() == b.version())
        && (a.repository_name() == b.repository_name())
        && (! a.arbitrary_less_than_comparison(b))
        && (! b.arbitrary_less_than_comparison(a));
}

namespace paludis
{
    template <>
    struct Imp<PackageIDComparator>
    {
        std::unordered_map<RepositoryName, unsigned, Hash<RepositoryName> > m;
    };
}

PackageIDComparator::PackageIDComparator(const Environment * const e) :
    _imp()
{
    unsigned c(0);
    for (auto r(e->begin_repositories()), r_end(e->end_repositories()) ; r != r_end ; ++r)
        _imp->m.insert(std::make_pair((*r)->name(), ++c));
}

PackageIDComparator::PackageIDComparator(const PackageIDComparator & other) :
    _imp()
{
    _imp->m = other._imp->m;
}

PackageIDComparator::~PackageIDComparator() = default;

bool
PackageIDComparator::operator() (const std::shared_ptr<const PackageID> & a,
        const std::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    std::unordered_map<RepositoryName, unsigned, Hash<RepositoryName> >::const_iterator
        ma(_imp->m.find(a->repository_name())),
        mb(_imp->m.find(b->repository_name()));

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
        (Hash<RepositoryName>()(repository_name()) << 9) ^
        (extra_hash_value() << 13);
}

void
PackageID::can_drop_in_memory_cache() const
{
}

namespace paludis
{
    template class Sequence<std::shared_ptr<const PackageID> >;
    template class WrappedForwardIterator<Sequence<std::shared_ptr<const PackageID> >::ConstIteratorTag,
             const std::shared_ptr<const PackageID> >;
    template class WrappedForwardIterator<Sequence<std::shared_ptr<const PackageID> >::ReverseConstIteratorTag,
             const std::shared_ptr<const PackageID> >;
    template class WrappedOutputIterator<Sequence<std::shared_ptr<const PackageID> >::InserterTag,
             std::shared_ptr<const PackageID> >;

    template class Set<std::shared_ptr<const PackageID>, PackageIDSetComparator>;
    template class WrappedForwardIterator<Set<std::shared_ptr<const PackageID>, PackageIDSetComparator>::ConstIteratorTag,
             const std::shared_ptr<const PackageID> >;
    template class WrappedOutputIterator<Set<std::shared_ptr<const PackageID>, PackageIDSetComparator>::InserterTag,
             std::shared_ptr<const PackageID> >;

    template class WrappedForwardIterator<PackageID::MasksConstIteratorTag, const std::shared_ptr<const Mask> >;
    template class WrappedForwardIterator<PackageID::OverriddenMasksConstIteratorTag, const std::shared_ptr<const OverriddenMask> >;
}

