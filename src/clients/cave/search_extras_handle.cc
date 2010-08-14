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

#include "search_extras_handle.hh"

#include <paludis/util/singleton-impl.hh>

#include <paludis/args/do_help.hh>

#include <paludis/about.hh>

#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;
using namespace cave;

SearchExtrasHandle::SearchExtrasHandle() :
    handle(0),
    create_db_function(0),
    open_db_function(0),
    cleanup_db_function(0),
    starting_adds_function(0),
    add_candidate_function(0),
    done_adds_function(0),
    find_candidates_function(0)
{
    handle = ::dlopen(("libcavesearchextras_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (! handle)
        throw args::DoHelp("Search index creation not available because dlopen said " + stringify(::dlerror()));

    create_db_function = STUPID_CAST(CreateDBFunction, ::dlsym(handle, "cave_search_extras_create_db"));
    if (! create_db_function)
        throw args::DoHelp("Search index creation not available because dlsym said " + stringify(::dlerror()));

    open_db_function = STUPID_CAST(CreateDBFunction, ::dlsym(handle, "cave_search_extras_open_db"));
    if (! open_db_function)
        throw args::DoHelp("Search index not available because dlsym said " + stringify(::dlerror()));

    cleanup_db_function = STUPID_CAST(CleanupDBFunction, ::dlsym(handle, "cave_search_extras_cleanup"));
    if (! cleanup_db_function)
        throw args::DoHelp("Search index not available because dlsym said " + stringify(::dlerror()));

    add_candidate_function = STUPID_CAST(AddCandidateFunction, ::dlsym(handle, "cave_search_extras_add_candidate"));
    if (! add_candidate_function)
        throw args::DoHelp("Search index creation not available because dlsym said " + stringify(::dlerror()));

    starting_adds_function = STUPID_CAST(StartingAddsFunction, ::dlsym(handle, "cave_search_extras_starting_adds"));
    if (! starting_adds_function)
        throw args::DoHelp("Search index creation not available because dlsym said " + stringify(::dlerror()));

    done_adds_function = STUPID_CAST(DoneAddsFunction, ::dlsym(handle, "cave_search_extras_done_adds"));
    if (! done_adds_function)
        throw args::DoHelp("Search index creation not available because dlsym said " + stringify(::dlerror()));

    find_candidates_function = STUPID_CAST(FindCandidatesFunction, ::dlsym(handle, "cave_search_extras_find_candidates"));
    if (! find_candidates_function)
        throw args::DoHelp("Search index not available because dlsym said " + stringify(::dlerror()));
}

SearchExtrasHandle::~SearchExtrasHandle()
{
    if (handle)
        ::dlclose(handle);
}

template class Singleton<SearchExtrasHandle>;

