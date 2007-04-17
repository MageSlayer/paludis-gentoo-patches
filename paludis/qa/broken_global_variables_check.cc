/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/qa/broken_global_variables_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/strip.hh>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::qa;

BrokenGlobalVariablesCheck::BrokenGlobalVariablesCheck()
{
}

CheckResult
BrokenGlobalVariablesCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_global("^[a-zA-Z0-9\\_]+=.*\\$[{}]?KV");
    static pcrepp::Pcre::Pcre r_detect_version("^detect_version$");

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else if (! is_file_with_extension(f, ".ebuild", IsFileWithOptions()))
        result << Message(qal_skip, "Not an ebuild file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read file");
        else
        {
            std::string line, func;
            unsigned line_number(0);

            while (std::getline(ff, line))
            {
                ++line_number;

                if (r_detect_version.search(line))
                    break;

                if (r_global.search(line))
                    result << Message(qal_maybe, "Suspect global variable on line "
                            + stringify(line_number) + ": " + strip_leading(line, " \t"));
            }
        }
    }

    return result;
}

const std::string &
BrokenGlobalVariablesCheck::identifier()
{
    static const std::string id("broken_global_variables");
    return id;
}

