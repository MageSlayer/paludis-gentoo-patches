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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_TEST_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVER_TEST_HH 1

#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/use_existing-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/required_confirmations-fwd.hh>
#include <paludis/resolver/package_or_block_dep_spec-fwd.hh>
#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/change_by_resolvent-fwd.hh>

#include <paludis/resolver/allow_choice_changes_helper.hh>
#include <paludis/resolver/allowed_to_remove_helper.hh>
#include <paludis/resolver/always_via_binary_helper.hh>
#include <paludis/resolver/can_use_helper.hh>
#include <paludis/resolver/confirm_helper.hh>
#include <paludis/resolver/find_repository_for_helper.hh>
#include <paludis/resolver/get_constraints_for_dependent_helper.hh>
#include <paludis/resolver/get_constraints_for_purge_helper.hh>
#include <paludis/resolver/get_constraints_for_via_binary_helper.hh>
#include <paludis/resolver/get_destination_types_for_error_helper.hh>
#include <paludis/resolver/get_initial_constraints_for_helper.hh>
#include <paludis/resolver/get_resolvents_for_helper.hh>
#include <paludis/resolver/get_use_existing_nothing_helper.hh>
#include <paludis/resolver/interest_in_spec_helper.hh>
#include <paludis/resolver/make_destination_filtered_generator_helper.hh>
#include <paludis/resolver/make_origin_filtered_generator_helper.hh>
#include <paludis/resolver/make_unmaskable_filter_helper.hh>
#include <paludis/resolver/order_early_helper.hh>
#include <paludis/resolver/remove_if_dependent_helper.hh>
#include <paludis/resolver/prefer_or_avoid_helper.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/map-fwd.hh>

#include <paludis/dep_spec-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>

#include <test/test_framework.hh>

#include <memory>
#include <string>
#include <map>
#include <list>

namespace paludis
{
    namespace resolver
    {
        namespace resolver_test
        {
            std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
                    const std::string & k);

            struct ResolverTestCase : test::TestCase
            {
                TestEnvironment env;
                std::shared_ptr<Repository> repo, inst_repo;
                std::shared_ptr<FakeInstalledRepository> fake_inst_repo;

                AllowChoiceChangesHelper allow_choice_changes_helper;
                AllowedToRemoveHelper allowed_to_remove_helper;
                AlwaysViaBinaryHelper always_via_binary_helper;
                CanUseHelper can_use_helper;
                ConfirmHelper confirm_helper;
                FindRepositoryForHelper find_repository_for_helper;
                GetConstraintsForDependentHelper get_constraints_for_dependent_helper;
                GetConstraintsForPurgeHelper get_constraints_for_purge_helper;
                GetConstraintsForViaBinaryHelper get_constraints_for_via_binary_helper;
                GetDestinationTypesForErrorHelper get_destination_types_for_error_helper;
                GetInitialConstraintsForHelper get_initial_constraints_for_helper;
                GetResolventsForHelper get_resolvents_for_helper;
                GetUseExistingNothingHelper get_use_existing_nothing_helper;
                InterestInSpecHelper interest_in_spec_helper;
                MakeDestinationFilteredGeneratorHelper make_destination_filtered_generator_helper;
                MakeOriginFilteredGeneratorHelper make_origin_filtered_generator_helper;
                MakeUnmaskableFilterHelper make_unmaskable_filter_helper;
                OrderEarlyHelper order_early_helper;
                PreferOrAvoidHelper prefer_or_avoid_helper;
                RemoveIfDependentHelper remove_if_dependent_helper;

                ResolverTestCase(const std::string & group, const std::string & test_name, const std::string & eapi,
                        const std::string & layout);

                const std::shared_ptr<const Resolved> get_resolved(const PackageOrBlockDepSpec & target);
                const std::shared_ptr<const Resolved> get_resolved(const std::string & target);

                virtual ResolverFunctions get_resolver_functions();

                struct DecisionChecks
                {
                    typedef std::function<bool (const std::shared_ptr<const Decision> &) > CheckFunction;
                    typedef std::function<std::string (const std::shared_ptr<const Decision> &) > MessageFunction;
                    typedef std::list<std::pair<CheckFunction, MessageFunction> > List;
                    List checks;

                    static std::string check_generic_msg(const std::string & q, const std::shared_ptr<const Decision> & r);

                    static bool check_change(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);
                    static std::string check_change_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);

                    static bool check_remove(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);
                    static std::string check_remove_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);

                    static bool check_unable(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);
                    static std::string check_unable_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);

                    static bool check_breaking(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);
                    static std::string check_breaking_msg(const QualifiedPackageName & q, const std::shared_ptr<const Decision> & r);

                    static bool check_finished(const std::shared_ptr<const Decision> & r);
                    static std::string check_finished_msg(const std::shared_ptr<const Decision> & r);

                    DecisionChecks & change(const QualifiedPackageName & q);
                    DecisionChecks & remove(const QualifiedPackageName & q);
                    DecisionChecks & unable(const QualifiedPackageName & q);
                    DecisionChecks & breaking(const QualifiedPackageName & q);
                    DecisionChecks & finished();
                };

                template <typename Decisions_>
                void check_resolved_one(
                        const std::shared_ptr<Decisions_> &,
                        const std::shared_ptr<const DecisionChecks> &);

                void check_resolved(
                        const std::shared_ptr<const Resolved> &,
                        const NamedValue<n::taken_change_or_remove_decisions, const std::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::taken_unable_to_make_decisions, const std::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::taken_unconfirmed_decisions, const std::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::taken_unorderable_decisions, const std::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::untaken_change_or_remove_decisions, const std::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::untaken_unable_to_make_decisions, const std::shared_ptr<const DecisionChecks> > &
                        );

                const std::shared_ptr<FakePackageID> install(
                        const std::string & c, const std::string & p, const std::string & v);
            };
        }
    }
}

#endif
