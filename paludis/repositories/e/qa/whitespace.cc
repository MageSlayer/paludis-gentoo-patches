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

#include <paludis/repositories/e/qa/whitespace.hh>
#include <paludis/qa.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <list>

using namespace paludis;

bool
paludis::erepository::whitespace_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using whitespace_check on '" + stringify(*id) + "':");

    Log::get_instance()->message(ll_debug, lc_context) << "whitespace '"
        << entry << "', '" << *id << "', '" << name << "'";

    std::list<std::string> lines;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(content, "\n", "", std::back_inserter(lines));

    unsigned line(0), err_count(0);
    for (std::list<std::string>::const_iterator l(lines.begin()), l_end(lines.end()) ;
            l != l_end ; ++l)
    {
        ++line;

        if (l->empty())
            continue;

        if (err_count >= 3)
        {
            reporter.message(QAMessage(entry, qaml_minor, name, "Skipping further whitespace checks")
                    .with_associated_id(id));
            break;
        }

        if (' ' == l->at(0))
        {
            reporter.message(QAMessage(entry, qaml_minor, name, "Spaces for indenting on line "
                        + stringify(line) + ": " + strip_leading(*l, " \t"))
                    .with_associated_id(id));
            ++err_count;
            continue;
        }
        else if ('\t' == l->at(0))
        {
            std::string::size_type p(l->find_first_of("\t"));
            if (std::string::npos == p)
            {
                reporter.message(QAMessage(entry, qaml_minor, name, "Indent followed by no content on line "
                            + stringify(line) + ": " + strip_leading(*l, " \t"))
                        .with_associated_id(id));
                ++err_count;
                continue;
            }
            else if (' ' == l->at(p))
            {
                reporter.message(QAMessage(entry, qaml_minor, name, "Mixed tabs and spaces for indenting on line "
                            + stringify(line) + ": " + strip_leading(*l, " \t"))
                        .with_associated_id(id));
                ++err_count;
                continue;
            }
            else if (std::string::npos != l->find(p, '\t'))
            {
                reporter.message(QAMessage(entry, qaml_minor, name, "Non-intent tab on line "
                            + stringify(line) + ": " + strip_leading(*l, " \t"))
                        .with_associated_id(id));
                ++err_count;
                continue;
            }
        }

        if (' ' == l->at(l->length() - 1) || '\t' == l->at(l->length() - 1))
        {
            reporter.message(QAMessage(entry, qaml_minor, name, "Trailing space on line "
                        + stringify(line) + ": " + strip_leading(*l, " \t"))
                    .with_associated_id(id));
            ++err_count;
            continue;
        }
    }

    return true;
}


