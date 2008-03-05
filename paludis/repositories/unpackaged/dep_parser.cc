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

#include <paludis/repositories/unpackaged/dep_parser.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <list>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

tr1::shared_ptr<const DependencySpecTree::ConstItem>
DepParser::parse(const std::string & s)
{
    Context context("When parsing '" + s + "':");

    tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > result(
            new ConstTreeSequence<DependencySpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));

    std::list<std::string> tokens;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, ",", "", std::back_inserter(tokens));

    for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
            t != t_end ; ++t)
    {
        std::string a(strip_leading(strip_trailing(*t, " \t\r\n"), " \t\r\n"));
        Context local_context("When parsing token '" + a + "':");

        if (a.empty())
            continue;

        tr1::shared_ptr<TreeLeaf<DependencySpecTree, PackageDepSpec> > spec(
                new TreeLeaf<DependencySpecTree, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                        new PackageDepSpec(parse_user_package_dep_spec(a, UserPackageDepSpecOptions())))));
        result->add(spec);
    }

    return result;
}

