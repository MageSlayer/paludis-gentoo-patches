/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010 Ciaran McCreesh
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

#include "options.hh"
#include <paludis/util/pimp-impl.hh>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdint.h>

using namespace paludis;

namespace paludis
{
    template<>
    struct Imp<OptionsStore>
    {
        std::vector<uint8_t> pool;

        Imp()
        {
        }

        Imp(const Imp & i) :
            pool(i.pool)
        {
        }
    };
}

OptionsStore::OptionsStore() :
    Pimp<OptionsStore>()
{
}

OptionsStore::OptionsStore(const OptionsStore & s) :
    Pimp<OptionsStore>(*s._imp.get())
{
}

const OptionsStore &
OptionsStore::operator= (const OptionsStore & s)
{
    if (this != &s)
        _imp.reset(new Imp<OptionsStore>(*s._imp.get()));

    return *this;
}

OptionsStore::~OptionsStore()
{
}

void
OptionsStore::add(const unsigned e)
{
    if (_imp->pool.size() < e / 8 + 1)
        _imp->pool.resize(e / 8 + 1, 0);

    _imp->pool.at(e / 8) |= (1 << (e % 8));
}

void
OptionsStore::remove(const unsigned e)
{
    if (_imp->pool.size() < e / 8 + 1)
        _imp->pool.resize(e / 8 + 1, 0);

    _imp->pool.at(e / 8) &= ~(1 << (e % 8));
}

void
OptionsStore::combine(const OptionsStore & e)
{
    if (_imp->pool.size() < e._imp->pool.size())
        _imp->pool.resize(e._imp->pool.size(), 0);

    for (unsigned s(0), s_end(e._imp->pool.size()) ; s != s_end ; ++s)
        _imp->pool.at(s) |= e._imp->pool.at(s);
}

void
OptionsStore::subtract(const OptionsStore & e)
{
    if (_imp->pool.size() < e._imp->pool.size())
        _imp->pool.resize(e._imp->pool.size(), 0);

    for (unsigned s(0), s_end(e._imp->pool.size()) ; s != s_end ; ++s)
        _imp->pool.at(s) &= ~e._imp->pool.at(s);
}

bool
OptionsStore::test(const unsigned e) const
{
    if (_imp->pool.size() < e / 8 + 1)
        return false;

    return _imp->pool.at(e / 8) & (1 << (e % 8));
}

bool
OptionsStore::any() const
{
    return _imp->pool.end() != std::find_if(_imp->pool.begin(), _imp->pool.end(),
            std::bind2nd(std::not_equal_to<uint8_t>(), 0));
}

unsigned
OptionsStore::highest_bit() const
{
    return _imp->pool.size() * 8;
}

