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

#include <fstream>
#include <paludis/match_sequence.hh>
#include <paludis/qa/defaults_check.hh>
#include <paludis/util/is_file_with_extension.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    enum State
    {
        st_default,
        st_src_compile,
        st_src_unpack
    };
}

DefaultsCheck::DefaultsCheck()
{
}

CheckResult
DefaultsCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    static const MatchRule r_echo(*MatchRule("\t") >> MatchRule("echo"));
    static const MatchRule r_einfo(*MatchRule("\t") >> MatchRule("einfo"));
    static const MatchRule r_colon(*MatchRule("\t") >> MatchRule(":"));
    static const MatchRule r_true(*MatchRule("\t") >> MatchRule("true"));
    static const MatchRule r_comment(*MatchRule("\t") >> MatchRule("#"));
    static const MatchRule r_econf(*MatchRule("\t") >> MatchRule("econf") >> *MatchRule(" ")
            >> (MatchRule::eol() || (MatchRule("||") >> *MatchRule(" ") >> MatchRule("die"))));
    static const MatchRule r_emake(*MatchRule("\t") >> MatchRule("emake") >> *MatchRule(" ")
            >> (MatchRule::eol() || (MatchRule("||") >> *MatchRule(" ") >> MatchRule("die"))));
    static const MatchRule r_unpack(*MatchRule("\t") >> MatchRule("unpack") >> *MatchRule(" ")
            >> (MatchRule("\"$A\"") || MatchRule("\"${A}\"") || MatchRule("$A") || MatchRule("${A}"))
            >> (MatchRule::eol() || (MatchRule("||") >> *MatchRule(" ") >> MatchRule("die"))));
    static const MatchRule r_cd_s(*MatchRule("\t") >> MatchRule("cd") >> *MatchRule(" ")
            >> (MatchRule("\"$S\"") || MatchRule("\"${S}\"") || MatchRule("$S") || MatchRule("${S}"))
            >> (MatchRule::eol() || (MatchRule("||") >> *MatchRule(" ") >> MatchRule("die"))));

    if (! f.is_regular_file())
        result << Message(qal_skip, "Not a regular file");
    else if (! IsFileWithExtension(".ebuild")(f.basename()))
        result << Message(qal_skip, "Not an ebuild file");
    else
    {
        std::ifstream ff(stringify(f).c_str());
        if (! ff)
            result << Message(qal_major, "Can't read file");
        else
        {
            State state(st_default);
            std::string line;
            bool src_compile_changed(false), src_unpack_changed(false);

            while (std::getline(ff, line))
            {
                switch (state)
                {
                    case st_default:
                        {
                            if (line == "src_compile() {")
                                state = st_src_compile;
                            else if (line == "src_unpack() {")
                                state = st_src_unpack;
                        }
                        continue;

                    case st_src_compile:
                        {
                            if (line == "}")
                            {
                                state = st_default;
                                if (! src_compile_changed)
                                    result << Message(qal_minor, "src_compile is redundant");
                            }
                            else if (line.empty())
                                ;
                            else if (r_econf.match(line))
                                ;
                            else if (r_emake.match(line))
                                ;
                            else if (r_echo.match(line))
                                ;
                            else if (r_einfo.match(line))
                                ;
                            else if (r_colon.match(line))
                                ;
                            else if (r_true.match(line))
                                ;
                            else if (r_comment.match(line))
                                ;
                            else
                                src_compile_changed = true;
                        }
                        continue;

                    case st_src_unpack:
                        {
                            if (line == "}")
                            {
                                state = st_default;
                                if (! src_unpack_changed)
                                    result << Message(qal_minor, "src_unpack is redundant");
                            }
                            else if (line.empty())
                                ;
                            else if (r_unpack.match(line))
                                ;
                            else if (r_cd_s.match(line))
                                ;
                            else if (r_echo.match(line))
                                ;
                            else if (r_einfo.match(line))
                                ;
                            else if (r_colon.match(line))
                                ;
                            else if (r_true.match(line))
                                ;
                            else if (r_comment.match(line))
                                ;
                            else
                                src_unpack_changed = true;
                        }
                        continue;
                }

                throw InternalError(PALUDIS_HERE, "bad state");
            }
        }
    }

    return result;
}

const std::string &
DefaultsCheck::identifier()
{
    static const std::string id("defaults");
    return id;
}


