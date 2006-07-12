/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/qa/file_permissions_check.hh>

using namespace paludis;
using namespace paludis::qa;

FilePermissionsCheck::FilePermissionsCheck()
{
}

CheckResult
FilePermissionsCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (f.is_directory())
    {
        if (! f.has_permission(fs_ug_owner, fs_perm_read))
            result << Message(qal_minor, "Directory owner does not have read permission");

        if (! f.has_permission(fs_ug_owner, fs_perm_execute))
            result << Message(qal_minor, "Directory owner does not have execute permission");
    }
    else
    {
        if (! f.has_permission(fs_ug_owner, fs_perm_read))
            result << Message(qal_minor, "File owner does not have read permission");

        if (f.has_permission(fs_ug_owner, fs_perm_execute))
            result << Message(qal_minor, "File owner has execute permission");
    }

    return result;
}

const std::string &
FilePermissionsCheck::identifier()
{
    static const std::string id("file_permissions");
    return id;
}

