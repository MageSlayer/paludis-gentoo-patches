/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/qa/has_ebuilds_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/dir_iterator.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

HasEbuildsCheck::HasEbuildsCheck()
{
}

CheckResult
HasEbuildsCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());

    if (DirIterator() == std::find_if(DirIterator(d), DirIterator(),
                IsFileWithExtension(d.basename() + "-", ".ebuild")))
        result << Message(qal_fatal, "No ebuilds found");

    return result;
}

const std::string &
HasEbuildsCheck::identifier()
{
    static const std::string id("has ebuilds");
    return id;
}

