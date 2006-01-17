/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_SRC_COLOUR_HH
#define PALUDIS_GUARD_SRC_COLOUR_HH 1

#include <string>
#include <paludis/stringify.hh>
#include "src/command_line.hh"

enum Colour
{
    cl_red           = 31,
    cl_green         = 32,
    cl_blue          = 34,

    cl_bold_red      = cl_red + 100,
    cl_bold_green    = cl_green + 100,
    cl_bold_blue     = cl_blue + 100,

    cl_package_name  = cl_bold_blue,
    cl_flag_on       = cl_green,
    cl_flag_off      = cl_red,
    cl_slot          = cl_blue,
    cl_visible       = cl_flag_on,
    cl_masked        = cl_flag_off
};

template <typename T_>
std::string colour(Colour colour, const T_ & s)
{
    if (CommandLine::get_instance()->a_no_color.specified())
        return paludis::stringify(s);
    else
        return "\033[" + paludis::stringify(static_cast<unsigned>(colour) / 100) + ";"
            + paludis::stringify(static_cast<unsigned>(colour) % 100) + "m" + paludis::stringify(s)
            + "\033[0;0m";
}

#endif
