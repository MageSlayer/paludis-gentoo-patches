/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
        ConstVisitor<SimpleURISpecTree>,
        ConstVisitor<SimpleURISpecTree>::VisitConstSequence<HomepageChecker, AllDepSpec>,
        ConstVisitor<SimpleURISpecTree>::VisitConstSequence<HomepageChecker, ConditionalDepSpec>
    {
        using ConstVisitor<SimpleURISpecTree>::VisitConstSequence<HomepageChecker, ConditionalDepSpec>::visit_sequence;
        using ConstVisitor<SimpleURISpecTree>::VisitConstSequence<HomepageChecker, AllDepSpec>::visit_sequence;

        const std::tr1::shared_ptr<const MetadataKey> & key;
        const FSEntry entry;
        QAReporter & reporter;
        const std::tr1::shared_ptr<const PackageID> id;
        const std::string name;
        bool found_one;

        HomepageChecker(
                const std::tr1::shared_ptr<const MetadataKey> & k,
                const FSEntry & f,
                QAReporter & r,
                const std::tr1::shared_ptr<const PackageID> & i,
                const std::string & n) :
            key(k),
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
                reporter.message(QAMessage(entry, qaml_normal, name, "Homepage specifies no URIs")
                        .with_associated_id(id)
                        .with_associated_key(id, key));
        }

        void visit_leaf(const SimpleURIDepSpec & u)
        {
            found_one = true;

            if (0 != u.text().compare(0, 7, "http://") &&
                    0 != u.text().compare(0, 8, "https://") &&
                    0 != u.text().compare(0, 6, "ftp://"))
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Homepage uses no or unknown protocol in part '" + u.text() + "'")
                        .with_associated_id(id)
                        .with_associated_key(id, key));
        }
    };
}

bool
paludis::erepository::homepage_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using homepage_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.homepage_key_check", ll_debug, lc_context) << "homepage_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (! id->homepage_key())
        reporter.message(QAMessage(entry, qaml_normal, name, "No homepage available")
                        .with_associated_id(id));
    else
    {
        HomepageChecker h(id->homepage_key(), entry, reporter, id, name);
        id->homepage_key()->value()->accept(h);
    }

    return true;
}


