/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Mike Kelly
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

#include <paludis/util/exception.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/stringify.hh>

#include <ctime>
#include <iomanip>
#include <sstream>

using namespace paludis;

std::string
paludis::pretty_print_bytes(const long & bytes)
{
    double size(bytes);
    int i(0);
    std::string suffix[] = {"Bytes", "kBytes", "MBytes", "GBytes"};
    std::ostringstream val;

    while (size >= 1024.0 && i < 3)
    {
        size /= 1024.0;
        i++;
    }

    if (i >= 1)
        val << std::fixed << std::setprecision(2);

    val << size << " " << suffix[i];

    return val.str();
}

std::string
paludis::pretty_print_time(const time_t & t)
{
    char buf[255];

    if (0 == std::strftime(buf, 255, "%a %b %d %T %Z %Y", std::localtime(&t)))
        throw InternalError(PALUDIS_HERE, "strftime failed");

    return std::string(buf);
}

