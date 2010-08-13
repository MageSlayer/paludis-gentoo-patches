/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/collect_world.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<const PackageIDSet>
paludis::resolver::collect_world(
        const Environment * const env,
        const std::shared_ptr<const PackageIDSet> & from
        )
{
    const std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    const std::shared_ptr<const SetSpecTree> set(env->set(SetName("world")));

    for (auto i(from->begin()), i_end(from->end()) ;
            i != i_end ; ++i)
        if (match_package_in_set(*env, *set, **i, { }))
            result->insert(*i);

    return result;
}
