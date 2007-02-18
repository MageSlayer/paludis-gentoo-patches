/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda <ferdy@gentoo.org>
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
#include <paludis/qa/function_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::qa;

FunctionCheck::FunctionCheck()
{
}

CheckResult
FunctionCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_function("^function +[^ ]+ *(\\(\\))? *{?");

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file.");
    else if (! IsFileWithExtension(".ebuild")(f) &&
            ! IsFileWithExtension(".eclass")(f))
        result << Message(qal_skip, "Not an ebuild or eclass.");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Could not read file.");
        else
        {
            std::string s;
            unsigned line_number(0);
            while (std::getline(ff, s))
            {
                ++line_number;

                if (s.empty())
                    continue;

                if (r_function.search(s))
                    result << Message(qal_minor, "Use of the keyword 'function' on line "
                            + stringify(line_number));
            }
        }
    }

    return result;
}

const std::string &
FunctionCheck::identifier()
{
    static const std::string id("function");
    return id;
}
