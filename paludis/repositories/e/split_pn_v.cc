/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/split_pn_v.hh>

#include <paludis/util/accept_visitor.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/package_dep_spec_requirement.hh>

#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct BitsFinder
    {
        std::pair<PackageNamePart, VersionSpec> result;
        bool had_package, had_version;

        BitsFinder() :
            result(PackageNamePart("x"), VersionSpec("0", { })),
            had_package(false),
            had_version(false)
        {
        }

        void visit(const PackageNamePartRequirement & r)
        {
            if (had_package)
                throw InternalError(PALUDIS_HERE, "got multiple /p parts");
            had_package = true;
            result.first = r.name_part();
        }

        void visit(const VersionRequirement & r)
        {
            if (had_version)
                throw InternalError(PALUDIS_HERE, "got multiple v parts");
            had_version = true;
            result.second = r.version_spec();
        }

        void visit(const CategoryNamePartRequirement &)
        {
        }

        void visit(const PackageDepSpecRequirement &)
        {
            throw InternalError(PALUDIS_HERE, "got unexpected requirement");
        }
    };
}

std::pair<PackageNamePart, VersionSpec>
paludis::erepository::split_pn_v(const Environment * const env, const std::string & s)
{
    PackageDepSpec spec(parse_user_package_dep_spec("=*/" + s, env, { updso_allow_wildcards }));
    BitsFinder f;
    std::for_each(indirect_iterator(spec.requirements()->begin()),
            indirect_iterator(spec.requirements()->end()),
            accept_visitor(f));

    if (! (f.had_package && f.had_version))
        throw InternalError(PALUDIS_HERE, "didn't get p/v");

    return f.result;
}

