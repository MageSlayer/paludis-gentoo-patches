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

#include "ebuild_count.hh"
#include <paludis/is_file_with_extension.hh>
#include <paludis/dir_iterator.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

EbuildCountCheck::EbuildCountCheck()
{
}

CheckResult
EbuildCountCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());

    std::size_t count(std::count_if(DirIterator(d), DirIterator(), IsFileWithExtension(".ebuild")));
    if (count > 20)
        result << Message(qal_minor, "Found " + stringify(count) +
                " ebuilds, which is too many to count on both hands and both feet");
    else if (count > 15)
        result << Message(qal_minor, "Found " + stringify(count) +
                " ebuilds, which is too many to count on both hands and one foot");
    else if (count > 10)
        result << Message(qal_minor, "Found " + stringify(count) +
                " ebuilds, which is too many to count on my fingers");

    return result;
}

const std::string &
EbuildCountCheck::identifier()
{
    static const std::string id("ebuild count");
    return id;
}


