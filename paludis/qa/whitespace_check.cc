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

#include "whitespace_check.hh"
#include <paludis/is_file_with_extension.hh>
#include <fstream>

using namespace paludis;
using namespace paludis::qa;

WhitespaceCheck::WhitespaceCheck()
{
}

CheckResult
WhitespaceCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else if (! IsFileWithExtension(".ebuild")(f.basename()) &&
            ! IsFileWithExtension(".xml")(f.basename()))
        result << Message(qal_skip, "Not an ebuild or xml file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read file");
        else
        {
            std::string s;
            unsigned line_number(0);
            unsigned err_count(0);
            while (std::getline(ff, s))
            {
                if (err_count >= 3)
                {
                    result << Message(qal_info, "Skipping further whitespace checks");
                    break;
                }

                ++line_number;

                if (s.empty())
                    continue;

                if (' ' == s.at(0))
                {
                    result << Message(qal_minor, "Spaces for indenting on line "
                            + stringify(line_number));
                    ++err_count;
                    continue;
                }

                if ('\t' == s.at(0))
                {
                    std::string::size_type p(s.find_first_not_of("\t"));
                    if (std::string::npos == p)
                    {
                        result << Message(qal_minor, "Indent followed by no content on line "
                                + stringify(line_number));
                        ++err_count;
                        continue;
                    }

                    if (' ' == s.at(p))
                    {
                        result << Message(qal_minor, "Mixed spaces and tabs on line "
                                + stringify(line_number));
                        ++err_count;
                        continue;
                    }

                    p = s.find(p, '\t');
                    if (std::string::npos != p)
                    {
                        result << Message(qal_minor, "Non-indent tab found on line "
                                + stringify(line_number));
                        ++err_count;
                        continue;
                    }
                }

                if (' ' == s.at(s.length() - 1) || '\t' == s.at(s.length() - 1))
                {
                    result << Message(qal_minor, "Trailing whitespace found on line "
                            + stringify(line_number));
                    ++err_count;
                    continue;
                }
            }
        }
    }

    return result;
}

const std::string &
WhitespaceCheck::identifier()
{
    static const std::string id("whitespace");
    return id;
}


