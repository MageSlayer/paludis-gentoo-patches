/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011, 2013 Ciaran McCreesh
 * Copyright (c) 2008, 2012 David Leverton
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

#include "eclass_mtimes.hh"
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/name.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/md5.hh>
#include <paludis/util/safe_ifstream.hh>
#include <mutex>
#include <unordered_map>

using namespace paludis;

typedef std::unordered_map<FSPath, std::string, Hash<FSPath> > MD5Map;

namespace
{
    struct Cache
    {
        std::shared_ptr<const FSPathSequence> dirs;
        std::unordered_map<std::string, std::pair<FSPath, FSStat>, Hash<std::string> > files;

        Cache(const std::shared_ptr<const FSPathSequence> & d) :
            dirs(d)
        {
        }
    };

    const std::pair<FSPath, FSStat> *
    lookup(const std::string & e, Cache & c)
    {
        auto i(c.files.find(e));
        if (i != c.files.end())
            return & i->second;

        for (FSPathSequence::ReverseConstIterator d(c.dirs->rbegin()),
                 d_end(c.dirs->rend()) ; d != d_end ; ++d)
        {
            FSPath f(*d / e);
            if (f.stat().exists())
                return & c.files.insert(std::make_pair(e, std::make_pair(f.realpath(), f.stat()))).first->second;
        }

        return nullptr;
    }
}

namespace paludis
{
    template<>
    struct Imp<EclassMtimes>
    {
        const ERepository * repo;
        mutable Cache eclasses;
        mutable std::unordered_map<QualifiedPackageName, Cache, Hash<QualifiedPackageName> > exlibs;
        mutable MD5Map md5s;
        mutable std::mutex mutex;

        Imp(const ERepository * r, const std::shared_ptr<const FSPathSequence> & d) :
            repo(r),
            eclasses(d)
        {
        }
    };
}

EclassMtimes::EclassMtimes(const ERepository * r, const std::shared_ptr<const FSPathSequence> & d) :
    _imp(r, d)
{
}

EclassMtimes::~EclassMtimes()
{
}

const std::pair<FSPath, FSStat> *
EclassMtimes::eclass(const std::string & e) const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);
    return lookup(e + ".eclass", _imp->eclasses);
}

const std::pair<FSPath, FSStat> *
EclassMtimes::exlib(const std::string & e, const QualifiedPackageName & qpn) const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);
    std::unordered_map<QualifiedPackageName, Cache, Hash<QualifiedPackageName> >::iterator cache(_imp->exlibs.find(qpn));
    if (_imp->exlibs.end() == cache)
        cache = _imp->exlibs.insert(std::make_pair(qpn, Cache(_imp->repo->layout()->exlibsdirs(qpn)))).first;
    return lookup(e + ".exlib", cache->second);
}

std::string
EclassMtimes::md5(const FSPath & p) const
{
    std::unique_lock<std::mutex> lock(_imp->mutex);
    MD5Map::const_iterator it(_imp->md5s.find(p));
    if (_imp->md5s.end() != it)
        return it->second;

    SafeIFStream s(p);
    MD5 m(s);
    return _imp->md5s.insert(std::make_pair(p, m.hexsum())).first->second;
}

