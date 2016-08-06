/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>

using namespace paludis;

OptionsStore::OptionsStore() :
    _bits(0)
{
}

void
OptionsStore::add(const unsigned e)
{
    if (e > (8 * sizeof(unsigned long)))
        throw InternalError(PALUDIS_HERE, "options oversized");

    _bits |= (1ul << e);
}

void
OptionsStore::remove(const unsigned e)
{
    if (e > (8 * sizeof(unsigned long)))
        throw InternalError(PALUDIS_HERE, "options oversized");

    _bits &= ~(1ul << e);
}

void
OptionsStore::combine(const OptionsStore & e)
{
    _bits |= e._bits;
}

void
OptionsStore::subtract(const OptionsStore & e)
{
    _bits &= ~e._bits;
}

void
OptionsStore::intersect(const OptionsStore & e)
{
    _bits &= e._bits;
}

bool
OptionsStore::test(const unsigned e) const
{
    if (e > (8 * sizeof(unsigned long)))
        throw InternalError(PALUDIS_HERE, "options oversized");

    return 0 != (_bits & (1ul << e));
}

bool
OptionsStore::any() const
{
    return 0 != _bits;
}

unsigned
OptionsStore::highest_bit() const
{
    return sizeof(unsigned long) * 8;
}

