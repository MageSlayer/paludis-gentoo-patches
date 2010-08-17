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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOURS_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COLOURS_HH 1

#include <string>

namespace paludis
{
    namespace cave
    {
        struct Colour
        {
            std::string name;

            std::string colour_string() const;
        };

        namespace c
        {
            const Colour bold_blue();
            const Colour blue();
            const Colour bold_green();
            const Colour green();
            const Colour bold_red();
            const Colour red();
            const Colour bold_yellow();
            const Colour yellow();
            const Colour bold_pink();
            const Colour pink();

            const Colour bold_blue_or_pink();
            const Colour blue_or_pink();
            const Colour bold_green_or_pink();
            const Colour green_or_pink();

            const Colour bold_normal();
            const Colour normal();
        }
    }
}

#endif
