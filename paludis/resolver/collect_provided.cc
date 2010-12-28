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

#include <paludis/resolver/collect_provided.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/spec_tree.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<const PackageIDSet>
paludis::resolver::collect_provided(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id)
{
    const std::shared_ptr<PackageIDSet> result(std::make_shared<PackageIDSet>());

    if (id->provide_key())
    {
        DepSpecFlattener<ProvideSpecTree, PackageDepSpec> f(env, id);
        id->provide_key()->value()->top()->accept(f);

        for (DepSpecFlattener<ProvideSpecTree, PackageDepSpec>::ConstIterator v(f.begin()), v_end(f.end()) ;
                v != v_end ; ++v)
        {
            const std::shared_ptr<const PackageIDSequence> virtuals((*env)[selection::AllVersionsUnsorted(
                        generator::Matches(**v, { }))]);
            std::copy(virtuals->begin(), virtuals->end(), result->inserter());
        }
    }

    return result;
}


