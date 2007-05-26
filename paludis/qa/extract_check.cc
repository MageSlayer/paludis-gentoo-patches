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

#include <paludis/dep_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/qa/extract_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/qa/qa_environment.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        ConstVisitor<GenericSpecTree>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, AllDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, AnyDepSpec>,
        ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, UseDepSpec>
    {
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, UseDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, AllDepSpec>::visit_sequence;
        using ConstVisitor<GenericSpecTree>::VisitConstSequence<Checker, AnyDepSpec>::visit_sequence;

        bool need_zip;
        bool have_zip;

        Checker() :
            need_zip(false),
            have_zip(false)
        {
        }

        void visit_leaf(const PlainTextDepSpec & a)
        {
            if (a.text().length() >= 4)
                if (a.text().substr(a.text().length() - 4) == ".zip")
                    need_zip = true;
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }

        void visit_leaf(const PackageDepSpec & p)
        {
            if (p.package_ptr() && *p.package_ptr() == QualifiedPackageName("app-arch/unzip"))
                have_zip = true;
        }
    };
}

ExtractCheck::ExtractCheck()
{
}

CheckResult
ExtractCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        do
        {
            PackageDatabaseEntry ee(e.name, e.version,
                    e.environment->main_repository()->name());
            tr1::shared_ptr<const VersionMetadata> metadata(
                    e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

            Checker checker;
            metadata->ebuild_interface->src_uri()->accept(checker);
            metadata->deps_interface->build_depend()->accept(checker);

            if (checker.need_zip && ! checker.have_zip)
                result << Message(qal_major, "Found .zip in SRC_URI but app-arch/unzip is not in DEPEND");

        } while (false);
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
ExtractCheck::identifier()
{
    static const std::string id("extract");
    return id;
}

