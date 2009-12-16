/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/common_sets.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/spec_tree.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>

using namespace paludis;

namespace
{
    std::tr1::shared_ptr<SetSpecTree> get_installed_set(
            const Environment * const env,
            const Repository * const repo,
            const bool slots)
    {
        Context context("When making " + std::string(slots ? "installed-slots" : "installed-packages") +
                " set from '" + stringify(repo->name()) + "':");

        std::tr1::shared_ptr<SetSpecTree> result(new SetSpecTree(make_shared_ptr(new AllDepSpec)));

        std::tr1::shared_ptr<const PackageIDSequence> ids;
        if (slots)
            ids = ((*env)[selection::BestVersionInEachSlot(generator::InRepository(repo->name()))]);
        else
            ids = ((*env)[selection::BestVersionOnly(generator::InRepository(repo->name()))]);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            if (slots && (*i)->slot_key())
                result->root()->append(make_shared_ptr(new PackageDepSpec(
                                make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                                .package((*i)->name())
                                .slot_requirement(make_shared_ptr(new ELikeSlotExactRequirement(
                                            (*i)->slot_key()->value(), false)))
                                )));
            else
                result->root()->append(make_shared_ptr(new PackageDepSpec(
                                make_package_dep_spec(PartiallyMadePackageDepSpecOptions())
                                .package((*i)->name())
                                )));

        return result;
    }
}

void
paludis::add_common_sets_for_installed_repo(
        const Environment * const env,
        const Repository & repo)
{
    env->add_set(SetName("installed-packages"), SetName("installed-packages::" + stringify(repo.name())),
            std::tr1::bind(&get_installed_set, env, &repo, false), true);
    env->add_set(SetName("installed-slots"), SetName("installed-slots::" + stringify(repo.name())),
            std::tr1::bind(&get_installed_set, env, &repo, true), true);
}

