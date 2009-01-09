/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include "license_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/package_id.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Checker
    {
        const FSEntry & entry;
        QAReporter & reporter;
        std::tr1::shared_ptr<const ERepositoryID> id;
        std::tr1::shared_ptr<const FSEntrySequence> dirs;
        std::string name;

        Checker(const FSEntry & e, QAReporter & r, const std::tr1::shared_ptr<const ERepositoryID> & p,
                const std::tr1::shared_ptr<const FSEntrySequence> d, const std::string & n) :
            entry(e),
            reporter(r),
            id(p),
            dirs(d),
            name(n)
        {
        }

        void visit(const LicenseSpecTree::NodeType<LicenseDepSpec>::Type & node)
        {
            for (FSEntrySequence::ConstIterator it(dirs->begin()),
                     it_end(dirs->end()); it_end != it; ++it)
                if (((*it) / node.spec()->text()).is_regular_file_or_symlink_to_regular_file())
                    return;

            reporter.message(QAMessage(entry, qaml_normal, name,
                        "Item '" + node.spec()->text() + "' in '" + id->license_key()->raw_name() + "' is not a licence")
                            .with_associated_id(id)
                            .with_associated_key(id, id->license_key()));
        }

        void visit(const LicenseSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const LicenseSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };
}

bool
paludis::erepository::license_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const ERepository> & repo,
        const std::tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using license_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.license_key_check", ll_debug, lc_context) << "license_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (id->license_key())
    {
        try
        {
            Checker c(entry, reporter, id, repo->layout()->licenses_dirs(), name);
            id->license_key()->value()->root()->accept(c);
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            reporter.message(QAMessage(entry, qaml_severe, name,
                        "Caught exception '" + stringify(e.message()) + "' ("
                        + stringify(e.what()) + ") when handling key '" + id->license_key()->raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, id->license_key()));
        }
    }

    return true;
}

