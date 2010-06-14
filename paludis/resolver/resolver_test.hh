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
#include <paludis/resolver/jobs-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/required_confirmations-fwd.hh>
#include <paludis/resolver/package_or_block_dep_spec-fwd.hh>
#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <test/test_framework.hh>
#include <tr1/memory>
#include <string>
#include <map>
#include <list>

namespace paludis
{
    namespace resolver
    {
        namespace resolver_test
        {
            std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
                    const std::string & k);

            typedef std::map<Resolvent, std::tr1::shared_ptr<Constraints> > InitialConstraints;

            SpecInterest interest_in_spec_fn(
                    const std::tr1::shared_ptr<const Resolution> &,
                    const SanitisedDependency &);

            const std::tr1::shared_ptr<Constraints> initial_constraints_for_fn(
                    const InitialConstraints & initial_constraints,
                    const Resolvent & resolvent);

            const std::tr1::shared_ptr<ConstraintSequence>
            get_constraints_for_dependent_fn(
                    const std::tr1::shared_ptr<const Resolution> &,
                    const std::tr1::shared_ptr<const PackageID> & id,
                    const std::tr1::shared_ptr<const PackageIDSequence> & ids);

            std::tr1::shared_ptr<Resolvents> get_resolvents_for_fn(const PackageDepSpec & spec,
                    const std::tr1::shared_ptr<const SlotName> &,
                    const std::tr1::shared_ptr<const Reason> &);

            bool is_just_suggestion(const SanitisedDependency & dep);

            UseExisting get_use_existing_fn(
                    const Resolvent &,
                    const PackageDepSpec &,
                    const std::tr1::shared_ptr<const Reason> &);

            const std::tr1::shared_ptr<const Repository> find_repository_for_fn(
                    const Environment * const,
                    const std::tr1::shared_ptr<const Resolution> &,
                    const ChangesToMakeDecision &);

            FilteredGenerator make_destination_filtered_generator_fn(const Generator &, const Resolvent &);

            DestinationTypes get_destination_types_for_fn(const PackageDepSpec &,
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Reason> &);

            bool allowed_to_break_fn(
                    const std::tr1::shared_ptr<const QualifiedPackageNameSet> &,
                    const std::tr1::shared_ptr<const PackageID> &);

            bool allowed_to_remove_fn(
                    const std::tr1::shared_ptr<const QualifiedPackageNameSet> &,
                    const std::tr1::shared_ptr<const PackageID> &);

            bool remove_if_dependent_fn(
                    const std::tr1::shared_ptr<const QualifiedPackageNameSet> &,
                    const std::tr1::shared_ptr<const PackageID> &);

            Tribool prefer_or_avoid_fn(
                        const std::tr1::shared_ptr<const Map<QualifiedPackageName, bool> > &,
                        const QualifiedPackageName &);

            bool confirm_fn(
                    const std::tr1::shared_ptr<const Resolution> &,
                    const std::tr1::shared_ptr<const RequiredConfirmation> &);

            struct ResolverTestCase : test::TestCase
            {
                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo, inst_repo;
                std::tr1::shared_ptr<FakeInstalledRepository> fake_inst_repo;
                std::tr1::shared_ptr<QualifiedPackageNameSet> allowed_to_break_names;
                std::tr1::shared_ptr<QualifiedPackageNameSet> allowed_to_remove_names;
                std::tr1::shared_ptr<QualifiedPackageNameSet> remove_if_dependent_names;
                std::tr1::shared_ptr<Map<QualifiedPackageName, bool> > prefer_or_avoid_names;

                ResolverTestCase(const std::string & group, const std::string & test_name, const std::string & eapi,
                        const std::string & layout);

                const std::tr1::shared_ptr<const Resolved> get_resolved(const PackageOrBlockDepSpec & target);
                const std::tr1::shared_ptr<const Resolved> get_resolved(const std::string & target);

                virtual ResolverFunctions get_resolver_functions(InitialConstraints &);

                struct DecisionChecks
                {
                    typedef std::tr1::function<bool (const std::tr1::shared_ptr<const Decision> &) > CheckFunction;
                    typedef std::tr1::function<std::string (const std::tr1::shared_ptr<const Decision> &) > MessageFunction;
                    typedef std::list<std::pair<CheckFunction, MessageFunction> > List;
                    List checks;

                    static std::string check_generic_msg(const std::string & q, const std::tr1::shared_ptr<const Decision> & r);

                    static bool check_change(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);
                    static std::string check_change_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);

                    static bool check_remove(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);
                    static std::string check_remove_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);

                    static bool check_unable(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);
                    static std::string check_unable_msg(const QualifiedPackageName & q, const std::tr1::shared_ptr<const Decision> & r);

                    static bool check_finished(const std::tr1::shared_ptr<const Decision> & r);
                    static std::string check_finished_msg(const std::tr1::shared_ptr<const Decision> & r);

                    DecisionChecks & change(const QualifiedPackageName & q);
                    DecisionChecks & remove(const QualifiedPackageName & q);
                    DecisionChecks & unable(const QualifiedPackageName & q);
                    DecisionChecks & finished();
                };

                template <typename Decisions_>
                void check_resolved_one(
                        const std::tr1::shared_ptr<Decisions_> &,
                        const std::tr1::shared_ptr<const DecisionChecks> &);

                void check_resolved(
                        const std::tr1::shared_ptr<const Resolved> &,
                        const NamedValue<n::display_change_or_remove_decisions, const std::tr1::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::taken_unable_to_make_decisions, const std::tr1::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::untaken_change_or_remove_decisions, const std::tr1::shared_ptr<const DecisionChecks> > &,
                        const NamedValue<n::untaken_unable_to_make_decisions, const std::tr1::shared_ptr<const DecisionChecks> > &
                        );

                const std::tr1::shared_ptr<FakePackageID> install(
                        const std::string & c, const std::string & p, const std::string & v);
            };
        }
    }
}

#endif
