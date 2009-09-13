/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/environments/paludis/action_to_string.hh>
#include <paludis/action.hh>

using namespace paludis;

namespace
{
    struct ActionToString
    {
        std::string visit(const InstallAction &) const
        {
            return "install";
        }

        std::string visit(const PretendFetchAction &) const
        {
            return "pretend-fetch";
        }

        std::string visit(const UninstallAction &) const
        {
            return "uninstall";
        }

        std::string visit(const FetchAction &) const
        {
            return "fetch";
        }

        std::string visit(const PretendAction &) const
        {
            return "pretend";
        }

        std::string visit(const InfoAction &) const
        {
            return "info";
        }

        std::string visit(const ConfigAction &) const
        {
            return "config";
        }
    };
}

const std::string
paludis::paludis_environment::action_to_string(const Action & a)
{
    ActionToString v;
    return a.accept_returning<std::string>(v);
}

