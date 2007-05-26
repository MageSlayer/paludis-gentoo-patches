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
#include <paludis/portage_dep_parser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/pdepend_overlap_check.hh>
#include <paludis/util/join.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/visitor-impl.hh>
#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Collector :
        ConstVisitor<DependencySpecTree>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, AllDepSpec>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, AnyDepSpec>,
        ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, UseDepSpec>
    {
        using ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, UseDepSpec>::visit_sequence;
        using ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, AllDepSpec>::visit_sequence;
        using ConstVisitor<DependencySpecTree>::VisitConstSequence<Collector, AnyDepSpec>::visit_sequence;

        std::set<QualifiedPackageName> result;

        Collector()
        {
        }

        void visit_leaf(const PackageDepSpec & p)
        {
            if (p.package_ptr())
                result.insert(*p.package_ptr());
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }
    };
}

PdependOverlapCheck::PdependOverlapCheck()
{
}

CheckResult
PdependOverlapCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->main_repository()->name());
        tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        Collector pdepend_collector;
        metadata->deps_interface->post_depend()->accept(pdepend_collector);

        {
            Collector depend_collector;
            metadata->deps_interface->build_depend()->accept(depend_collector);

            std::set<QualifiedPackageName> overlap;
            std::set_intersection(depend_collector.result.begin(), depend_collector.result.end(),
                    pdepend_collector.result.begin(), pdepend_collector.result.end(),
                    std::inserter(overlap, overlap.begin()));

            if (! overlap.empty())
                result << Message(qal_major, "Overlap between DEPEND and PDEPEND: '" +
                        join(overlap.begin(), overlap.end(), "', ") + "'");
        }

        {
            Collector rdepend_collector;
            metadata->deps_interface->run_depend()->accept(rdepend_collector);

            std::set<QualifiedPackageName> overlap;
            std::set_intersection(rdepend_collector.result.begin(), rdepend_collector.result.end(),
                    pdepend_collector.result.begin(), pdepend_collector.result.end(),
                    std::inserter(overlap, overlap.begin()));

            if (! overlap.empty())
                result << Message(qal_major, "Overlap between RDEPEND and PDEPEND: '" +
                        join(overlap.begin(), overlap.end(), "', ") + "'");
        }

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
PdependOverlapCheck::identifier()
{
    static const std::string id("pdepend_overlap");
    return id;
}


