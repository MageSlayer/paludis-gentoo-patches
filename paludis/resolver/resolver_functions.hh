/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_FUNCTIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_FUNCTIONS_HH 1

#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/use_existing-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/required_confirmations-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>
#include <paludis/resolver/collect_depped_upon-fwd.hh>

#include <paludis/util/named_value.hh>
#include <paludis/util/tribool-fwd.hh>

#include <paludis/filter-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>

#include <functional>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_allow_choice_changes_fn> allow_choice_changes_fn;
        typedef Name<struct name_allowed_to_remove_fn> allowed_to_remove_fn;
        typedef Name<struct name_allowed_to_restart_fn> allowed_to_restart_fn;
        typedef Name<struct name_always_via_binary_fn> always_via_binary_fn;
        typedef Name<struct name_can_use_fn> can_use_fn;
        typedef Name<struct name_confirm_fn> confirm_fn;
        typedef Name<struct name_find_replacing_fn> find_replacing_fn;
        typedef Name<struct name_find_repository_for_fn> find_repository_for_fn;
        typedef Name<struct name_get_constraints_for_dependent_fn> get_constraints_for_dependent_fn;
        typedef Name<struct name_get_constraints_for_purge_fn> get_constraints_for_purge_fn;
        typedef Name<struct name_get_constraints_for_via_binary_fn> get_constraints_for_via_binary_fn;
        typedef Name<struct name_get_destination_types_for_blocker_fn> get_destination_types_for_blocker_fn;
        typedef Name<struct name_get_destination_types_for_error_fn> get_destination_types_for_error_fn;
        typedef Name<struct name_get_initial_constraints_for_fn> get_initial_constraints_for_fn;
        typedef Name<struct name_get_resolvents_for_fn> get_resolvents_for_fn;
        typedef Name<struct name_get_use_existing_nothing_fn> get_use_existing_nothing_fn;
        typedef Name<struct name_interest_in_spec_fn> interest_in_spec_fn;
        typedef Name<struct name_make_destination_filtered_generator_fn> make_destination_filtered_generator_fn;
        typedef Name<struct name_make_origin_filtered_generator_fn> make_origin_filtered_generator_fn;
        typedef Name<struct name_make_unmaskable_filter_fn> make_unmaskable_filter_fn;
        typedef Name<struct name_order_early_fn> order_early_fn;
        typedef Name<struct name_prefer_or_avoid_fn> prefer_or_avoid_fn;
        typedef Name<struct name_remove_hidden_fn> remove_hidden_fn;
        typedef Name<struct name_remove_if_dependent_fn> remove_if_dependent_fn;
    }

    namespace resolver
    {
        typedef std::function<bool (
                const std::shared_ptr<const Resolution> &
                )> AllowChoiceChangesFunction;

        typedef std::function<bool (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const PackageID> &
                )> AllowedToRemoveFunction;

        typedef std::function<bool (
                const std::shared_ptr<const Resolution> &
                )> AllowedToRestartFunction;

        typedef std::function<bool (
                const std::shared_ptr<const Resolution> &
                )> AlwaysViaBinaryFunction;

        typedef std::function<bool (
                const std::shared_ptr<const PackageID> &
                )> CanUseFunction;

        typedef std::function<bool (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const RequiredConfirmation> &
                )> ConfirmFunction;

        typedef std::function<std::shared_ptr<const PackageIDSequence> (
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const Repository> &
                )> FindReplacingFunction;

        typedef std::function<const std::shared_ptr<const Repository> (
                const std::shared_ptr<const Resolution> &,
                const ChangesToMakeDecision &
                )> FindRepositoryForFunction;

        typedef std::function<std::shared_ptr<ConstraintSequence> (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const DependentPackageIDSequence> &
                )> GetConstraintsForDependentFunction;

        typedef std::function<std::shared_ptr<ConstraintSequence> (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const ChangeByResolventSequence> &
                )> GetConstraintsForPurgeFunction;

        typedef std::function<std::shared_ptr<ConstraintSequence> (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const Resolution> &
                )> GetConstraintsForViaBinaryFunction;

        typedef std::function<DestinationTypes (
                const BlockDepSpec &,
                const std::shared_ptr<const Reason> &
                )> GetDestinationTypesForBlockerFunction;

        typedef std::function<DestinationTypes (
                const PackageDepSpec &,
                const std::shared_ptr<const Reason> &
                )> GetDestinationTypesForErrorFunction;

        typedef std::function<std::shared_ptr<Constraints> (
                const Resolvent &
                )> GetInitialConstraintsForFunction;

        typedef std::function<std::pair<std::shared_ptr<const Resolvents>, bool> (
                const PackageDepSpec &,
                const std::shared_ptr<const PackageID> &,
                const std::shared_ptr<const SlotName> &,
                const std::shared_ptr<const Reason> &
                )> GetResolventsForFunction;

        typedef std::function<std::pair<UseExisting, bool> (
                const std::shared_ptr<const Resolution> &,
                const PackageDepSpec &,
                const std::shared_ptr<const Reason> &
                )> GetUseExistingNothingFunction;

        typedef std::function<SpecInterest (
                const std::shared_ptr<const Resolution> &,
                const std::shared_ptr<const PackageID> &,
                const SanitisedDependency &
                )> InterestInSpecFunction;

        typedef std::function<FilteredGenerator (
                const Generator &,
                const std::shared_ptr<const Resolution> &
                )> MakeDestinationFilteredGeneratorFunction;

        typedef std::function<FilteredGenerator (
                const Generator &
                )> MakeOriginFilteredGeneratorFunction;

        typedef std::function<Filter (
                const QualifiedPackageName &
                )> MakeUnmaskableFilterFunction;

        typedef std::function<Tribool (
                const std::shared_ptr<const Resolution> &
                )> OrderEarlyFunction;

        typedef std::function<Tribool (
                const PackageDepSpec &,
                const std::shared_ptr<const PackageID> &
                )> PreferOrAvoidFunction;

        typedef std::function<std::shared_ptr<const PackageIDSequence> (
                const std::shared_ptr<const PackageIDSequence> &
                )> RemoveHiddenFunction;

        typedef std::function<bool (
                const std::shared_ptr<const PackageID> &
                )> RemoveIfDependentFunction;

        struct ResolverFunctions
        {
            NamedValue<n::allow_choice_changes_fn, AllowChoiceChangesFunction> allow_choice_changes_fn;
            NamedValue<n::allowed_to_remove_fn, AllowedToRemoveFunction> allowed_to_remove_fn;
            NamedValue<n::allowed_to_restart_fn, AllowedToRestartFunction> allowed_to_restart_fn;
            NamedValue<n::always_via_binary_fn, AlwaysViaBinaryFunction> always_via_binary_fn;
            NamedValue<n::can_use_fn, CanUseFunction> can_use_fn;
            NamedValue<n::confirm_fn, ConfirmFunction> confirm_fn;
            NamedValue<n::find_replacing_fn, FindReplacingFunction> find_replacing_fn;
            NamedValue<n::find_repository_for_fn, FindRepositoryForFunction> find_repository_for_fn;
            NamedValue<n::get_constraints_for_dependent_fn, GetConstraintsForDependentFunction> get_constraints_for_dependent_fn;
            NamedValue<n::get_constraints_for_purge_fn, GetConstraintsForPurgeFunction> get_constraints_for_purge_fn;
            NamedValue<n::get_constraints_for_via_binary_fn, GetConstraintsForViaBinaryFunction> get_constraints_for_via_binary_fn;
            NamedValue<n::get_destination_types_for_blocker_fn, GetDestinationTypesForBlockerFunction> get_destination_types_for_blocker_fn;
            NamedValue<n::get_destination_types_for_error_fn, GetDestinationTypesForErrorFunction> get_destination_types_for_error_fn;
            NamedValue<n::get_initial_constraints_for_fn, GetInitialConstraintsForFunction> get_initial_constraints_for_fn;
            NamedValue<n::get_resolvents_for_fn, GetResolventsForFunction> get_resolvents_for_fn;
            NamedValue<n::get_use_existing_nothing_fn, GetUseExistingNothingFunction> get_use_existing_nothing_fn;
            NamedValue<n::interest_in_spec_fn, InterestInSpecFunction> interest_in_spec_fn;
            NamedValue<n::make_destination_filtered_generator_fn,
                MakeDestinationFilteredGeneratorFunction> make_destination_filtered_generator_fn;
            NamedValue<n::make_origin_filtered_generator_fn,
                MakeOriginFilteredGeneratorFunction> make_origin_filtered_generator_fn;
            NamedValue<n::make_unmaskable_filter_fn,
                MakeUnmaskableFilterFunction> make_unmaskable_filter_fn;
            NamedValue<n::order_early_fn, OrderEarlyFunction> order_early_fn;
            NamedValue<n::prefer_or_avoid_fn, PreferOrAvoidFunction> prefer_or_avoid_fn;
            NamedValue<n::remove_hidden_fn, RemoveHiddenFunction> remove_hidden_fn;
            NamedValue<n::remove_if_dependent_fn, RemoveIfDependentFunction> remove_if_dependent_fn;
        };
    }
}

#endif
