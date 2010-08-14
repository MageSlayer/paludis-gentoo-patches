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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_SEARCH_EXTRAS_HANDLE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_SEARCH_EXTRAS_HANDLE_HH 1

#include <paludis/util/singleton.hh>
#include <list>
#include <string>

struct CaveSearchExtrasDB;

namespace paludis
{
    namespace cave
    {
        struct SearchExtrasHandle :
            Singleton<SearchExtrasHandle>
        {
            typedef CaveSearchExtrasDB * (* CreateDBFunction)(const std::string &);
            typedef CaveSearchExtrasDB * (* OpenDBFunction)(const std::string &);

            typedef void (* CleanupDBFunction)(CaveSearchExtrasDB * const);

            typedef void (* AddCandidateFunction)(CaveSearchExtrasDB * const, const std::string &,
                    const bool, const bool, const bool, const std::string &, const std::string &, const std::string &);
            typedef void (* StartingAddsFunction)(CaveSearchExtrasDB * const);
            typedef void (* DoneAddsFunction)(CaveSearchExtrasDB * const);

            typedef void (* FindCandidatesFunction)(CaveSearchExtrasDB * const, std::list<std::string> &,
                    const bool, const bool, const std::string &);

            void * handle;

            CreateDBFunction create_db_function;
            OpenDBFunction open_db_function;

            CleanupDBFunction cleanup_db_function;

            StartingAddsFunction starting_adds_function;
            AddCandidateFunction add_candidate_function;
            DoneAddsFunction done_adds_function;

            FindCandidatesFunction find_candidates_function;

            SearchExtrasHandle();
            ~SearchExtrasHandle();
        };
    }
}

#endif
