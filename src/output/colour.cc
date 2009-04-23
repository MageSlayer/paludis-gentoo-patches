/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include "colour.hh"
#include <paludis/util/system.hh>
#include <unistd.h>

namespace
{
    static bool can_use_colour = true;
    static bool force_colour = false;
}

bool
use_colour()
{
    if (! can_use_colour)
        return false;

    if (force_colour)
        return true;

    static bool result(
            (1 == isatty(1)) &&
            (0 != paludis::getenv_with_default("TERM", "").compare(0, 4, "dumb")));

    return result;
}

void
set_use_colour(const bool value)
{
    can_use_colour = value;
}

void
set_force_colour(const bool value)
{
    force_colour = value;
}

bool
use_xterm_titles()
{
    static bool result(
            (0 != paludis::getenv_with_default("TERM", "").compare(0, 4, "dumb")) &&
            (0 != paludis::getenv_with_default("TERM", "").compare(0, 5, "linux")) &&
            (0 == paludis::getenv_with_default("PALUDIS_NO_XTERM_TITLES", "").length()));

    return result;
}


