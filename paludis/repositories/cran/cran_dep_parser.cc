/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk
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
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/normalise.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <string>
#include <list>

using namespace paludis;

tr1::shared_ptr<DependencySpecTree::ConstItem>
cranrepository::parse_depends(const std::string & s)
{
    Context context("When parsing CRAN 'Depends:' string: '" + s + "':");

    tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > result(
            new ConstTreeSequence<DependencySpecTree, AllDepSpec>(tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));

    std::list<std::string> specs;
    Tokeniser<delim_kind::AnyOfTag>::tokenise(s, ",", std::back_inserter(specs));

    std::list<std::string>::const_iterator a(specs.begin()), a_end(specs.end());
    for ( ; a != a_end ; ++a)
    {
        Context local_context("When processing token '" + *a + "':");

        std::string aa = strip_leading(strip_trailing(*a, ")"), " \t");

        std::string name, tmp, version, range;
        std::string::size_type p(aa.find('('));
        if ((std::string::npos != p))
        {
            name = strip_leading(strip_trailing(aa.substr(0, p), " \t"), " \t");
            tmp = aa.substr(p + 1);
            p = tmp.find(')');
            aa = tmp.substr(0, p);
            version = strip_trailing(strip_leading(aa, " \t(<>=~"), " )\t\n");
            range = strip_trailing(strip_leading(aa.substr(0, aa.find(version)), " \t"), " \t\n");
        }
        else
            name = strip_leading(strip_trailing(aa, " \t"), " \t");

        name = cran_name_to_internal(name);
        version = cran_version_to_internal(version);

        if ("R" == name)
            name = "dev-lang/R";
        else
            name = "cran/" + name;

        std::string spec_string;
        if (version.empty() || range.empty())
            spec_string = name;
        else
            spec_string = range + name + "-" + version;
        tr1::shared_ptr<TreeLeaf<DependencySpecTree, PackageDepSpec> > spec(
                new TreeLeaf<DependencySpecTree, PackageDepSpec>(tr1::shared_ptr<PackageDepSpec>(
                        new PackageDepSpec(spec_string, pds_pm_permissive))));
        result->add(spec);
    }

    return result;
}

