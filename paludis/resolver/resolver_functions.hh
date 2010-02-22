/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/named_value.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/name-fwd.hh>
#include <tr1/functional>

namespace paludis
{
    namespace n
    {
        struct allowed_to_remove_fn;
        struct care_about_dep_fn;
        struct confirm_fn;
        struct find_repository_for_fn;
        struct get_destination_types_for_fn;
        struct get_initial_constraints_for_fn;
        struct get_resolvents_for_fn;
        struct get_use_existing_fn;
        struct make_destination_filtered_generator_fn;
        struct prefer_or_avoid_fn;
        struct take_dependency_fn;
    }

    namespace resolver
    {
        typedef std::tr1::function<bool (
                const std::tr1::shared_ptr<const PackageID> &
                )> AllowedToRemoveFunction;

        typedef std::tr1::function<bool (
                const Resolvent &,
                const std::tr1::shared_ptr<const Resolution> &,
                const SanitisedDependency &
                )> CareAboutDepFunction;

        typedef std::tr1::function<bool (
                const Resolvent &,
                const std::tr1::shared_ptr<const Resolution> &,
                const std::tr1::shared_ptr<const RequiredConfirmation> &
                )> ConfirmFunction;

        typedef std::tr1::function<const std::tr1::shared_ptr<const Repository> (
                const Resolvent &,
                const std::tr1::shared_ptr<const Resolution> &,
                const ChangesToMakeDecision &
                )> FindRepositoryForFunction;

        typedef std::tr1::function<DestinationTypes (
                const PackageDepSpec &,
                const std::tr1::shared_ptr<const PackageID> &,
                const std::tr1::shared_ptr<const Reason> &
                )> GetDestinationTypesForFunction;

        typedef std::tr1::function<std::tr1::shared_ptr<Constraints> (
                const Resolvent &
                )> GetInitialConstraintsFunction;

        typedef std::tr1::function<std::tr1::shared_ptr<Resolvents> (
                const PackageDepSpec &,
                const std::tr1::shared_ptr<const SlotName> &,
                const std::tr1::shared_ptr<const Reason> &
                )> GetResolventsForFunction;

        typedef std::tr1::function<UseExisting (
                const Resolvent &,
                const PackageDepSpec &,
                const std::tr1::shared_ptr<const Reason> &
                )> GetUseExistingFunction;

        typedef std::tr1::function<FilteredGenerator (
                const Generator &,
                const Resolvent &
                )> MakeDestinationFilteredGeneratorFunction;

        typedef std::tr1::function<Tribool (
                const QualifiedPackageName &
                )> PreferOrAvoidFunction;

        typedef std::tr1::function<bool (
                const Resolvent &,
                const SanitisedDependency &,
                const std::tr1::shared_ptr<const Reason> &
                )> TakeDependencyFunction;

        struct ResolverFunctions
        {
            NamedValue<n::allowed_to_remove_fn, AllowedToRemoveFunction> allowed_to_remove_fn;
            NamedValue<n::care_about_dep_fn, CareAboutDepFunction> care_about_dep_fn;
            NamedValue<n::confirm_fn, ConfirmFunction> confirm_fn;
            NamedValue<n::find_repository_for_fn, FindRepositoryForFunction> find_repository_for_fn;
            NamedValue<n::get_destination_types_for_fn, GetDestinationTypesForFunction> get_destination_types_for_fn;
            NamedValue<n::get_initial_constraints_for_fn, GetInitialConstraintsFunction> get_initial_constraints_for_fn;
            NamedValue<n::get_resolvents_for_fn, GetResolventsForFunction> get_resolvents_for_fn;
            NamedValue<n::get_use_existing_fn, GetUseExistingFunction> get_use_existing_fn;
            NamedValue<n::make_destination_filtered_generator_fn,
                MakeDestinationFilteredGeneratorFunction> make_destination_filtered_generator_fn;
            NamedValue<n::prefer_or_avoid_fn, PreferOrAvoidFunction> prefer_or_avoid_fn;
            NamedValue<n::take_dependency_fn, TakeDependencyFunction> take_dependency_fn;
        };
    }
}

#endif
