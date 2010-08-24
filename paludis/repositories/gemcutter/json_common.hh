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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_COMMON_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMCUTTER_JSON_COMMON_HH 1

#include <paludis/repositories/gemcutter/json_common-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_name> name;
        typedef Name<struct version_name> version;
    }

    namespace gemcutter_repository
    {
        struct GemJSONInfo
        {
            NamedValue<n::name, std::string> name;
            NamedValue<n::version, std::string> version;
        };

        class PALUDIS_VISIBLE JSONError :
            public Exception
        {
            public:
                JSONError(const std::string &) throw ();
        };
    }
}

#endif
