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

#include "deprecated_functions.hh"
#include <paludis/qa.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/system.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <list>
#include <string>
#include <sstream>
#include <utility>
#include <pcre++.h>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct DeprecatedFunctions :
        InstantiationPolicy<DeprecatedFunctions, instantiation_method::SingletonTag>
    {
        std::list<std::string> deprecated_functions;

        DeprecatedFunctions(const FSEntry & f = FSEntry(getenv_with_default("PALUDIS_QA_DATA_DIR",
                        stringify(FSEntry(DATADIR) / "paludis" / "qa"))) / "deprecated_functions.conf")
        {
            try
            {
                LineConfigFile deprecated_functions_file(f, LineConfigFileOptions());
                std::copy(deprecated_functions_file.begin(), deprecated_functions_file.end(),

                        std::back_inserter(deprecated_functions));
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Got error '" << e.message() << "' (" << e.what()
                    << ") when loading deprecated_functions.conf for QA deprecated_functions";
            }
        }
    };

    QAMessage
    with_id(QAMessage m, const tr1::shared_ptr<const PackageID> & id)
    {
        return id ? m.with_associated_id(id) : m;
    }
}

bool
paludis::erepository::deprecated_functions_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using deprecated_functions_check on '" + (id ? stringify(*id) : stringify(entry)) + "':");

    pcrepp::Pcre::Pcre r_comment("^\\s*#");
    std::list<std::pair<std::string, pcrepp::Pcre::Pcre> > deprecated_functions;
    for (std::list<std::string>::const_iterator
             it(DeprecatedFunctions::get_instance()->deprecated_functions.begin()),
             it_end(DeprecatedFunctions::get_instance()->deprecated_functions.end());
             it_end != it; ++it)
        deprecated_functions.push_back(std::make_pair(*it, pcrepp::Pcre::Pcre(*it)));

    if (id)
        Log::get_instance()->message(ll_debug, lc_context) << "deprecated_functions '"
            << entry << "', '" << *id << "', '" << name << "'";
    else
        Log::get_instance()->message(ll_debug, lc_context) << "deprecated_functions '"
            << entry << "', '" << name << "'";

    std::stringstream ff(content);

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
            if (r->second.search(s))
                reporter.message(with_id(QAMessage(entry, qaml_normal, name,
                            "Deprecated call to '" + r->first + "' on line " + stringify(line_number)), id));
    }

    return true;
}

