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

#include "inherited_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/package_id.hh>
#include <paludis/util/fs_entry.hh>
#include <set>

using namespace paludis;

namespace
{
    struct InheritedBlacklist :
        InstantiationPolicy<InheritedBlacklist, instantiation_method::SingletonTag>
    {
        std::set<std::string> inherited_blacklist;

        InheritedBlacklist(const FSEntry & f = FSEntry(getenv_with_default("PALUDIS_QA_DATA_DIR",
                        stringify(FSEntry(DATADIR) / "paludis" / "qa"))) / "inherited_blacklist.conf")
        {
            try
            {
                LineConfigFile inherited_blacklist_file(f, LineConfigFileOptions());
                std::copy(inherited_blacklist_file.begin(), inherited_blacklist_file.end(),
                        std::inserter(inherited_blacklist, inherited_blacklist.end()));
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.qa.inherited_key_check.configuration_error", ll_warning, lc_context)
                    << "Got error '" << e.message() << "' (" << e.what()
                    << ") when loading inherited_blacklist.conf for QA inherited_key";
            }
        }
    };
}

bool
paludis::erepository::inherited_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using inherited_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.inherited_key_check", ll_debug, lc_context) << "inherited_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (id->inherited_key())
    {
        try
        {
            const std::set<std::string> & inherited_blacklist(InheritedBlacklist::get_instance()->inherited_blacklist);

            for (Set<std::string>::ConstIterator it(id->inherited_key()->value()->begin()),
                     it_end(id->inherited_key()->value()->end()); it_end != it; ++it)
                if (inherited_blacklist.end() != inherited_blacklist.find(*it))
                    reporter.message(QAMessage(entry, qaml_normal, name, "Deprecated inherit '" + *it + "' in '" + id->inherited_key()->raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, id->inherited_key()));
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            reporter.message(QAMessage(entry, qaml_severe, name,
                        "Caught exception '" + stringify(e.message()) + "' ("
                        + stringify(e.what()) + ") when handling key '" + id->inherited_key()->raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, id->inherited_key()));
        }
    }

    return true;
}

