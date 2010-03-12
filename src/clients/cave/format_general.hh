/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMAT_GENERAL_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_FORMAT_GENERAL_HH 1

#include <paludis/util/attributes.hh>
#include <string>

namespace paludis
{
    namespace cave
    {
        std::string format_general_s(const std::string & f, const std::string & s)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_si(const std::string & f, const std::string & s, const int i)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_sr(const std::string & f, const std::string & s, const std::string & r)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_kv(const std::string & f, const std::string & k, const std::string & v)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_rhvib(const std::string & f, const std::string & r,
                const std::string & h, const std::string & v, const int i, const bool b)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_spad(const std::string & f, const std::string & s,
                const int p, const int a, const int d)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_i(const std::string & f, const int i)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));

        std::string format_general_his(const std::string & f, const std::string & h, const int i, const std::string & s)
            PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
    }
}

#endif
