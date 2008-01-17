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
#include <paludis/util/tokeniser.hh>
#include <pcre++.h>
#include <time.h>
#include <vector>

using namespace paludis;

bool
paludis::erepository::header_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using default_functions on '" + stringify(*id) + "':");

    static pcrepp::Pcre::Pcre r_licence("^# Distributed under the terms of the GNU General Public License v2$");
    // Match both CVS tag and extract year.[0]
    static pcrepp::Pcre::Pcre r_cvs_header("^#\\s*\\$Header.*\\s(\\d{4})/\\d{2}/\\d{2}\\s.*\\$$");
    static pcrepp::Pcre::Pcre r_cvs_empty_header("^#\\s*\\$Header:\\s*\\$$");

    std::vector<std::string> lines;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(content, "\n", "", std::back_inserter(lines));

    do
    {
        if (! (lines.size() > 1 && r_licence.search(lines[1])))
            reporter.message(QAMessage(entry, qaml_normal, name, "Wrong licence statement in line 2")
                    .with_associated_id(id));

        std::string year;

        // Check line 3 before line 1 to extract date of last commit
        if (lines.size() > 2 && r_cvs_empty_header.search(lines[2]))
        {
            time_t now(time(NULL));
            struct tm now_struct;
            year = stringify(localtime_r(&now, &now_struct)->tm_year + 1900);
        }
        else if (lines.size() > 2 && r_cvs_header.search(lines[2]))
            year = r_cvs_header[0];
        else
        {
            reporter.message(QAMessage(entry, qaml_minor, name, "Unknown CVS tag in line 3")
                    .with_associated_id(id));
            break;
        }

        Log::get_instance()->message(ll_debug, lc_context, "Expected copyright year is " + year);
        pcrepp::Pcre::Pcre r_copyright("^# Copyright ((1999|200\\d)-)?" + year + " Gentoo Foundation$");

        if (! (lines.size() > 0 && r_copyright.search(lines[0])))
            reporter.message(QAMessage(entry, qaml_normal, name, "Wrong copyright assignment in line 1, possibly date related")
                    .with_associated_id(id));

    } while (false);

    return true;
}

