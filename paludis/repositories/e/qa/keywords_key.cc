/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "keywords_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/log.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;

bool
paludis::erepository::keywords_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using keywords_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.keywords_key_check", ll_debug, lc_context) << "keywords_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (id->keywords_key())
    {
        try
        {
            const Set<KeywordName> & keywords(*id->keywords_key()->value());

            if (1 == keywords.size() && keywords.end() != keywords.find(KeywordName("-*")))
                reporter.message(QAMessage(entry, qaml_normal, name,
                        "-* abuse in '" + id->keywords_key()->raw_name() + "' (use package.mask and keyword properly)")
                                .with_associated_id(id)
                                .with_associated_key(id, id->keywords_key()));

            else if (keywords.empty())
                reporter.message(QAMessage(entry, qaml_normal, name,
                        "Empty '" + id->keywords_key()->raw_name() + "' (use package.mask and keyword properly)")
                                .with_associated_id(id)
                                .with_associated_key(id, id->keywords_key()));
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            reporter.message(QAMessage(entry, qaml_severe, name,
                        "Caught exception '" + stringify(e.message()) + "' ("
                        + stringify(e.what()) + ") when handling key '" + id->keywords_key()->raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, id->keywords_key()));
        }
    }

    return true;
}

