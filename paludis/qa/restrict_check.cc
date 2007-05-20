/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <algorithm>
#include <iterator>
#include <paludis/qa/restrict_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/qa/qa_environment.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>::visit;

        CheckResult & result;
        const std::set<std::string> & allowed;

        Checker(CheckResult & rr, const std::set<std::string> & a) :
            result(rr),
            allowed(a)
        {
        }

        void visit(const PackageDepSpec * const)
        {
            result << Message(qal_major, "Got a PackageDepSpec in RESTRICT");
        }

        void visit(const PlainTextDepSpec * const t)
        {
            if (allowed.end() == allowed.find(t->text()))
                result << Message(qal_major, "Unrecognised RESTRICT value '" + t->text() + "'");
        }

        void visit(const BlockDepSpec * const)
        {
            result << Message(qal_major, "Got a PackageDepSpec in RESTRICT");
        }

        void visit(const AnyDepSpec * const a)
        {
            result << Message(qal_major, "Got a || ( ) block in RESTRICT");
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
    };
}

RestrictCheck::RestrictCheck()
{
}

CheckResult
RestrictCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->main_repository()->name());
        tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        static std::set<std::string> allowed_restricts;
        if (allowed_restricts.empty())
        {
            allowed_restricts.insert("fetch");
            allowed_restricts.insert("mirror");
            allowed_restricts.insert("nomirror");
            allowed_restricts.insert("primaryuri");
            allowed_restricts.insert("nostrip");
            allowed_restricts.insert("strip");
            allowed_restricts.insert("sandbox");
            allowed_restricts.insert("userpriv");
            allowed_restricts.insert("test");
        }

        Checker c(result, allowed_restricts);
        metadata->ebuild_interface->restrictions()->accept(&c);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & err)
    {
        result << Message(qal_fatal, "Caught Exception '" + err.message() + "' ("
                + err.what() + ")");
    }

    return result;
}

const std::string &
RestrictCheck::identifier()
{
    static const std::string id("restrict");
    return id;
}



