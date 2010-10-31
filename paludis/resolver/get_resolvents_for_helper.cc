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

#include <paludis/resolver/get_resolvents_for_helper.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/reason_utils.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/resolver/labels_classifier.hh>
#include <paludis/resolver/destination_utils.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/enum_iterator.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<GetResolventsForHelper>
    {
        const Environment * const env;

        DestinationType target_destination_type;

        bool want_target_dependencies;
        bool want_target_runtime_dependencies;

        bool want_dependencies_on_slash;
        bool want_runtime_dependencies_on_slash;

        bool want_best_slot_for_targets;
        bool want_installed_slots_for_targets;
        bool fallback_to_other_slots_for_targets;

        bool want_best_slot_otherwise;
        bool want_installed_slots_otherwise;
        bool fallback_to_other_slots_otherwise;

        Imp(const Environment * const e) :
            env(e),
            target_destination_type(dt_install_to_slash),
            want_target_dependencies(true),
            want_target_runtime_dependencies(true),
            want_dependencies_on_slash(true),
            want_runtime_dependencies_on_slash(true),
            want_best_slot_for_targets(true),
            want_installed_slots_for_targets(true),
            fallback_to_other_slots_for_targets(false),
            want_best_slot_otherwise(true),
            want_installed_slots_otherwise(true),
            fallback_to_other_slots_otherwise(false)
        {
        }
    };
}

GetResolventsForHelper::GetResolventsForHelper(const Environment * const e) :
    Pimp<GetResolventsForHelper>(e)
{
}

GetResolventsForHelper::~GetResolventsForHelper() = default;

void
GetResolventsForHelper::set_target_destination_type(const DestinationType v)
{
    _imp->target_destination_type = v;
}

void
GetResolventsForHelper::set_want_target_dependencies(const bool b)
{
    _imp->want_target_dependencies = b;
}

void
GetResolventsForHelper::set_want_target_runtime_dependencies(const bool b)
{
    _imp->want_target_runtime_dependencies = b;
}

void
GetResolventsForHelper::set_want_dependencies_on_slash(const bool b)
{
    _imp->want_dependencies_on_slash = b;
}

void
GetResolventsForHelper::set_want_runtime_dependencies_on_slash(const bool b)
{
    _imp->want_runtime_dependencies_on_slash = b;
}

void
GetResolventsForHelper::set_slots(const bool best, const bool installed, const bool fallback)
{
    _imp->want_best_slot_otherwise = best;
    _imp->want_installed_slots_otherwise = installed;
    _imp->fallback_to_other_slots_otherwise = fallback;
}

void
GetResolventsForHelper::set_target_slots(const bool best, const bool installed, const bool fallback)
{
    _imp->want_best_slot_for_targets = best;
    _imp->want_installed_slots_for_targets = installed;
    _imp->fallback_to_other_slots_for_targets = fallback;
}

namespace
{
    struct DestinationTypesFinder
    {
        const DestinationType target_destination_type;
        const bool want_target_dependencies;
        const bool want_target_runtime_dependencies;
        const bool want_dependencies_on_slash;
        const bool want_runtime_dependencies_on_slash;
        const std::shared_ptr<const PackageID> package_id;

        DestinationTypes visit(const TargetReason &) const
        {
            return { target_destination_type };
        }

        DestinationTypes visit(const DependentReason &) const
        {
            return { dt_install_to_slash };
        }

        DestinationTypes visit(const ViaBinaryReason &) const
        {
            return { };
        }

        DestinationTypes visit(const WasUsedByReason &) const
        {
            return { dt_install_to_slash };
        }

        DestinationTypes visit(const DependencyReason & dep) const
        {
            DestinationTypes extras, slash;

            switch (target_destination_type)
            {
                case dt_create_binary:
                    {
                        bool binary_if_possible(false);
                        if (want_target_dependencies)
                            binary_if_possible = true;
                        else if (want_target_runtime_dependencies)
                        {
                            /* this will track run deps of build deps, which isn't
                             * really right... */
                            if (is_run_or_post_dep(dep.sanitised_dependency()))
                                binary_if_possible = true;
                        }

                        if (binary_if_possible && can_make_binary_for(package_id))
                            extras += dt_create_binary;
                    }
                    break;

                case dt_install_to_chroot:
                    {
                        bool chroot_if_possible(false);
                        if (want_target_dependencies)
                            chroot_if_possible = true;
                        else if (want_target_runtime_dependencies)
                        {
                            if (is_run_or_post_dep(dep.sanitised_dependency()))
                                chroot_if_possible = true;
                        }

                        if (chroot_if_possible && can_chroot(package_id))
                            extras += dt_install_to_chroot;
                    }
                    break;

                case dt_install_to_slash:
                    break;

                case last_dt:
                    throw InternalError(PALUDIS_HERE, "unhandled dt");
            }

            if (want_runtime_dependencies_on_slash ^ want_dependencies_on_slash)
            {
                if (want_dependencies_on_slash && ! is_run_or_post_dep(dep.sanitised_dependency()))
                    slash += dt_install_to_slash;
                if (want_runtime_dependencies_on_slash && is_run_or_post_dep(dep.sanitised_dependency()))
                    slash += dt_install_to_slash;
            }
            else if (want_runtime_dependencies_on_slash || want_dependencies_on_slash)
                slash += dt_install_to_slash;

            return extras | slash;
        }

