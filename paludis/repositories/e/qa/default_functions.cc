/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include "default_functions.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <pcre++.h>
#include <sstream>

using namespace paludis;

namespace
{
    enum State
    {
        st_default,
        st_src_compile,
        st_src_unpack
    };
}

bool
paludis::erepository::default_functions_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using default_functions_check on '" + stringify(*id) + "':");

    pcrepp::Pcre::Pcre r_echo("^\\s*(echo|einfo|ewarn)[^|<>]*$");
    pcrepp::Pcre::Pcre r_colon("^\\s*:");
    pcrepp::Pcre::Pcre r_true("^\\s*true");
    pcrepp::Pcre::Pcre r_comment("^\\s*#");
    pcrepp::Pcre::Pcre r_econf("^\\s*econf( *\\|\\| *die.*)?$");
    pcrepp::Pcre::Pcre r_emake("^\\s*emake( *\\|\\| *die.*)?$");
    pcrepp::Pcre::Pcre r_unpack("^\\s*unpack *([$]A|[$][{]A[}]|\"[$][{]A[}]\"|[$][{]A[}])( *\\|\\| *die.*)?$");
    pcrepp::Pcre::Pcre r_cd_s("^\\s*cd *([$]S|[$][{]S[}]|\"[$][{]S[}]\"|[$][{]S[}])( *\\|\\| *die.*)?$");

    Log::get_instance()->message(ll_debug, lc_context) << "default_functions '"
        << entry << "', '" << *id << "', '" << name << "'";

    std::istringstream ff(content);

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
                            reporter.message(QAMessage(entry, qaml_minor, name, "src_compile is redundant")
                                    .with_associated_id(id));
                    }
                    else if (line.empty())
                        ;
                    else if (r_econf.search(line))
                        ;
                    else if (r_emake.search(line))
                        ;
                    else if (r_echo.search(line))
                        ;
                    else if (r_colon.search(line))
                        ;
                    else if (r_true.search(line))
                        ;
                    else if (r_comment.search(line))
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
                            reporter.message(QAMessage(entry, qaml_minor, name, "src_unpack is redundant")
                                    .with_associated_id(id));
                    }
                    else if (line.empty())
                        ;
                    else if (r_unpack.search(line))
                        ;
                    else if (r_cd_s.search(line))
                        ;
                    else if (r_echo.search(line))
                        ;
                    else if (r_colon.search(line))
                        ;
                    else if (r_true.search(line))
                        ;
                    else if (r_comment.search(line))
                        ;
                    else
                        src_unpack_changed = true;
                }
                continue;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }

    return true;
}

