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

#include <paludis/qa/dep_any_check.hh>

#include <paludis/portage_dep_parser.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/save.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/visitor-impl.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        ConstVisitor<DependencySpecTree>
    {
        CheckResult & result;
        const std::string role;
        bool in_any;

        Checker(CheckResult & rr, const std::string & r) :
            result(rr),
            role(r),
            in_any(false)
        {
        }

        void visit_leaf(const PackageDepSpec &)
        {
        }

        void visit_sequence(const AllDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            /* yes, the following line is correct. */
            Save<bool> in_any_save(&in_any, false);
            std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<bool> in_any_save(&in_any, true);
            if (cur == end)
                result << Message(qal_minor, "Empty || ( ) block in " + role);
            else
            {
                if (end == next(cur))
                    result << Message(qal_minor, "One item in || ( ) block in " + role);

                std::for_each(cur, end, accept_visitor(*this));
            }
        }

        void visit_sequence(const UseDepSpec & u,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            if (in_any)
                result << Message(qal_maybe, "Conditional on '" + stringify(u.flag()) +
                        "' inside || ( ) block in " + role);
            std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }
    };
}

DepAnyCheck::DepAnyCheck()
{
}

CheckResult
DepAnyCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->main_repository()->name());
        tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        if (metadata->deps_interface)
        {
            Checker depend_checker(result, "DEPEND");
            metadata->deps_interface->build_depend()->accept(depend_checker);

            Checker rdepend_checker(result, "RDEPEND");
            metadata->deps_interface->run_depend()->accept(rdepend_checker);

            Checker pdepend_checker(result, "PDEPEND");
            metadata->deps_interface->post_depend()->accept(pdepend_checker);
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
DepAnyCheck::identifier()
{
    static const std::string id("any_deps");
    return id;
}

