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

#include <paludis/repositories/gemcutter/json_things_handle.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/exception.hh>

#include <paludis/about.hh>

#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

#include "config.h"

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;
using namespace paludis::gemcutter_repository;

JSONThingsHandle::JSONThingsHandle() :
    handle(0),
    parse_all_gems_function(0)
{
    handle = ::dlopen(("libpaludisgemcutterrepositoryjsonthings_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (! handle)
        throw NotAvailableError("JSON not available because dlopen said " + stringify(::dlerror()));

    parse_all_gems_function = STUPID_CAST(ParseAllGemsFunction, ::dlsym(handle, "gemcutter_json_things_parse_all_gems"));
    if (! parse_all_gems_function)
        throw NotAvailableError("JSON not available because dlsym said " + stringify(::dlerror()));
}

JSONThingsHandle::~JSONThingsHandle()
{
    if (handle)
        ::dlclose(handle);
}

void
JSONThingsHandle::parse_all_gems(const FSPath & p, const ParsedOneGemCallback & c) const
{
    parse_all_gems_function(p, c);
}

namespace paludis
{
    template class Singleton<JSONThingsHandle>;
}
