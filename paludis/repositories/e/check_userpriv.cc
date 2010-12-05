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

#include <paludis/repositories/e/check_userpriv.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/environment.hh>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::check_userpriv(const FSPath & f, const Environment * env, bool mandatory)
{
    Context c("When checking permissions on '" + stringify(f) + "' for userpriv:");

    if (! getenv_with_default("PALUDIS_BYPASS_USERPRIV_CHECKS", "").empty())
        return false;

    FSStat f_stat(f);
    if (f_stat.exists())
    {
        if (f_stat.group() != env->reduced_gid())
        {
            if (mandatory)
                throw ConfigurationError("Directory '" + stringify(f) + "' owned by group '" + get_group_name(f_stat.group())
                        + "', not '" + get_group_name(env->reduced_gid()) + "'");
            else
                Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                    f << "' owned by group '" << get_group_name(f_stat.group()) << "', not '"
                    << get_group_name(env->reduced_gid()) << "', so cannot enable userpriv";
            return false;
        }
        else if (0 == (f_stat.permissions() & S_IWGRP))
        {
            if (mandatory)
                throw ConfigurationError("Directory '" + stringify(f) + "' does not have group write permission");
            else
                Log::get_instance()->message("e.ebuild.userpriv_disabled", ll_warning, lc_context) << "Directory '" <<
                    f << "' does not have group write permission, cannot enable userpriv";
            return false;
        }
    }

    return true;
}
