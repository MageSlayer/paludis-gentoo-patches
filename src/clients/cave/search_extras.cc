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

#include "search_extras.hh"
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <sqlite3.h>

using namespace paludis;

struct CaveSearchExtrasDB
{
    sqlite3 * db;
    sqlite3_stmt * add_candidate;
};

extern "C"
CaveSearchExtrasDB *
cave_search_extras_create_db(const std::string & file)
{
    auto data(cave_search_extras_open_db(file));

    if (SQLITE_OK != sqlite3_exec(data->db, "drop table if exists candidates", nullptr, nullptr, nullptr))
        throw InternalError(PALUDIS_HERE, "sqlite3_exec drop candidates failed");

    if (SQLITE_OK != sqlite3_exec(data->db, "create table candidates ( "
                "spec text not null primary key, "
                "is_visible int not null, "
                "is_best int not_null, "
                "is_best_visible int not_null, "
                "name text not null, "
                "short_desc text not null, "
                "long_desc text not null"
                ")", nullptr, nullptr, nullptr))
        throw InternalError(PALUDIS_HERE, "sqlite3_exec create candidates failed");

    if (SQLITE_OK != sqlite3_prepare_v2(data->db, "insert into candidates "
                "( spec, is_visible, is_best, is_best_visible, name, short_desc, long_desc ) "
                "values ( ?1, ?2, ?3, ?4, ?5, ?6, ?7 )",
                -1, &data->add_candidate, nullptr))
        throw InternalError(PALUDIS_HERE, "sqlite3_prepare_v2 insert into candidates failed");

    return data;
}

extern "C" CaveSearchExtrasDB *
cave_search_extras_open_db(const std::string & file)
{
    auto data(new CaveSearchExtrasDB);
    if (SQLITE_OK != sqlite3_open(file.c_str(), &data->db))
        throw InternalError(PALUDIS_HERE, "sqlite3_open failed");

    data->add_candidate = nullptr;

    return data;
}

extern "C"
void
cave_search_extras_cleanup(CaveSearchExtrasDB * const data)
{
    if (data->add_candidate)
        sqlite3_finalize(data->add_candidate);

    sqlite3_close(data->db);
    delete data;
}

extern "C"
void
cave_search_extras_add_candidate(
        CaveSearchExtrasDB * const data,
        const std::string & spec,
        const bool visible,
        const bool best,
        const bool best_visible,
        const std::string & name,
        const std::string & short_desc,
        const std::string & long_desc)
{
    if (SQLITE_OK != sqlite3_reset(data->add_candidate))
        throw InternalError(PALUDIS_HERE, "sqlite3_reset add candidate failed");
    if (SQLITE_OK != sqlite3_clear_bindings(data->add_candidate))
        throw InternalError(PALUDIS_HERE, "sqlite3_clear_bindings add candidate failed");

    if (SQLITE_OK != sqlite3_bind_text(data->add_candidate, 1, spec.c_str(), spec.length(), SQLITE_TRANSIENT))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 1 failed");
    if (SQLITE_OK != sqlite3_bind_int(data->add_candidate, 2, visible ? 1 : 0))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 2 failed");
    if (SQLITE_OK != sqlite3_bind_int(data->add_candidate, 3, best ? 1 : 0))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 3 failed");
    if (SQLITE_OK != sqlite3_bind_int(data->add_candidate, 4, best_visible ? 1 : 0))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 4 failed");
    if (SQLITE_OK != sqlite3_bind_text(data->add_candidate, 5, name.c_str(), name.length(), SQLITE_TRANSIENT))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 5 failed");
    if (SQLITE_OK != sqlite3_bind_text(data->add_candidate, 6, short_desc.c_str(), short_desc.length(), SQLITE_TRANSIENT))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 6 failed");
    if (SQLITE_OK != sqlite3_bind_text(data->add_candidate, 7, long_desc.c_str(), long_desc.length(), SQLITE_TRANSIENT))
        throw InternalError(PALUDIS_HERE, "sqlite3_bind_text add candidate 7 failed");

    int code;
    if (SQLITE_DONE != (code = sqlite3_step(data->add_candidate)))
        throw InternalError(PALUDIS_HERE, "sqlite3_step failed: " + stringify(code));
}

extern "C"
void
cave_search_extras_starting_adds(CaveSearchExtrasDB * const data)
{
    if (SQLITE_OK != sqlite3_exec(data->db, "begin", nullptr, nullptr, nullptr))
        throw InternalError(PALUDIS_HERE, "sqlite3_exec begin failed");
}

extern "C"
void
cave_search_extras_done_adds(CaveSearchExtrasDB * const data)
{
    if (SQLITE_OK != sqlite3_exec(data->db, "commit", nullptr, nullptr, nullptr))
        throw InternalError(PALUDIS_HERE, "sqlite3_exec commit failed");
}

extern "C"
void
cave_search_extras_find_candidates(CaveSearchExtrasDB * const data,
        std::list<std::string> & out,
        const bool all_versions, const bool visible,
        const std::string & name_description_substring_hint)
{
    sqlite3_stmt * find_candidates;

    std::string s;
    if (all_versions && visible)
        s = "is_visible";
    else if (visible)
        s = "is_best_visible";
    else if (all_versions)
        s = "1";
    else
        s = "is_best";

    std::string h;
    std::string p1;
    if (! name_description_substring_hint.empty())
    {
        h = " and ( name like ?1 escape '\\' or short_desc like ?1 escape '\\' or long_desc like ?1 escape '\\' )";

        p1 = "%";
        for (char i : name_description_substring_hint)
            switch (i)
            {
                case '%':
                case '_':
                case '\\':
                    p1.append(1, '\\');
                    /* fall through */
                default:
                    p1.append(1, i);
            }
        p1.append("%");
    }

    int code;
    if (SQLITE_OK != ((code = sqlite3_prepare_v2(data->db, ("select spec from candidates where " + s + " = 1" + h).c_str(), -1, &find_candidates, nullptr))))
        throw InternalError(PALUDIS_HERE, "sqlite3_prepare_v2 select from candidates failed:" + stringify(code));

    if (! p1.empty())
    {
        if (SQLITE_OK != sqlite3_bind_text(find_candidates, 1, p1.c_str(), p1.length(), SQLITE_TRANSIENT))
            throw InternalError(PALUDIS_HERE, "sqlite3_bind_text select from candidates 1 failed");
    }

    while (true)
    {
        code = sqlite3_step(find_candidates);

        if (code == SQLITE_DONE)
            break;
        else if (code == SQLITE_ROW)
        {
            std::string text(reinterpret_cast<const char *>(sqlite3_column_text(find_candidates, 0)));
            out.push_back(text);
        }
        else
            throw InternalError(PALUDIS_HERE, "sqlite3_step select from candidates failed:" + stringify(code));
    }

    sqlite3_finalize(find_candidates);
}

