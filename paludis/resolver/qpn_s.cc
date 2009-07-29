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

#include <paludis/resolver/qpn_s.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::operator< (const QPN_S & lhs, const QPN_S & rhs)
{
    if (lhs.package() < rhs.package())
        return true;
    if (lhs.package() > rhs.package())
        return false;

    /* no slot orders before any slot */
    if (lhs.slot_name_or_null())
    {
        if (rhs.slot_name_or_null())
            return *lhs.slot_name_or_null() < *rhs.slot_name_or_null();
        else
            return false;
    }
    else
    {
        if (rhs.slot_name_or_null())
            return true;
        else
            return false;
    }
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const QPN_S & q)
{
    std::stringstream ss;
    ss << q.package();
    if (q.slot_name_or_null())
        ss << ":" << *q.slot_name_or_null();
    else
        ss << ":(no slot)";

    s << ss.str();
    return s;
}

