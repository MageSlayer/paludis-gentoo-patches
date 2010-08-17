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

#include "formats.hh"
#include "format_user_config.hh"
#include "config.h"

using namespace paludis;
using namespace cave;

const std::string
paludis::cave::c::bold_blue()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_blue", 0, "");
}

const std::string
paludis::cave::c::blue()
{
    return FormatUserConfigFile::get_instance()->fetch("blue", 0, "");
}

const std::string
paludis::cave::c::bold_green()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_green", 0, "");
}

const std::string
paludis::cave::c::green()
{
    return FormatUserConfigFile::get_instance()->fetch("green", 0, "");
}

const std::string
paludis::cave::c::red()
{
    return FormatUserConfigFile::get_instance()->fetch("red", 0, "");
}

const std::string
paludis::cave::c::bold_red()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_red", 0, "");
}

const std::string
paludis::cave::c::yellow()
{
    return FormatUserConfigFile::get_instance()->fetch("yellow", 0, "");
}

const std::string
paludis::cave::c::bold_yellow()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_yellow", 0, "");
}

const std::string
paludis::cave::c::pink()
{
    return FormatUserConfigFile::get_instance()->fetch("pink", 0, "");
}

const std::string
paludis::cave::c::bold_pink()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_pink", 0, "");
}

const std::string
paludis::cave::c::bold_blue_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::bold_pink();
#else
    return c::bold_blue();
#endif
}

const std::string
paludis::cave::c::blue_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::pink();
#else
    return c::blue();
#endif
}

const std::string
paludis::cave::c::bold_green_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::bold_pink();
#else
    return c::bold_green();
#endif
}

const std::string
paludis::cave::c::green_or_pink()
{
#if PALUDIS_COLOUR_PINK
    return c::pink();
#else
    return c::green();
#endif
}

const std::string
paludis::cave::c::normal()
{
    return FormatUserConfigFile::get_instance()->fetch("normal", 0, "");
}

const std::string
paludis::cave::c::bold_normal()
{
    return FormatUserConfigFile::get_instance()->fetch("bold_normal", 0, "");
}

