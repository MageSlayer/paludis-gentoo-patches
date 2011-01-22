/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EUnacceptedMask>
    {
        const char key;
        const std::string description;
        const std::string unaccepted_key_name;

        Imp(const char k, const std::string & d, const std::string & u) :
            key(k),
            description(d),
            unaccepted_key_name(u)
        {
        }
    };
}

EUnacceptedMask::EUnacceptedMask(const char k, const std::string & d, const std::string & u) :
    _imp(k, d, u)
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

const std::string
EUnacceptedMask::unaccepted_key_name() const
{
    return _imp->unaccepted_key_name;
}

namespace paludis
{
    template <>
    struct Imp<EUnsupportedMask>
    {
        const char key;
        const std::string description;
        const std::string eapi_name;

        Imp(const char k, const std::string & d, const std::string & n) :
            key(k),
            description(d),
            eapi_name(n)
        {
        }
    };
}

EUnsupportedMask::EUnsupportedMask(const char k, const std::string & d, const std::string & n) :
    _imp(k, d, n)
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
    struct Imp<ERepositoryMask>
    {
        const char key;
        const std::string description;
        const std::string mask_key_name;

        Imp(const char k, const std::string & d, const std::string & m) :
            key(k),
            description(d),
            mask_key_name(m)
        {
        }
    };
}

ERepositoryMask::ERepositoryMask(const char k, const std::string & d, const std::string & m) :
    _imp(k, d, m)
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

const std::string
ERepositoryMask::mask_key_name() const
{
    return _imp->mask_key_name;
}

