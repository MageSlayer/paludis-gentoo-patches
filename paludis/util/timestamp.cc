/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/timestamp.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <time.h>

using namespace paludis;

Timestamp::Timestamp(const struct timespec & t) :
    _s(t.tv_sec),
    _ns(t.tv_nsec)
{
}

Timestamp::Timestamp(const time_t s, const long ns) :
    _s(s),
    _ns(ns)
{
}

Timestamp::Timestamp(const Timestamp & other) :
    _s(other._s),
    _ns(other._ns)
{
}

Timestamp &
Timestamp::operator= (const Timestamp & other)
{
    _s = other._s;
    _ns = other._ns;
    return *this;
}

bool
Timestamp::operator== (const Timestamp & other) const
{
    return _s == other._s && _ns == other._ns;
}

bool
Timestamp::operator< (const Timestamp & other) const
{
    return (_s < other._s) || (_s == other._s && _ns < other._ns);
}

time_t
Timestamp::seconds() const
{
    return _s;
}

long
Timestamp::nanoseconds() const
{
    return _ns;
}

struct timespec
Timestamp::as_timespec() const
{
    struct timespec result = { _s, _ns };
    return result;
}

Timestamp
Timestamp::now()
{
    struct timespec t;
    if (0 != clock_gettime(CLOCK_REALTIME, &t))
        throw InternalError(PALUDIS_HERE, "clock_gettime returned non-zero");
    return Timestamp(t);
}

