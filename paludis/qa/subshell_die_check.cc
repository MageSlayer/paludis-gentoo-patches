/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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
#include <paludis/qa/subshell_die_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::qa;

SubshellDieCheck::SubshellDieCheck()
{
}

CheckResult
SubshellDieCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_comment("^\\s*#");
    static pcrepp::Pcre::Pcre r_subshell_die("\\([^\\)]*\\bdie\\b");

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else if (! IsFileWithExtension(".ebuild")(f) &&
            ! IsFileWithExtension(".eclass")(f))
        result << Message(qal_skip, "Not an ebuild or eclass file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read file");
        else
        {
            std::string s;
            unsigned line_number(0);
            while (std::getline(ff, s))
            {
                ++line_number;

                if (s.empty() || r_comment.search(s))
                    continue;

                if (r_subshell_die.search(s))
                {
                    result << Message(qal_major, "Invalid call of 'die' within subshell on line "
                            + stringify(line_number));
                    continue;
                }
            }
        }
    }

    return result;
}

const std::string &
SubshellDieCheck::identifier()
{
    static const std::string id("subshell_die");
    return id;
}



