/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011, 2012 Ciaran McCreesh
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

#include <paludis/util/pool-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <list>
#include <memory>

using namespace paludis;

typedef std::list<std::shared_ptr<const PoolKey> > PoolKeyList;

namespace paludis
{
    template <>
    struct Imp<PoolKeys>
    {
        std::shared_ptr<PoolKeyList> values;

        Imp() :
            values(std::make_shared<PoolKeyList>())
        {
        }

        Imp(const std::shared_ptr<PoolKeyList> & v) :
            values(v)
        {
        }
    };
}

PoolKeys::PoolKeys() :
    _imp()
{
}

PoolKeys::PoolKeys(const PoolKeys & other) :
    _imp(other._imp->values)
{
}

PoolKeys::~PoolKeys() = default;

void
PoolKeys::add()
{
}

void
PoolKeys::add_one(const std::shared_ptr<const PoolKey> & k)
{
    _imp->values->push_back(k);
}

PoolKey::PoolKey(int tc) :
    _tc(tc)
{
}

PoolKey::~PoolKey() = default;

bool
PoolKey::same_type_and_value(const PoolKey & other) const
{
    return _tc == other._tc && same_value(other);
}

std::size_t
PoolKeysHasher::operator() (const PoolKeys & keys) const
{
    std::size_t result(0);

    for (auto i(keys._imp->values->begin()), i_end(keys._imp->values->end()) ;
            i != i_end ; ++i)
    {
        result <<= 4;
        result ^= (*i)->hash();
    }

    return result;
}

bool
PoolKeysComparator::operator() (const PoolKeys & a, const PoolKeys & b) const
{
    for (auto i(a._imp->values->begin()), j(b._imp->values->begin()), i_end(a._imp->values->end()), j_end(b._imp->values->end()) ;
            (i != i_end || j != j_end) ; ++i, ++j)
    {
        if (i == i_end || j == j_end || ! (*i)->same_type_and_value(**j))
            return false;
    }

    return true;
}

int
PoolKeyTypeCodes::next()
{
    static std::mutex mutex;
    static int result(0);

    std::unique_lock<std::mutex> lock(mutex);
    return ++result;
}

namespace paludis
{
    template class Pimp<PoolKeys>;
}
