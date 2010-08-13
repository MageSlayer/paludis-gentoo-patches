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

#include <paludis/resolver/collect_purges.hh>
#include <paludis/resolver/collect_world.hh>
#include <paludis/resolver/accumulate_deps_and_provides.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/package_id.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<PackageIDSet>
paludis::resolver::collect_purges(
        const Environment * const env,
        const std::shared_ptr<const PackageIDSet> & have_now,
        const std::shared_ptr<const PackageIDSequence> & have_now_seq,
        const std::function<void ()> & step)
{
    const std::shared_ptr<const PackageIDSet> world(collect_world(env, have_now));
    const std::shared_ptr<const PackageIDSet> world_plus_deps(accumulate_deps_and_provides(env, world, have_now_seq, true, step));

    const std::shared_ptr<PackageIDSet> unused(std::make_shared<PackageIDSet>());
    std::set_difference(have_now->begin(), have_now->end(),
            world_plus_deps->begin(), world_plus_deps->end(), unused->inserter(), PackageIDSetComparator());

    return unused;
}

