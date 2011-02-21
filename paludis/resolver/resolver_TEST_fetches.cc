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
    struct ResolverFetchTestCase : ResolverTestCase
    {
        ResolverFetchTestCase(const std::string & s) :
            ResolverTestCase("fetch", s, "exheres-0", "exheres")
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
    struct TestFetchDeps : ResolverFetchTestCase
    {
        TestFetchDeps() : ResolverFetchTestCase("fetch") { }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("fetch/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("fetch/fetch-dep"))
                        .change(QualifiedPackageName("fetch/target"))
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

            const FetchJob * const fetch_fetch_dep_job(visitor_cast<const FetchJob>(**resolved->job_lists()->execute_job_list()->fetch(0)));
            TEST_CHECK(fetch_fetch_dep_job);
            TEST_CHECK_EQUAL(join(fetch_fetch_dep_job->requirements()->begin(), fetch_fetch_dep_job->requirements()->end(), ", ", stringify_req),
                    "");

            const InstallJob * const fetch_dep_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(1)));
            TEST_CHECK(fetch_dep_job);
            TEST_CHECK_EQUAL(join(fetch_dep_job->requirements()->begin(), fetch_dep_job->requirements()->end(), ", ", stringify_req),
                    "0 satisfied independent always");

            const FetchJob * const fetch_target_job(visitor_cast<const FetchJob>(**resolved->job_lists()->execute_job_list()->fetch(2)));
            TEST_CHECK(fetch_target_job);
            TEST_CHECK_EQUAL(join(fetch_target_job->requirements()->begin(), fetch_target_job->requirements()->end(), ", ", stringify_req),
                    "1 satisfied, 1 independent");

            const InstallJob * const target_job(visitor_cast<const InstallJob>(**resolved->job_lists()->execute_job_list()->fetch(3)));
            TEST_CHECK(target_job);
            TEST_CHECK_EQUAL(join(target_job->requirements()->begin(), target_job->requirements()->end(), ", ", stringify_req),
                    "2 satisfied independent always, 1 independent");
        }
    } test_fetch_dep;
}

