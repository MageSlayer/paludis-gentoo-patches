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

#include <fstream>
#include <paludis/qa/changelog_check.hh>
#include <paludis/util/pstream.hh>

using namespace paludis;
using namespace paludis::qa;

ChangeLogCheck::ChangeLogCheck()
{
}

CheckResult
ChangeLogCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (f.basename() != "ChangeLog")
        result << Message(qal_skip, "Not a ChangeLog");
    else if (! f.is_regular_file())
        result << Message(qal_major, "Not a regular file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        do
        {
            if (! ff)
            {
                result << Message(qal_major, "Can't read ChangeLog");
                continue;
            }

            std::string s;
            if (! std::getline(ff, s))
            {
                result << Message(qal_major, "Can't read ChangeLog header");
                continue;
            }

            if (s != "# ChangeLog for " + stringify(f.dirname().dirname().basename())
                    + "/" + stringify(f.dirname().basename()))
                result << Message(qal_minor, "ChangeLog header is incorrect");
        }
        while (false);
    }

    return result;
}

const std::string &
ChangeLogCheck::identifier()
{
    static const std::string id("changelog");
    return id;
}


