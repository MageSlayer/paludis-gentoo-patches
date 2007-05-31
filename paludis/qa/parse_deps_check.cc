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

#include <paludis/portage_dep_parser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/eapi.hh>
#include <paludis/environment.hh>
#include <paludis/qa/parse_deps_check.hh>
#include <paludis/qa/qa_environment.hh>

using namespace paludis;
using namespace paludis::qa;

ParseDepsCheck::ParseDepsCheck()
{
}

CheckResult
ParseDepsCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->main_repository()->name());
        tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        try
        {
            std::string depend(metadata->deps_interface->get_raw_build_depend());
            PortageDepParser::parse_depend(depend, *metadata->eapi);
        }
        catch (const Exception & err)
        {
            result << Message(qal_fatal, "Unparsable DEPEND: '" +
                    err.message() + "' (" + err.what() + ")");
        }

        try
        {
            std::string rdepend(metadata->deps_interface->get_raw_run_depend());
            PortageDepParser::parse_depend(rdepend, *metadata->eapi);
        }
        catch (const Exception & err)
        {
            result << Message(qal_fatal, "Unparsable RDEPEND: '" +
                    err.message() + "' (" + err.what() + ")");
        }

        try
        {
            std::string pdepend(metadata->deps_interface->get_raw_post_depend());
            PortageDepParser::parse_depend(pdepend, *metadata->eapi);
        }
        catch (const Exception & err)
        {
            result << Message(qal_fatal, "Unparsable PDEPEND: '" +
                    err.message() + "' (" + err.what() + ")");
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
ParseDepsCheck::identifier()
{
    static const std::string id("parse_deps");
    return id;
}

