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
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/hashes.hh>
#include <unordered_map>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    class EUnacceptedMask :
        public UnacceptedMask
    {
        private:
            Pimp<EUnacceptedMask> _imp;

        public:
            EUnacceptedMask(const char, const std::string &, const std::string &);
            ~EUnacceptedMask();

            char key() const;
            const std::string description() const;
            const std::string unaccepted_key_name() const;
    };
}

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
        const std::string comment;
        const std::string token;
        const FSPath mask_file;

        Imp(const char k, const std::string & d, const std::string & c, const std::string & t, const FSPath & f) :
            key(k),
            description(d),
            comment(c),
            token(t),
            mask_file(f)
        {
        }
    };
}

ERepositoryMask::ERepositoryMask(const char k, const std::string & d, const std::string & c,
        const std::string & t, const FSPath & f) :
    _imp(k, d, c, t, f)
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
ERepositoryMask::comment() const
{
    return _imp->comment;
}

const std::string
ERepositoryMask::token() const
{
    return _imp->token;
}

const FSPath
ERepositoryMask::mask_file() const
{
    return _imp->mask_file;
}

namespace
{
    typedef std::tuple<char, std::string, std::string> EUnacceptedMaskIndex;
}

namespace paludis
{
    template <>
    struct Imp<EUnacceptedMaskStore>
    {
        mutable Mutex mutex;
        mutable std::unordered_map<EUnacceptedMaskIndex, std::shared_ptr<const UnacceptedMask>, Hash<EUnacceptedMaskIndex> > store;
    };
}

EUnacceptedMaskStore::EUnacceptedMaskStore() :
    _imp()
{
}

EUnacceptedMaskStore::~EUnacceptedMaskStore()
{
}

const std::shared_ptr<const UnacceptedMask>
EUnacceptedMaskStore::fetch(const char c, const std::string & s, const std::string & k)
{
    EUnacceptedMaskIndex index(c, s, k);

    Lock lock(_imp->mutex);
    auto i(_imp->store.find(index));
    if (i == _imp->store.end())
        i = _imp->store.insert(std::make_pair(index, std::make_shared<EUnacceptedMask>(c, s, k))).first;

    return i->second;
}

template class Singleton<EUnacceptedMaskStore>;
template class Pimp<EUnacceptedMaskStore>;

