/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include "root_variable.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <pcrecpp.h>
#include <list>
#include <sstream>

using namespace paludis;

namespace
{
    enum State
    {
        st_default,
        st_in_src
    };
}

bool
paludis::erepository::root_variable_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using root_variable_check on '" + stringify(*id) + "':");

    pcrecpp::RE r_root("\\$[{]?ROOT[}]?([^=a-zA-Z0-9]|$)");
    pcrecpp::RE r_start("^src_");
    pcrecpp::RE r_end("^}");

    Log::get_instance()->message("e.qa.root_variable_check", ll_debug, lc_context) << "root_variable '"
        << entry << "', '" << *id << "', '" << name << "'";

    std::stringstream ff(content);

    State state(st_default);
    std::string line, func;
    unsigned line_number(0);

    while (std::getline(ff, line))
    {
        ++line_number;

        switch (state)
        {
            case st_default:
                {
                    if (r_start.PartialMatch(line))
                    {
                        state = st_in_src;
                        func = line;
                        if (std::string::npos != func.find('('))
                            func = func.substr(0, func.find('('));
                    }
                }
                continue;

            case st_in_src:
                {
                    if (r_end.PartialMatch(line))
                        state = st_default;
                    else if (r_root.PartialMatch(line))
                        reporter.message(QAMessage(entry, qaml_maybe, name, "ROOT abuse in " + func + " on line "
                                + stringify(line_number) + ": " + strip_leading(line, " \t"))
                                .with_associated_id(id));
                }
                continue;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }

    return true;
}

