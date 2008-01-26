/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk
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

#include "header.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <pcre++.h>
#include <time.h>
#include <vector>
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
paludis::erepository::header_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using default_functions on '" + (id ? stringify(*id) : stringify(entry)) + "':");

    if (id)
        Log::get_instance()->message(ll_debug, lc_context) << "header '"
            << entry << "', '" << *id << "', '" << name << "'";
    else
        Log::get_instance()->message(ll_debug, lc_context) << "header '"
            << entry << "', '" << name << "'";

    pcrepp::Pcre::Pcre r_licence("^# Distributed under the terms of the GNU General Public License v2$");
    // Match both CVS tag and extract year.[0]
    pcrepp::Pcre::Pcre r_cvs_header("^#\\s*\\$Header.*\\s(\\d{4})/\\d{2}/\\d{2}\\s.*\\$$");
    pcrepp::Pcre::Pcre r_cvs_empty_header("^#\\s*\\$Header:\\s*\\$$");

    std::stringstream ff(content);

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
            reporter.message(with_id(QAMessage(entry, qaml_normal, name, "Wrong licence statement in line 2"), id));

        std::string year;

        // Check line 3 before line 1 to extract date of last commit
        if (r_cvs_empty_header.search(lines[2]))
        {
            time_t now(time(NULL));
            struct tm now_struct;
            year = stringify(localtime_r(&now, &now_struct)->tm_year + 1900);
        }
        else if (r_cvs_header.search(lines[2]))
            year = r_cvs_header[0];
        else
        {
            reporter.message(with_id(QAMessage(entry, qaml_minor, name, "Unknown CVS tag in line 3"), id));
            break;
        }

        Log::get_instance()->message(ll_debug, lc_context, "Expected copyright year is " + year);
        pcrepp::Pcre::Pcre r_copyright("^# Copyright ((1999|200\\d)-)?" + year + " Gentoo Foundation$");

        if (! r_copyright.search(lines[0]))
            reporter.message(with_id(QAMessage(entry, qaml_normal, name, "Wrong copyright assignment in line 1, possibly date related"), id));

    } while (false);

    return true;
}

