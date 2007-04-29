/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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

#include <paludis/name.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/qa/ebuild_name_check.hh>

using namespace paludis;
using namespace paludis::qa;

EbuildNameCheck::EbuildNameCheck()
{
}

CheckResult
EbuildNameCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (is_file_with_extension(f, ".ebuild", IsFileWithOptions()))
    {
        if (stringify(f.dirname().basename()) != stringify(f.basename()).substr(
                    0, stringify(f.dirname().basename()).length()))
            result << Message(qal_fatal, "Ebuild name does not match directory name");
    }

    return result;
}

const std::string &
EbuildNameCheck::identifier()
{
    static const std::string id("ebuild_name");
    return id;
}

