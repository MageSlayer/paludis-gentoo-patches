/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include "colours.hh"
#include "format_user_config.hh"
#include "config.h"

using namespace paludis;
using namespace cave;

const Colour
paludis::cave::c::bold_blue()
{
    return Colour{"bold_blue"};
}

const Colour
paludis::cave::c::blue()
{
    return Colour{"blue"};
}

const Colour
paludis::cave::c::bold_green()
{
    return Colour{"bold_green"};
}

const Colour
paludis::cave::c::green()
{
    return Colour{"green"};
}

const Colour
paludis::cave::c::red()
{
    return Colour{"red"};
}

const Colour
paludis::cave::c::bold_red()
{
    return Colour{"bold_red"};
}

const Colour
paludis::cave::c::yellow()
{
    return Colour{"yellow"};
}

const Colour
paludis::cave::c::bold_yellow()
{
    return Colour{"bold_yellow"};
}

const Colour
paludis::cave::c::pink()
{
    return Colour{"pink"};
}

const Colour
paludis::cave::c::bold_pink()
{
    return Colour{"bold_pink"};
}

const Colour
paludis::cave::c::bold_blue_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::bold_pink();
#else
    return c::bold_blue();
#endif
}

const Colour
paludis::cave::c::blue_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::pink();
#else
    return c::blue();
#endif
}

const Colour
paludis::cave::c::bold_green_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::bold_pink();
#else
    return c::bold_green();
#endif
}

const Colour
paludis::cave::c::green_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::pink();
#else
    return c::green();
#endif
}

const Colour
paludis::cave::c::normal()
{
    return Colour{"normal"};
}

const Colour
paludis::cave::c::bold_normal()
{
    return Colour{"bold_normal"};
}

std::string
Colour::colour_string() const
{
    return FormatUserConfigFile::get_instance()->fetch(name, 0, "");
}

