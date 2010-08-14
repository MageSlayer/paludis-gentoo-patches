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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_SEARCH_EXTRAS_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_SEARCH_EXTRAS_HH 1

#include <paludis/util/attributes.hh>
#include <string>
#include <list>

struct CaveSearchExtrasDB;

extern "C" CaveSearchExtrasDB * cave_search_extras_create_db(const std::string &) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

extern "C" CaveSearchExtrasDB * cave_search_extras_open_db(const std::string &) PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

extern "C" void cave_search_extras_cleanup(CaveSearchExtrasDB * const) PALUDIS_VISIBLE;

extern "C" void cave_search_extras_starting_adds(CaveSearchExtrasDB * const) PALUDIS_VISIBLE;

extern "C" void cave_search_extras_add_candidate(CaveSearchExtrasDB * const, const std::string &,
        const bool, const bool, const bool, const std::string &, const std::string &, const std::string &) PALUDIS_VISIBLE;

extern "C" void cave_search_extras_done_adds(CaveSearchExtrasDB * const) PALUDIS_VISIBLE;

extern "C" void cave_search_extras_find_candidates(CaveSearchExtrasDB * const, std::list<std::string> &,
        const bool, const bool, const std::string &) PALUDIS_VISIBLE;

#endif