        DestinationTypes visit(const PresetReason &) const
        {
            return { };
        }

        DestinationTypes visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<DestinationTypes>(*this);
        }

        DestinationTypes visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<DestinationTypes>(*this);
        }
    };

    DestinationTypes get_destination_types_for_fn(
            const Environment * const,
            const PackageDepSpec &,
            const DestinationType target_destination_type,
            const bool want_target_dependencies,
            const bool want_target_runtime_dependencies,
            const bool want_dependencies_on_slash,
            const bool want_runtime_dependencies_on_slash,
            const std::shared_ptr<const PackageID> & package_id,
            const std::shared_ptr<const Reason> & reason)
    {
        DestinationTypesFinder f{target_destination_type, want_target_dependencies, want_target_runtime_dependencies,
            want_dependencies_on_slash, want_runtime_dependencies_on_slash, package_id};
        return reason->accept_returning<DestinationTypes>(f);
    }
}

std::pair<std::shared_ptr<Resolvents>, bool>
GetResolventsForHelper::operator() (
        const PackageDepSpec & spec,
        const std::shared_ptr<const SlotName> & maybe_slot,
        const std::shared_ptr<const Reason> & reason) const
{
    auto result_ids(std::make_shared<PackageIDSequence>());
    std::shared_ptr<const PackageID> best;

    auto ids((*_imp->env)[selection::BestVersionOnly(
                generator::Matches(spec, { mpo_ignore_additional_requirements }) |
                filter::SupportsAction<InstallAction>() |
                filter::NotMasked() |
                (maybe_slot ? Filter(filter::Slot(*maybe_slot)) : Filter(filter::All())))]);

    if (! ids->empty())
        best = *ids->begin();

    auto installed_ids((*_imp->env)[selection::BestVersionInEachSlot(
                generator::Matches(spec, { }) |
                (_imp->target_destination_type == dt_install_to_chroot ?
                 Filter(filter::InstalledNotAtRoot(_imp->env->system_root_key()->value())) :
                 Filter(filter::InstalledAtRoot(_imp->env->system_root_key()->value()))))]);

    auto target(is_target(reason));
    auto want_installed(target ? _imp->want_installed_slots_for_targets : _imp->want_installed_slots_otherwise);
    auto want_best(target ? _imp->want_best_slot_for_targets : _imp->want_best_slot_otherwise);
    auto fallback(target ? _imp->fallback_to_other_slots_for_targets : _imp->fallback_to_other_slots_otherwise);

    if (! best)
        std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
    else if (want_best && fallback && ! want_installed)
    {
        if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                    indirect_iterator(installed_ids->end()), *best))
            result_ids->push_back(best);
        else
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
    }
    else if (want_installed && fallback && ! want_best)
    {
        if (installed_ids->empty())
            result_ids->push_back(best);
        else
            std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
    }
    else if (want_installed && want_best)
    {
        if (indirect_iterator(installed_ids->end()) == std::find(indirect_iterator(installed_ids->begin()),
                    indirect_iterator(installed_ids->end()), *best))
            result_ids->push_back(best);
        std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());
    }
    else if (want_best)
        result_ids->push_back(best);
    else if (want_installed)
        std::copy(installed_ids->begin(), installed_ids->end(), result_ids->back_inserter());

    std::pair<std::shared_ptr<Resolvents>, bool> result(std::make_shared<Resolvents>(), false);

    if (! result_ids->empty())
    {
        result.second = true;

        for (PackageIDSequence::ConstIterator i(result_ids->begin()), i_end(result_ids->end()) ;
                i != i_end ; ++i)
        {
            DestinationTypes destination_types(get_destination_types_for_fn(_imp->env, spec,
                        _imp->target_destination_type, _imp->want_target_dependencies, _imp->want_target_runtime_dependencies,
                        _imp->want_dependencies_on_slash, _imp->want_runtime_dependencies_on_slash, *i, reason));

            if (destination_types.any())
            {
                result.second = false;
                for (EnumIterator<DestinationType> t, t_end(last_dt) ; t != t_end ; ++t)
                    if (destination_types[*t])
                        result.first->push_back(Resolvent(*i, *t));
            }
        }
    }

    return result;
}

template class Pimp<GetResolventsForHelper>;

