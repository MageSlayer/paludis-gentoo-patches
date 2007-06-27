/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "metadata_key.hh"
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<MetadataKey>
    {
        const std::string raw_name;
        const std::string human_name;

        Implementation(const std::string & r, const std::string & h) :
            raw_name(r),
            human_name(h)
        {
        }
    };
}

MetadataKey::MetadataKey(const std::string & r, const std::string & h) :
    PrivateImplementationPattern<MetadataKey>(new Implementation<MetadataKey>(r, h))
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

MetadataStringKey::MetadataStringKey(const std::string & r, const std::string & h) :
    MetadataKey(r, h)
{
}

MetadataPackageIDKey::MetadataPackageIDKey(const std::string & r, const std::string & h) :
    MetadataKey(r, h)
{
}

template <typename C_>
MetadataCollectionKey<C_>::MetadataCollectionKey(const std::string & r, const std::string & h) :
    MetadataKey(r, h)
{
}

template <typename C_>
MetadataSpecTreeKey<C_>::MetadataSpecTreeKey(const std::string & r, const std::string & h) :
    MetadataKey(r, h)
{
}

template class MetadataCollectionKey<KeywordNameCollection>;
template class MetadataCollectionKey<IUseFlagCollection>;
template class MetadataCollectionKey<InheritedCollection>;
template class MetadataCollectionKey<UseFlagNameCollection>;

template class MetadataSpecTreeKey<LicenseSpecTree>;
template class MetadataSpecTreeKey<ProvideSpecTree>;
template class MetadataSpecTreeKey<DependencySpecTree>;
template class MetadataSpecTreeKey<RestrictSpecTree>;
template class MetadataSpecTreeKey<URISpecTree>;

