/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_COLOUR_HH
#define PALUDIS_GUARD_SRC_COLOUR_HH 1

#include <paludis/util/stringify.hh>
#include <config.h>
#include <string>

enum Colour
{
    cl_none          = 0,
    cl_red           = 31,
    cl_green         = 32,
    cl_yellow        = 33,
    cl_blue          = 34,
    cl_pink          = 35,
    cl_grey          = 38,

    cl_bold_red      = cl_red + 100,
    cl_bold_green    = cl_green + 100,
    cl_bold_yellow   = cl_yellow + 100,
    cl_bold_blue     = cl_blue + 100,
    cl_bold_pink     = cl_pink + 100,

#if PALUDIS_COLOUR_PINK==1
    cl_package_name  = cl_bold_pink,
    cl_installable_package_name  = cl_pink,
    cl_repository_name = cl_pink,
    cl_flag_on       = cl_pink,
    cl_flag_off      = cl_red,
    cl_slot          = cl_blue,
    cl_visible       = cl_flag_on,
    cl_masked        = cl_flag_off,
    cl_not_masked    = cl_flag_on,
    cl_heading       = cl_bold_pink,
    cl_updatemode    = cl_yellow,
    cl_tag           = cl_yellow,
    cl_key_name      = cl_pink,
#else
    cl_package_name  = cl_bold_blue,
    cl_installable_package_name  = cl_blue,
    cl_repository_name = cl_blue,
    cl_flag_on       = cl_green,
    cl_flag_off      = cl_red,
    cl_slot          = cl_blue,
    cl_visible       = cl_flag_on,
    cl_masked        = cl_flag_off,
    cl_not_masked    = cl_flag_on,
    cl_heading       = cl_bold_green,
    cl_updatemode    = cl_yellow,
    cl_tag           = cl_yellow,
    cl_key_name      = cl_blue,
#endif

    cl_unimportant   = cl_grey,
    cl_error         = cl_bold_red,

    cl_file          = cl_none,
    cl_dir           = cl_blue,
    cl_sym           = cl_pink,
    cl_other         = cl_red
};

bool PALUDIS_VISIBLE use_colour();
void PALUDIS_VISIBLE set_use_colour(const bool value);
void PALUDIS_VISIBLE set_force_colour(const bool value);
bool PALUDIS_VISIBLE use_xterm_titles() PALUDIS_ATTRIBUTE((pure));

template <typename T_>
std::string colour(Colour colour, const T_ & s)
{
    if (! use_colour())
        return paludis::stringify(s);
    else if (cl_none != colour)
        return "\033[" + paludis::stringify(static_cast<unsigned>(colour) / 100) + ";"
            + paludis::stringify(static_cast<unsigned>(colour) % 100) + "m" + paludis::stringify(s)
            + "\033[0;0m";
    else
        return paludis::stringify(s);
}

template <typename T_>
std::string xterm_title(const T_ & s)
{
    if (! use_colour() || ! use_xterm_titles())
        return "";
    else
        return "\x1b]2;" + paludis::stringify(s) + "\x07";
}

#endif
