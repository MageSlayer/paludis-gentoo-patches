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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/job_lists.hh>
#include <paludis/resolver/job_list.hh>
#include <paludis/resolver/job.hh>
#include <paludis/resolver/job_requirements.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_database.hh>

#include <paludis/resolver/resolver_test.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <list>
#include <functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;
using namespace test;

namespace
{
    struct ResolverContinueOnFailureTestCase : ResolverTestCase
    {
        ResolverContinueOnFailureTestCase(const std::string & s) :
            ResolverTestCase("continue_on_failure", s, "exheres-0", "exheres")
        {
        }
    };

    std::string
    stringify_req(const JobRequirement & r)
    {
        std::stringstream result;
        result << r.job_number();
        if (r.required_if()[jri_require_for_satisfied])
            result << " satisfied";
        if (r.required_if()[jri_require_for_independent])
            result << " independent";
        if (r.required_if()[jri_require_always])
            result << " always";
        return result.str();
    }
}

namespace test_cases
{
    struct TestContinueOnFailure : ResolverContinueOnFailureTestCase
    {
        const bool direct_dep_installed;

        TestContinueOnFailure(const bool d) :
            ResolverContinueOnFailureTestCase("continue on failure " + stringify(d)),
            direct_dep_installed(d)
        {
            if (d)
                install("continue-on-failure", "direct-dep", "0");
            install("continue-on-failure", "unchanged-dep", "1")->build_dependencies_key()->set_from_string("continue-on-failure/indirect-dep");

            get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same);
            get_use_existing_nothing_helper.set_use_existing_for_targets(ue_if_same);
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("continue-on-failure/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("continue-on-failure/direct-dep"))
                        .change(QualifiedPackageName("continue-on-failure/indirect-dep"))
                        .change(QualifiedPackageName("continue-on-failure/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );

            TEST_CHECK_EQUAL(resolved->job_lists()->execute_job_list()->length(), 6);

            const InstallJob * const direct_dep_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(1)));
            TEST_CHECK(direct_dep_job);
            TEST_CHECK_EQUAL(join(direct_dep_job->requirements()->begin(), direct_dep_job->requirements()->end(), ", ", stringify_req),
                    "0 satisfied independent always");

            const InstallJob * const indirect_dep_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(3)));
            TEST_CHECK(indirect_dep_job);
            TEST_CHECK_EQUAL(join(indirect_dep_job->requirements()->begin(), indirect_dep_job->requirements()->end(), ", ", stringify_req),
                    "2 satisfied independent always");

            const InstallJob * const target_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(5)));
            TEST_CHECK(target_job);
            if (direct_dep_installed)
                TEST_CHECK_EQUAL(join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req),
                        "4 satisfied independent always, 3 independent, 1 independent");
            else
                TEST_CHECK_EQUAL(join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req),
                        "4 satisfied independent always, 1 satisfied, 3 independent, 1 independent");
        }
    } test_continue_on_failure_false(false), test_continue_on_failure_true(true);

    struct TestUninstallContinueOnFailure : ResolverContinueOnFailureTestCase
    {
        TestUninstallContinueOnFailure() :
            ResolverContinueOnFailureTestCase("uninstall continue on failure")
        {
            install("continue-on-failure-uninstall", "dep-of-dep", "1")->build_dependencies_key()->set_from_string("");
            install("continue-on-failure-uninstall", "dep", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/dep-of-dep");
            install("continue-on-failure-uninstall", "target", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/dep");
            install("continue-on-failure-uninstall", "needs-target", "1")->build_dependencies_key()->set_from_string("continue-on-failure-uninstall/target");

            get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/dep-of-dep", &env, { }));
            get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/dep", &env, { }));
            get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/target", &env, { }));

            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/needs-target", &env, { }));

            remove_if_dependent_helper.add_remove_if_dependent_spec(parse_user_package_dep_spec("continue-on-failure-uninstall/needs-target", &env, { }));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved(BlockDepSpec(
                            "!continue-on-failure-uninstall/target",
                            parse_user_package_dep_spec("continue-on-failure-uninstall/target", &env, { }),
                            bk_weak)));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("continue-on-failure-uninstall/needs-target"))
                        .remove(QualifiedPackageName("continue-on-failure-uninstall/target"))
                        .remove(QualifiedPackageName("continue-on-failure-uninstall/dep"))
                        .remove(QualifiedPackageName("continue-on-failure-uninstall/dep-of-dep"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );

            TEST_CHECK_EQUAL(resolved->job_lists()->execute_job_list()->length(), 4);

            const UninstallJob * const needs_target_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(0)));
            TEST_CHECK(needs_target_job);
            TEST_CHECK_EQUAL(join(needs_target_job->requirements()->begin(), needs_target_job->requirements()->end(), ", ", stringify_req),
                    "");

            const UninstallJob * const target_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(1)));
            TEST_CHECK(target_job);
            TEST_CHECK_EQUAL(join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req),
                    "0 satisfied");

            const UninstallJob * const dep_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(2)));
            TEST_CHECK(dep_job);
            TEST_CHECK_EQUAL(join(dep_job->requirements()->begin(), dep_job->requirements()->end(), ", ", stringify_req),
                    "1 satisfied");

            const UninstallJob * const dep_of_dep_job(visitor_cast<const UninstallJob>(**resolved->job_lists()->execute_job_list()->fetch(3)));
            TEST_CHECK(dep_of_dep_job);
            TEST_CHECK_EQUAL(join(dep_of_dep_job->requirements()->begin(), dep_of_dep_job->requirements()->end(), ", ", stringify_req),
                    "2 satisfied");
        }
    } test_uninstall_continue_on_failure_uninstall;
}

