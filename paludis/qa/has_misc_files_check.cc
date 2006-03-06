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

#include <paludis/qa/has_misc_files_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/dir_iterator.hh>

using namespace paludis;
using namespace paludis::qa;

HasMiscFilesCheck::HasMiscFilesCheck()
{
}

CheckResult
HasMiscFilesCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());

    if (! (d / "ChangeLog").exists())
        result << Message(qal_major, "No ChangeLog found");

    if (! (d / "files").exists())
        result << Message(qal_major, "No files/ found");

    if (! (d / "metadata.xml").exists())
        result << Message(qal_major, "No metadata.xml found");

    return result;
}

const std::string &
HasMiscFilesCheck::identifier()
{
    static const std::string id("has misc files");
    return id;
}

