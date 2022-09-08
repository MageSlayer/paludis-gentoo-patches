/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/accumulate_deps.hh>
#include <paludis/resolver/collect_depped_upon.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/package_id.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<const PackageIDSet>
paludis::resolver::accumulate_deps(
        const Environment * const env,
        const std::shared_ptr<const PackageIDSet> & start,
        const std::shared_ptr<const PackageIDSequence> & will_eventually_have,
        const bool recurse,
        const std::function<void ()> & step)
{
    const std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());
    const std::shared_ptr<PackageIDSet> done(std::make_shared<PackageIDSet>());
    std::copy(start->begin(), start->end(), result->inserter());

    while (result->size() > done->size())
    {
        const std::shared_ptr<PackageIDSet> more(std::make_shared<PackageIDSet>());
        std::set_difference(result->begin(), result->end(), done->begin(), done->end(), more->inserter(), PackageIDSetComparator());

        for (const auto & id : *more)
        {
            step();

            done->insert(id);

            const std::shared_ptr<const PackageIDSet> depped_upon(collect_depped_upon(
                        env, id, will_eventually_have, std::make_shared<PackageIDSequence>()));
            std::copy(depped_upon->begin(), depped_upon->end(), result->inserter());
        }

        if (! recurse)
            break;
    }

    return result;
}
