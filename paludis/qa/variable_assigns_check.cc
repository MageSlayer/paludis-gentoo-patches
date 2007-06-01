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

#include <fstream>
#include <paludis/qa/variable_assigns_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/stringify.hh>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::qa;

VariableAssignsCheck::VariableAssignsCheck()
{
}

CheckResult
VariableAssignsCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_comment("^\\s*#");
    static pcrepp::Pcre::Pcre r_make_line("^\\s*e?make\\b");
    static pcrepp::Pcre::Pcre r_make_continuation_line("\\\\\\s*$");
    static pcrepp::Pcre::Pcre r_strip_quotes("(\"(\\\\.|[^\"])+\"|'(\\\\.|[^'])+')", "g");

    static std::map<std::string, pcrepp::Pcre::Pcre> r_vars;
    if (r_vars.empty())
    {
        r_vars.insert(std::make_pair("CFLAGS", pcrepp::Pcre::Pcre("\\bCFLAGS=")));
        r_vars.insert(std::make_pair("CXXFLAGS", pcrepp::Pcre::Pcre("\\bCXXFLAGS=")));
        r_vars.insert(std::make_pair("CPPFLAGS", pcrepp::Pcre::Pcre("\\bCPPFLAGS=")));
        r_vars.insert(std::make_pair("LDFLAGS", pcrepp::Pcre::Pcre("\\bLDFLAGS=")));
        r_vars.insert(std::make_pair("ASFLAGS", pcrepp::Pcre::Pcre("\\bASFLAGS=")));
    }

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else if (! is_file_with_extension(f, ".ebuild", IsFileWithOptions()) &&
            ! is_file_with_extension(f, ".eclass", IsFileWithOptions()))
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
            bool in_make_continuation(false);
            while (std::getline(ff, s))
            {
                ++line_number;
                if (s.empty() || r_comment.search(s))
                    continue;

                s = r_strip_quotes.replace(s, "");

                if (r_make_line.search(s))
                {
                    if (r_make_continuation_line.search(s))
                        in_make_continuation = true;
                    continue;
                }

                if (in_make_continuation)
                {
                    in_make_continuation = r_make_continuation_line.search(s);
                    continue;
                }

                for (std::map<std::string, pcrepp::Pcre::Pcre>::iterator r(r_vars.begin()),
                        r_end(r_vars.end()) ; r != r_end ; ++r)
                    if (r->second.search(s))
                    {
                        result << Message(qal_major, "Attempting to assign to " + r->first + " on line "
                                + stringify(line_number));
                        continue;
                    }
            }
        }
    }

    return result;
}

const std::string &
VariableAssignsCheck::identifier()
{
    static const std::string id("variable_assigns");
    return id;
}



