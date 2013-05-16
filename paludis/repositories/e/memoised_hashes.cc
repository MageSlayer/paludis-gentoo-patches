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

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/digest_registry.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>

#include <map>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    typedef std::map<std::pair<std::string, std::string>, std::pair<Timestamp, std::string> > HashesMap;

    template <>
    struct Imp<MemoisedHashes>
    {
        mutable std::mutex mutex;
        mutable HashesMap hashes;

        Imp()
        {
        }
    };
}

MemoisedHashes::MemoisedHashes() :
    _imp()
{
}

MemoisedHashes::~MemoisedHashes()
{
}

const std::string
MemoisedHashes::get(const std::string & algo, const FSPath & file, SafeIFStream & stream) const
{
    std::pair<std::string, std::string> key(stringify(file), algo);
    Timestamp mtime(file.stat().mtim());

    std::unique_lock<std::mutex> lock(_imp->mutex);

    HashesMap::iterator i(_imp->hashes.find(key));

    if (i == _imp->hashes.end() || i->second.first != mtime)
    {
        std::pair<Timestamp, std::string> value(std::make_pair(mtime, DigestRegistry::get_instance()->get(algo)(stream)));
        stream.clear();
        stream.seekg(0, std::ios::beg);

        if (i != _imp->hashes.end())
            i->second = value;
        else
            i = _imp->hashes.insert(std::make_pair(key, value)).first;
    }

    return i->second.second;
}

namespace paludis
{
    template class Pimp<MemoisedHashes>;
    template class Singleton<MemoisedHashes>;
}
