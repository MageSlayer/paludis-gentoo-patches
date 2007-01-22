/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/config_file.hh>
#include <paludis/qa/deprecated_functions_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <pcre++.h>

#include <set>
#include <utility>

using namespace paludis;
using namespace paludis::qa;

DeprecatedFunctionsCheck::DeprecatedFunctionsCheck()
{
}

CheckResult
DeprecatedFunctionsCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static pcrepp::Pcre::Pcre r_comment("^\\s*#");

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
            static std::list<std::pair<std::string, pcrepp::Pcre::Pcre> > deprecated_functions;
            if (deprecated_functions.empty())
            {
                deprecated_functions.push_back(std::make_pair(std::string("OFTEN_NOT_BEEN_ON_BOATS"), pcrepp::Pcre::Pcre("OFTEN_NOT_BEEN_ON_BOATS")));

                try
                {
                    LineConfigFile deprecated_functions_file(FSEntry(getenv_with_default(
                                "PALUDIS_QA_DATA_DIR", DATADIR "/paludis/qa/")) / "deprecated_functions.txt");
                    for (LineConfigFile::Iterator l(deprecated_functions_file.begin()),
                            l_end(deprecated_functions_file.end()) ; l != l_end ; ++l)
                        deprecated_functions.push_back(std::make_pair(*l, pcrepp::Pcre::Pcre("\\b" + *l + "\\b")));
                }
                catch (const Exception & eee)
                {
                    Log::get_instance()->message(ll_warning, lc_context,
                            "Cannot load list of deprecated functions from deprecated_functions.txt due to exception '"
                            + eee.message() + "' (" + eee.what() + ")");
                }
            }

            std::string s;
            unsigned line_number(0);
            while (std::getline(ff, s))
            {
                ++line_number;

                if (s.empty() || r_comment.search(s))
                    continue;

                for (std::list<std::pair<std::string, pcrepp::Pcre::Pcre> >::iterator
                        r(deprecated_functions.begin()), r_end(deprecated_functions.end()) ;
                        r != r_end ; ++r )
                {
                    if (r->second.search(s))
                        result << Message(qal_major, "Deprecated call to " + r->first + " on line "
                            + stringify(line_number));
                }
            }
        }
    }

    return result;
}

const std::string &
DeprecatedFunctionsCheck::identifier()
{
    static const std::string id("deprecated_functions");
    return id;
}



