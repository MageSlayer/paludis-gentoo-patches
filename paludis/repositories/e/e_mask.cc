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

#include <paludis/repositories/e/e_mask.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<EUnacceptedMask>
    {
        const char key;
        const std::string description;
        const std::shared_ptr<const MetadataKey> unaccepted_key;

        Implementation(const char k, const std::string & d, const std::shared_ptr<const MetadataKey> & u) :
            key(k),
            description(d),
            unaccepted_key(u)
        {
        }
    };
}

EUnacceptedMask::EUnacceptedMask(const char k, const std::string & d, const std::shared_ptr<const MetadataKey> & u) :
    PrivateImplementationPattern<EUnacceptedMask>(new Implementation<EUnacceptedMask>(k, d, u))
{
}

EUnacceptedMask::~EUnacceptedMask()
{
}

char
EUnacceptedMask::key() const
{
    return _imp->key;
}

const std::string
EUnacceptedMask::description() const
{
    return _imp->description;
}

const std::shared_ptr<const MetadataKey>
EUnacceptedMask::unaccepted_key() const
{
    return _imp->unaccepted_key;
}

namespace paludis
{
    template <>
    struct Implementation<EUnsupportedMask>
    {
        const char key;
        const std::string description;
        const std::string eapi_name;

        Implementation(const char k, const std::string & d, const std::string & n) :
            key(k),
            description(d),
            eapi_name(n)
        {
        }
    };
}

EUnsupportedMask::EUnsupportedMask(const char k, const std::string & d, const std::string & n) :
    PrivateImplementationPattern<EUnsupportedMask>(new Implementation<EUnsupportedMask>(k, d, n))
{
}

EUnsupportedMask::~EUnsupportedMask()
{
}

char
EUnsupportedMask::key() const
{
    return _imp->key;
}

const std::string
EUnsupportedMask::description() const
{
    return _imp->description;
}

const std::string
EUnsupportedMask::explanation() const
{
    if (_imp->eapi_name == "UNKNOWN")
        return "Unsupported EAPI 'UNKNOWN' (likely a broken package or configuration error)";
    return "Unsupported EAPI '" + _imp->eapi_name + "'";
}

namespace paludis
{
    template <>
    struct Implementation<ERepositoryMask>
    {
        const char key;
        const std::string description;
        const std::shared_ptr<const MetadataKey> mask_key;

        Implementation(const char k, const std::string & d, const std::shared_ptr<const MetadataKey> & m) :
            key(k),
            description(d),
            mask_key(m)
        {
        }
    };
}

ERepositoryMask::ERepositoryMask(const char k, const std::string & d, const std::shared_ptr<const MetadataKey> & m) :
    PrivateImplementationPattern<ERepositoryMask>(new Implementation<ERepositoryMask>(k, d, m))
{
}

ERepositoryMask::~ERepositoryMask()
{
}

char
ERepositoryMask::key() const
{
    return _imp->key;
}

const std::string
ERepositoryMask::description() const
{
    return _imp->description;
}

const std::shared_ptr<const MetadataKey>
ERepositoryMask::mask_key() const
{
    return _imp->mask_key;
}

