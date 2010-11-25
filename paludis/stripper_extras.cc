/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/stripper_extras.hh>
#include <paludis/stripper.hh>
#include <magic.h>

#include "config.h"

using namespace paludis;

struct PaludisStripperExtras
{
    magic_t magic;
};

extern "C"
PaludisStripperExtras *
paludis_stripper_extras_init()
{
    auto extras(new PaludisStripperExtras);
    extras->magic = magic_open(MAGIC_NONE);

    if (! extras->magic)
        throw StripperError("magic_open returned null");

    if (-1 == magic_load(extras->magic, NULL))
        throw StripperError("magic_load returned error");

    return extras;
}

extern "C"
const std::string
paludis_stripper_extras_lookup(PaludisStripperExtras * const extras, const std::string & path)
{
    auto result(magic_file(extras->magic, path.c_str()));
    if (! result)
    {
        result = magic_error(extras->magic);
        if (result)
            return "(error) " + std::string(result);
        else
            return "(no error)";
    }

    return std::string(result);
}

extern "C"
void
paludis_stripper_extras_cleanup(PaludisStripperExtras * const extras)
{
    magic_close(extras->magic);
    delete extras;
}

