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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_THINGS_HANDLE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_THINGS_HANDLE_HH 1

#include <paludis/repositories/gemcutter/json_things_handle-fwd.hh>
#include <paludis/repositories/gemcutter/json_things.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <string>

namespace paludis
{
    namespace gemcutter_repository
    {
        class JSONThingsHandle :
            public Singleton<JSONThingsHandle>
        {
            friend class Singleton<JSONThingsHandle>;

            private:
                void * handle;

                typedef void (* ParseAllGemsFunction)(const FSPath &, const ParsedOneGemCallback &);
                ParseAllGemsFunction parse_all_gems_function;

                JSONThingsHandle();
                ~JSONThingsHandle();

            public:
                void parse_all_gems(const FSPath &, const ParsedOneGemCallback &) const;
        };
    }

    extern template class Singleton<gemcutter_repository::JSONThingsHandle>;
}

#endif
