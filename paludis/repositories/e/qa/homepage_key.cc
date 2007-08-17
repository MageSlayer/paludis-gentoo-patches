/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "homepage_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct HomepageChecker :
        ConstVisitor<URISpecTree>,
        ConstVisitor<URISpecTree>::VisitConstSequence<HomepageChecker, AllDepSpec>,
        ConstVisitor<URISpecTree>::VisitConstSequence<HomepageChecker, UseDepSpec>
    {
        using ConstVisitor<URISpecTree>::VisitConstSequence<HomepageChecker, UseDepSpec>::visit_sequence;
        using ConstVisitor<URISpecTree>::VisitConstSequence<HomepageChecker, AllDepSpec>::visit_sequence;

        const FSEntry entry;
        QAReporter & reporter;
        const tr1::shared_ptr<const ERepositoryID> id;
        const std::string name;
        bool found_one;

        HomepageChecker(
                const FSEntry & f,
                QAReporter & r,
                const tr1::shared_ptr<const ERepositoryID> & i,
                const std::string & n) :
            entry(f),
            reporter(r),
            id(i),
            name(n),
            found_one(false)
        {
        }

        ~HomepageChecker()
        {
            if (! found_one)
                reporter.message(QAMessage(entry, qaml_normal, name, "Homepage specifies no URIs"));
        }

        void visit_leaf(const URIDepSpec & u)
        {
            found_one = true;

            if (! u.renamed_url_suffix().empty())
                reporter.message(
                        QAMessage(entry, qaml_normal, name, "Homepage uses -> in part '" + u.text() + "'"));

            if (0 == u.original_url().compare(0, 7, "http://") &&
                    0 == u.original_url().compare(0, 8, "https://") &&
                    0 == u.original_url().compare(0, 6, "ftp://"))
                reporter.message(QAMessage(entry, qaml_normal, name, "Homepage uses no or unknown protocol in part '" + u.text() + "'"));
        }

        void visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> &)
        {
            reporter.message(QAMessage(entry, qaml_normal, name, "Homepage uses labels"));
        }
    };
}

bool
paludis::erepository::homepage_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const ERepositoryID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using homepage_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "homepage_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (! id->homepage_key())
        reporter.message(QAMessage(entry, qaml_normal, name, "No homepage available"));
    else
    {
        HomepageChecker h(entry, reporter, id, name);
        id->homepage_key()->value()->accept(h);
    }

    return true;
}


