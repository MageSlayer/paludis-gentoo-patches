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
#include <paludis/spec_tree.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/normalise.hh>
#include <paludis/repositories/cran/package_dep_spec.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_shared_ptr.hh>

#include <string>
#include <list>

using namespace paludis;

std::shared_ptr<DependencySpecTree>
cranrepository::parse_depends(const std::string & s)
{
    Context context("When parsing CRAN 'Depends:' string: '" + s + "':");

    std::shared_ptr<DependencySpecTree> result(new DependencySpecTree(make_shared_ptr(new AllDepSpec)));

    std::list<std::string> specs;

    std::string::size_type p(0), s_p(0);
    while (p < s.length())
    {
        if (s[p] == ',')
        {
            specs.push_back(s.substr(s_p, p - s_p));
            s_p = ++p;
        }
        else if (s[p] == '(')
        {
            p = s.find(')', p);
            if (std::string::npos == p)
                p = s.length();
        }
        else
            ++p;
    }

    if (s_p < s.length())
        specs.push_back(s.substr(s_p));

    std::list<std::string>::const_iterator a(specs.begin()), a_end(specs.end());
    for ( ; a != a_end ; ++a)
    {
        Context local_context("When processing token '" + *a + "':");

        std::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(cranrepository::parse_cran_package_dep_spec(
                        strip_leading(strip_trailing(*a, " \r\n\t"), " \r\n\t"))));
        result->root()->append(spec);
    }

    return result;
}

