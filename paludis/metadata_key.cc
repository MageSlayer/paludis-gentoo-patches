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
#include <paludis/name.hh>

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

template <typename C_>
MetadataSetKey<C_>::MetadataSetKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
    MetadataKey(r, h, t)
{
}

MetadataSetKey<IUseFlagSet>::MetadataSetKey(const std::string & r, const std::string & h, const MetadataKeyType t) :
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

template class MetadataSetKey<KeywordNameSet>;
template class MetadataSetKey<IUseFlagSet>;
template class MetadataSetKey<Set<std::string> >;
template class MetadataSetKey<UseFlagNameSet>;
template class MetadataSetKey<PackageIDSequence>;

template class MetadataSpecTreeKey<LicenseSpecTree>;
template class MetadataSpecTreeKey<ProvideSpecTree>;
template class MetadataSpecTreeKey<DependencySpecTree>;
template class MetadataSpecTreeKey<RestrictSpecTree>;
template class MetadataSpecTreeKey<FetchableURISpecTree>;
template class MetadataSpecTreeKey<SimpleURISpecTree>;

