/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <kugelfang@gentoo.org>
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
#include <sstream>
#include <paludis/qa/header_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::qa;

HeaderCheck::HeaderCheck()
{
}

CheckResult
HeaderCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_licence("^# Distributed under the terms of the GNU General Public License v2$");
    // Match both CVS tag and extract year.[0]
    static pcrepp::Pcre::Pcre r_cvs_header("^#\\s*\\$Header.*\\s(\\d{4})/\\d{2}/\\d{2}\\s.*\\$$");


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
            std::vector<std::string> lines;

            for (unsigned line_number(0) ; line_number < 3 ; ++line_number)
            {
                std::getline(ff, s);
                lines.push_back(s);
            }

            do
            {
                if (! r_licence.search(lines[1]))
                    result << Message(qal_major, "Wrong licence statement in line 2");

                // Check line 3 before line 1 to extract date of last commit
                if (! r_cvs_header.search(lines[2]))
                {
                    result << Message(qal_minor, "Unknown CVS tag in line 3");
                    break;
                }

                pcrepp::Pcre::Pcre r_copyright("^# Copyright ((1999|200\\d)-)?" + r_cvs_header[0] + " Gentoo Foundation$");

                if (! r_copyright.search(lines[0]))
                    result << Message(qal_major, "Wrong copyright assignment in line 1, possibly date related");

            } while (false);
        }
    }

    return result;
}

const std::string &
HeaderCheck::identifier()
{
    static const std::string id("header");
    return id;
}

