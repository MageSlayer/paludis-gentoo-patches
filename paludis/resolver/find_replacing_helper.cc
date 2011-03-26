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

#include <paludis/resolver/find_replacing_helper.hh>
#include <paludis/resolver/same_slot.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>
#include <paludis/package_id.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/elike_slot_requirement.hh>
#include <paludis/metadata_key.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<FindReplacingHelper>
    {
        const Environment * const env;
        bool one_binary_per_slot;

        Imp(const Environment * const e) :
            env(e),
            one_binary_per_slot(false)
        {
        }
    };
}

FindReplacingHelper::FindReplacingHelper(const Environment * const e) :
    _imp(e)
{
}

FindReplacingHelper::~FindReplacingHelper() = default;

void
FindReplacingHelper::set_one_binary_per_slot(bool value)
{
    _imp->one_binary_per_slot = value;
}

const std::shared_ptr<const PackageIDSequence>
FindReplacingHelper::operator() (
        const std::shared_ptr<const PackageID> & id,
        const std::shared_ptr<const Repository> & repo) const
{
    Context context("When working out what is replaced by '" + stringify(*id) +
            "' when it is installed to '" + stringify(repo->name()) + "':");

    std::set<RepositoryName> repos;

    if (repo->installed_root_key())
    {
        for (auto r(_imp->env->begin_repositories()), r_end(_imp->env->end_repositories()) ;
                r != r_end ; ++r)
            if ((*r)->installed_root_key() &&
                    (*r)->installed_root_key()->value() == repo->installed_root_key()->value())
                repos.insert((*r)->name());
    }
    else
        repos.insert(repo->name());

    std::shared_ptr<PackageIDSequence> result(std::make_shared<PackageIDSequence>());
    for (std::set<RepositoryName>::const_iterator r(repos.begin()),
            r_end(repos.end()) ;
            r != r_end ; ++r)
    {
        std::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsUnsorted(
                    generator::Package(id->name()) & generator::InRepository(*r))]);
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->version() == id->version() || (same_slot(*i, id) && (_imp->one_binary_per_slot || repo->installed_root_key())))
                result->push_back(*i);
        }
    }

    return result;
}

template class Pimp<FindReplacingHelper>;

