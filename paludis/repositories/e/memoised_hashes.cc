/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Piotr Jaroszyński
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

#include <paludis/repositories/e/memoised_hashes.hh>

#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/md5.hh>
#include <paludis/util/timestamp.hh>

#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    typedef std::map<std::pair<std::string, int>, std::pair<Timestamp, std::string> > HashesMap;

    template <>
    struct Imp<MemoisedHashes>
    {
        mutable Mutex mutex;
        mutable HashesMap hashes;

        Imp()
        {
        }
    };
}

MemoisedHashes::MemoisedHashes() :
    Pimp<MemoisedHashes>()
{
}

MemoisedHashes::~MemoisedHashes()
{
}

namespace
{
    template <typename>
    struct HashIDs;

    template <>
    struct HashIDs<RMD160>
    {
        static const int id;
    };
    const int HashIDs<RMD160>::id = 0;

    template <>
    struct HashIDs<SHA1>
    {
        static const int id;
    };
    const int HashIDs<SHA1>::id = 1;

    template <>
    struct HashIDs<SHA256>
    {
        static const int id;
    };
    const int HashIDs<SHA256>::id = 2;

    template <>
    struct HashIDs<MD5>
    {
        static const int id;
    };
    const int HashIDs<MD5>::id = 3;
}

template <typename H_>
const std::string
MemoisedHashes::get(const FSEntry & file, SafeIFStream & stream) const
{
    std::pair<std::string, int> key(stringify(file), HashIDs<H_>::id);
    Timestamp mtime(file.mtim());

    Lock l(_imp->mutex);

    HashesMap::iterator i(_imp->hashes.find(key));

    if (i == _imp->hashes.end() || i->second.first != mtime)
    {
        H_ hash(stream);
        std::pair<Timestamp, std::string> value(std::make_pair(mtime, hash.hexsum()));
        stream.clear();
        stream.seekg(0, std::ios::beg);

        if (i != _imp->hashes.end())
            i->second = value;
        else
            i = _imp->hashes.insert(std::make_pair(key, value)).first;
    }

    return i->second.second;
}

template const std::string MemoisedHashes::get<RMD160>(const FSEntry &, SafeIFStream &) const;
template const std::string MemoisedHashes::get<SHA1>(const FSEntry &, SafeIFStream &) const;
template const std::string MemoisedHashes::get<SHA256>(const FSEntry &, SafeIFStream &) const;
template const std::string MemoisedHashes::get<MD5>(const FSEntry &, SafeIFStream &) const;

template class Pimp<MemoisedHashes>;
template class Singleton<MemoisedHashes>;

