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

#include <paludis/resolver/get_use_existing_nothing_helper.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/labels_classifier.hh>
#include <paludis/resolver/match_qpns.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<GetUseExistingNothingHelper>
    {
        const Environment * const env;
        std::list<PackageDepSpec> with_specs;
        std::list<PackageDepSpec> without_specs;
        UseExisting use_existing_for_dependencies;
        UseExisting use_existing_for_targets;
        UseExisting use_existing_for_set_targets;

        Imp(const Environment * const e) :
            env(e),
            use_existing_for_dependencies(ue_if_possible),
            use_existing_for_targets(ue_never),
            use_existing_for_set_targets(ue_if_same)
        {
        }
    };
}

GetUseExistingNothingHelper::GetUseExistingNothingHelper(const Environment * const e) :
    _imp(e)
{
}

GetUseExistingNothingHelper::~GetUseExistingNothingHelper() = default;

void
GetUseExistingNothingHelper::add_with_spec(const PackageDepSpec & spec)
{
    _imp->with_specs.push_back(spec);
}

void
GetUseExistingNothingHelper::add_without_spec(const PackageDepSpec & spec)
{
    _imp->without_specs.push_back(spec);
}

void
GetUseExistingNothingHelper::set_use_existing_for_dependencies(const UseExisting v)
{
    _imp->use_existing_for_dependencies = v;
}

void
GetUseExistingNothingHelper::set_use_existing_for_targets(const UseExisting v)
{
    _imp->use_existing_for_targets = v;
}

void
GetUseExistingNothingHelper::set_use_existing_for_set_targets(const UseExisting v)
{
    _imp->use_existing_for_set_targets = v;
}

namespace
{
    bool use_existing_from_withish(
            const Environment * const env,
            const QualifiedPackageName & name,
            const std::list<PackageDepSpec> & specs)
    {
        for (const auto & l : specs)
            if (match_qpns(*env, l, name))
                return true;
        return false;
    }

    struct UseExistingVisitor
    {
        const Environment * const env;
        const bool from_set;
        const Resolvent resolvent;
        const UseExisting use_existing_for_dependencies;
        const UseExisting use_existing_for_targets;
        const UseExisting use_existing_for_set_targets;

        bool creating_and_no_appropriate_ids() const
        {
            bool (* can)(const std::shared_ptr<const PackageID> &)(nullptr);
            switch (resolvent.destination_type())
            {
                case dt_install_to_slash:
                    return false;

                case dt_create_binary:
                    can = &can_make_binary_for;
                    break;

                case dt_install_to_chroot:
                    can = &can_chroot;
                    break;

                case last_dt:
                    break;
            }

            if (! can)
                throw InternalError(PALUDIS_HERE, "unhandled dt");

            auto origin_ids((*env)[selection::AllVersionsSorted(
                        generator::Package(resolvent.package()) |
                        make_slot_filter(resolvent) |
                        filter::SupportsAction<InstallAction>() |
                        filter::NotMasked()
                        )]);
            if (origin_ids->empty())
                return false;
            else
            {
                for (const auto & i : *origin_ids)
                    if ((*can)(i))
                        return false;

                return true;
            }
        }

        std::pair<UseExisting, bool> visit(const DependencyReason &) const
        {
            return std::make_pair(use_existing_for_dependencies, creating_and_no_appropriate_ids());
        }

        std::pair<UseExisting, bool> visit(const TargetReason &) const
        {
            return std::make_pair(from_set ? use_existing_for_set_targets : use_existing_for_targets,
                    creating_and_no_appropriate_ids());
        }

        std::pair<UseExisting, bool> visit(const DependentReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const WasUsedByReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const PresetReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const ViaBinaryReason &) const
        {
            return std::make_pair(ue_if_possible, false);
        }

        std::pair<UseExisting, bool> visit(const SetReason & r) const
        {
            UseExistingVisitor v{env, true, resolvent, use_existing_for_dependencies, use_existing_for_targets, use_existing_for_set_targets};
            return r.reason_for_set()->accept_returning<std::pair<UseExisting, bool> >(v);
        }

        std::pair<UseExisting, bool> visit(const LikeOtherDestinationTypeReason & r) const
        {
            UseExistingVisitor v{env, from_set, resolvent, use_existing_for_dependencies, use_existing_for_targets, use_existing_for_set_targets};
            return r.reason_for_other()->accept_returning<std::pair<UseExisting, bool> >(v);
        }
    };
}

std::pair<UseExisting, bool>
GetUseExistingNothingHelper::operator() (
        const std::shared_ptr<const Resolution> & resolution,
        const PackageDepSpec & spec,
        const std::shared_ptr<const Reason> & reason) const
{
    Context context("When determining use existing for '" + stringify(spec) + "':");

    if (spec.package_ptr())
    {
        if (use_existing_from_withish(_imp->env, *spec.package_ptr(), _imp->without_specs))
            return std::make_pair(ue_if_possible, true);
        if (use_existing_from_withish(_imp->env, *spec.package_ptr(), _imp->with_specs))
            return std::make_pair(ue_never, false);
    }

    UseExistingVisitor v{_imp->env, false, resolution->resolvent(), _imp->use_existing_for_dependencies, _imp->use_existing_for_targets,
        _imp->use_existing_for_set_targets};
    return reason->accept_returning<std::pair<UseExisting, bool> >(v);
}

namespace paludis
{
    template class Pimp<GetUseExistingNothingHelper>;
}
