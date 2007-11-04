/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/e/qa/kv_variables.hh>
#include <paludis/qa.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <pcre++.h>
#include <list>

using namespace paludis;

bool
paludis::erepository::kv_variables_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using kv_variables on '" + stringify(*id) + "':");

    pcrepp::Pcre::Pcre r_global("^[a-zA-Z0-9\\_]+=.*\\$[{}]?KV");
    pcrepp::Pcre::Pcre r_detect_version("^detect_version$");

    Log::get_instance()->message(ll_debug, lc_context) << "kv_variables '"
        << entry << "', '" << *id << "', '" << name << "'";

    std::list<std::string> lines;
    Tokeniser<delim_kind::AnyOfTag>::tokenise(content, "\n", std::back_inserter(lines));

    unsigned line(0);
    for (std::list<std::string>::const_iterator l(lines.begin()), l_end(lines.end()) ;
            l != l_end ; ++l)
    {
        ++line;

        if (r_detect_version.search(*l))
            break;

        if (r_global.search(*l))
            reporter.message(QAMessage(entry, qaml_normal, name, "KV variable with no detect_version on line "
                        + stringify(line) + ": " + strip_leading(*l, " \t")));
    }

    return true;
}

