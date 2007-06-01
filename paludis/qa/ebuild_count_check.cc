/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/ebuild_count_check.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/stringify.hh>
#include <algorithm>
#include <paludis/util/tr1_functional.hh>

using namespace paludis;
using namespace paludis::qa;

EbuildCountCheck::EbuildCountCheck()
{
}

CheckResult
EbuildCountCheck::operator() (const FSEntry & d) const
{
    using namespace tr1::placeholders;

    CheckResult result(d, identifier());

    std::size_t count(std::count_if(DirIterator(d), DirIterator(),
                tr1::bind(&is_file_with_extension, _1, ".ebuild", IsFileWithOptions())));
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


