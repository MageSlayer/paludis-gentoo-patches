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

#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_collection.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<AllowedToRemoveHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection allowed_to_remove_specs;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

AllowedToRemoveHelper::AllowedToRemoveHelper(const Environment * const e) :
    Pimp<AllowedToRemoveHelper>(e)
{
}

AllowedToRemoveHelper::~AllowedToRemoveHelper() = default;

void
AllowedToRemoveHelper::add_allowed_to_remove_spec(const PackageDepSpec & spec)
{
    _imp->allowed_to_remove_specs.insert(spec);
}

namespace
{
    struct AllowedToRemoveVisitor
    {
        bool visit(const DependentReason &) const
        {
            return true;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return true;
        }

        bool visit(const ViaBinaryReason &) const
        {
            return false;
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }

        bool visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<bool>(*this);
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }
    };
}

bool
AllowedToRemoveHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const std::shared_ptr<const PackageID> & id) const
{
    for (auto c(resolution->constraints()->begin()), c_end(resolution->constraints()->end()) ;
            c != c_end ; ++c)
        if ((*c)->reason()->accept_returning<bool>(AllowedToRemoveVisitor()))
            return true;

    return _imp->allowed_to_remove_specs.match_any(_imp->env, id, { });
}

template class Pimp<AllowedToRemoveHelper>;

