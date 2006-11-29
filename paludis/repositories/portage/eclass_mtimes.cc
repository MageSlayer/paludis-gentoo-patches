/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/hashed_containers.hh>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<EclassMtimes> :
        public InternalCounted<Implementation<EclassMtimes> >
    {
        FSEntryCollection::ConstPointer eclass_dirs;
        mutable MakeHashedMap<std::string, time_t>::Type eclass_mtimes;

        Implementation(FSEntryCollection::ConstPointer d) :
            eclass_dirs(d)
        {
        }
    };
}

EclassMtimes::EclassMtimes(FSEntryCollection::ConstPointer d) :
    PrivateImplementationPattern<EclassMtimes>(new Implementation<EclassMtimes>(d))
{
}

EclassMtimes::~EclassMtimes()
{
}

time_t
EclassMtimes::mtime(const std::string & e) const
{
    MakeHashedMap<std::string, time_t>::Type::const_iterator i(_imp->eclass_mtimes.find(e));
    if (i != _imp->eclass_mtimes.end())
        return i->second;

    time_t r(0);
    for (FSEntryCollection::Iterator d(_imp->eclass_dirs->begin()),
            d_end(_imp->eclass_dirs->end()) ; d != d_end ; ++d)
    {
        FSEntry f(*d / (e + ".eclass"));
        if (f.exists())
            r = std::max(r, f.mtime());
    }

    _imp->eclass_mtimes.insert(std::make_pair(e, r));
    return r;
}

