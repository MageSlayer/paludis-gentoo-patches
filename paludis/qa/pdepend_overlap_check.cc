/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/dep_atom.hh>
#include <paludis/dep_parser.hh>
#include <paludis/qa/pdepend_overlap_check.hh>
#include <paludis/util/join.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Collector :
        DepAtomVisitorTypes::ConstVisitor
    {
        std::set<QualifiedPackageName> result;

        Collector()
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            result.insert(p->package());
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PlainTextDepAtom * const)
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
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        Collector pdepend_collector;
        std::string pdepend(metadata->get(vmk_pdepend));
        DepParser::parse(pdepend)->accept(&pdepend_collector);

        {
            Collector depend_collector;
            std::string depend(metadata->get(vmk_depend));
            DepParser::parse(depend)->accept(&depend_collector);

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
            std::string rdepend(metadata->get(vmk_rdepend));
            DepParser::parse(rdepend)->accept(&rdepend_collector);

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
    catch (const Exception & e)
    {
        result << Message(qal_fatal, "Caught Exception '" + e.message() + "' ("
                + e.what() + ")");
    }

    return result;
}

const std::string &
PdependOverlapCheck::identifier()
{
    static const std::string id("pdepend overlap");
    return id;
}


