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

#include <paludis/resolver/decision.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Decision & d)
{
    std::stringstream ss;

    ss << "Decision(";

    if (d.if_package_id())
        ss << *d.if_package_id();
    else
        ss << "(nothing)";

    if (d.is_installed())
        ss << ", is installed";
    if (d.is_new())
        ss << ", is new";
    if (d.is_nothing())
        ss << ", is nothing";
    if (d.is_same())
        ss << ", is same";
    if (d.is_same_version())
        ss << ", is same version";

    ss << ")";

    s << ss.str();
    return s;
}

