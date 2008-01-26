/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include "function_keyword.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <pcre++.h>
#include <list>
#include <sstream>

using namespace paludis;

namespace
{
    QAMessage
    with_id(QAMessage m, const tr1::shared_ptr<const PackageID> & id)
    {
        return id ? m.with_associated_id(id) : m;
    }
}

bool
paludis::erepository::function_keyword_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using function_keyword_check on '" + (id ? stringify(*id) : stringify(entry)) + "':");

    pcrepp::Pcre::Pcre r_function("^function +[^ ]+ *(\\(\\))? *{?");

    if (id)
        Log::get_instance()->message(ll_debug, lc_context) << "function_keyword '"
            << entry << "', '" << *id << "', '" << name << "'";
    else
        Log::get_instance()->message(ll_debug, lc_context) << "function_keyword '"
            << entry << "', '" << name << "'";

    std::stringstream ff(content);

    std::string s;
    unsigned line_number(0);
    while (std::getline(ff, s))
    {
        ++line_number;

        if (s.empty())
            continue;

        if (r_function.search(s))
            reporter.message(with_id(QAMessage(entry, qaml_minor, name, "Use of the keyword 'function' on line " + stringify(line_number)), id));
    }

    return true;
}

