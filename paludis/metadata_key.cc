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

#include <paludis/metadata_key.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/name.hh>
#include <list>
#include <algorithm>

using namespace paludis;

#include <paludis/metadata_key-se.cc>

namespace paludis
{
    template <>
    struct Implementation<MetadataKey>
    {
        const std::string raw_name;
        const std::string human_name;
        const MetadataKeyType type;

        Implementation(const std::string & r, const std::string & h, const MetadataKeyType t) :
            raw_name(r),
            human_name(h),
            type(t)
        {
        }
    };
}

MetadataKey::MetadataKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    PrivateImplementationPattern<MetadataKey>(new Implementation<MetadataKey>(r, h, t))
{
}

MetadataKey::~MetadataKey()
{
}

const std::string
MetadataKey::raw_name() const
{
    return _imp->raw_name;
}

const std::string
MetadataKey::human_name() const
{
    return _imp->human_name;
}

MetadataKeyType
MetadataKey::type() const
{
    return _imp->type;
}

MetadataStringKey::MetadataStringKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataFSEntryKey::MetadataFSEntryKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataTimeKey::MetadataTimeKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataContentsKey::MetadataContentsKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataPackageIDKey::MetadataPackageIDKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataRepositoryMaskInfoKey::MetadataRepositoryMaskInfoKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

namespace paludis
{
    template <>
    struct Implementation<MetadataSectionKey>
    {
        mutable std::list<tr1::shared_ptr<const MetadataKey> > keys;
    };
}

MetadataSectionKey::MetadataSectionKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t),
    PrivateImplementationPattern<MetadataSectionKey>(new Implementation<MetadataSectionKey>),
    _imp(PrivateImplementationPattern<MetadataSectionKey>::_imp)
{
}

MetadataSectionKey::~MetadataSectionKey()
{
}

void
MetadataSectionKey::add_metadata_key(const tr1::shared_ptr<const MetadataKey> & k) const
{
    using namespace tr1::placeholders;

    if (indirect_iterator(_imp->keys.end()) != std::find_if(indirect_iterator(_imp->keys.begin()), indirect_iterator(_imp->keys.end()),
                tr1::bind(std::equal_to<std::string>(), k->raw_name(), tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1))))
        throw ConfigurationError("Tried to add duplicate key '" + k->raw_name() + "'");

    _imp->keys.push_back(k);
}

MetadataSectionKey::MetadataConstIterator
MetadataSectionKey::begin_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.begin());
}

MetadataSectionKey::MetadataConstIterator
MetadataSectionKey::end_metadata() const
{
    need_keys_added();
    return MetadataConstIterator(_imp->keys.end());
}

MetadataSectionKey::MetadataConstIterator
MetadataSectionKey::find_metadata(const std::string & s) const
{
    using namespace tr1::placeholders;

    need_keys_added();

    // tr1::mem_fn on a sptr doesn't work with boost
    // return std::find_if(begin_metadata(), end_metadata(),
    //        tr1::bind(std::equal_to<std::string>(), s, tr1::bind(tr1::mem_fn(&MetadataKey::raw_name), _1)));

    for (MetadataConstIterator i(begin_metadata()), i_end(end_metadata()) ;
            i != i_end ; ++i)
        if ((*i)->raw_name() == s)
            return i;
    return end_metadata();
}

template <typename C_>
MetadataCollectionKey<C_>::MetadataCollectionKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataCollectionKey<IUseFlagSet>::MetadataCollectionKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

template <typename C_>
MetadataSpecTreeKey<C_>::MetadataSpecTreeKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataSpecTreeKey<FetchableURISpecTree>::MetadataSpecTreeKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

template class MetadataCollectionKey<KeywordNameSet>;
#ifndef PALUDIS_NO_EXPLICIT_FULLY_SPECIALISED
template class MetadataCollectionKey<IUseFlagSet>;
#endif
template class MetadataCollectionKey<Set<std::string> >;
template class MetadataCollectionKey<UseFlagNameSet>;
template class MetadataCollectionKey<PackageIDSequence>;
template class MetadataCollectionKey<FSEntrySequence>;

template class MetadataSpecTreeKey<LicenseSpecTree>;
template class MetadataSpecTreeKey<ProvideSpecTree>;
template class MetadataSpecTreeKey<DependencySpecTree>;
template class MetadataSpecTreeKey<RestrictSpecTree>;
#ifndef PALUDIS_NO_EXPLICIT_FULLY_SPECIALISED
template class MetadataSpecTreeKey<FetchableURISpecTree>;
#endif
template class MetadataSpecTreeKey<SimpleURISpecTree>;

