/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/name.hh>
#include <tr1/functional>
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

MetadataSectionKey::MetadataSectionKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataSectionKey::~MetadataSectionKey()
{
}

MetadataTimeKey::MetadataTimeKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

template <typename C_>
MetadataValueKey<C_>::MetadataValueKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

template <typename C_>
MetadataCollectionKey<C_>::MetadataCollectionKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

ExtraMetadataValueKeyMethods<long>::~ExtraMetadataValueKeyMethods()
{
}

ExtraMetadataValueKeyMethods<bool>::~ExtraMetadataValueKeyMethods()
{
}

ExtraMetadataValueKeyMethods<std::tr1::shared_ptr<const PackageID> >::~ExtraMetadataValueKeyMethods()
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

MetadataSpecTreeKey<DependencySpecTree>::MetadataSpecTreeKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
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
template class MetadataSpecTreeKey<RestrictSpecTree>;
#ifndef PALUDIS_NO_EXPLICIT_FULLY_SPECIALISED
template class MetadataSpecTreeKey<FetchableURISpecTree>;
template class MetadataSpecTreeKey<DependencySpecTree>;
#endif
template class MetadataSpecTreeKey<SimpleURISpecTree>;

template class MetadataValueKey<std::string>;
template class MetadataValueKey<long>;
template class MetadataValueKey<bool>;
template class MetadataValueKey<FSEntry>;
template class MetadataValueKey<std::tr1::shared_ptr<const PackageID> >;
template class MetadataValueKey<std::tr1::shared_ptr<const Contents> >;
template class MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> >;

