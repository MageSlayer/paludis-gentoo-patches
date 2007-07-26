/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "paludis_environment.hh"
#include "paludis_config.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/query.hh>
#include <paludis/package_id.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <cstdlib>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TestPaludisEnvironmentUse : TestCase
    {
        TestPaludisEnvironmentUse() : TestCase("use") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home1").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));
            const tr1::shared_ptr<const PackageID> one(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-one-1", pds_pm_permissive)), qo_require_exactly_one)->begin());
            const tr1::shared_ptr<const PackageID> three(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-two-3", pds_pm_permissive)), qo_require_exactly_one)->begin());

            TEST_CHECK(env->query_use(UseFlagName("foo"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), *one));
            TEST_CHECK(env->query_use(UseFlagName("moo"), *one));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), *one));
            TEST_CHECK(env->query_use(UseFlagName("exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), *one));

            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), *three));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), *three));
        }
    } paludis_environment_use_test;

    struct TestPaludisEnvironmentKnownUse : TestCase
    {
        TestPaludisEnvironmentKnownUse() : TestCase("known use") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home5").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const tr1::shared_ptr<const PackageID> one(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-one-1", pds_pm_permissive)), qo_require_exactly_one)->begin());
            tr1::shared_ptr<const UseFlagNameSet> k1(env->known_use_expand_names(UseFlagName("foo_cards"), *one));
            TEST_CHECK_EQUAL(join(k1->begin(), k1->end(), " "), "foo_cards_one foo_cards_three foo_cards_two");
        }
    } paludis_environment_use_test_known;

    struct TestPaludisEnvironmentUseMinusStar : TestCase
    {
        TestPaludisEnvironmentUseMinusStar() : TestCase("use -*") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home2").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const tr1::shared_ptr<const PackageID> one(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-one-1", pds_pm_permissive)), qo_require_exactly_one)->begin());
            const tr1::shared_ptr<const PackageID> three(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-two-3", pds_pm_permissive)), qo_require_exactly_one)->begin());

            TEST_CHECK(env->query_use(UseFlagName("foo"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("moo"), *one));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), *one));

            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), *three));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), *three));
        }
    } paludis_environment_use_test_minus_star;

    struct TestPaludisEnvironmentUseMinusPartialStar : TestCase
    {
        TestPaludisEnvironmentUseMinusPartialStar() : TestCase("use -* partial") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home3").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const tr1::shared_ptr<const PackageID> one(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-one-1", pds_pm_permissive)), qo_require_exactly_one)->begin());
            const tr1::shared_ptr<const PackageID> three(*env->package_database()->query(
                        query::Matches(PackageDepSpec("=cat-one/pkg-two-3", pds_pm_permissive)), qo_require_exactly_one)->begin());

            TEST_CHECK(env->query_use(UseFlagName("foo"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), *one));
            TEST_CHECK(env->query_use(UseFlagName("moo"), *one));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), *one));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), *one));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), *one));

            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), *three));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), *three));
        }
    } paludis_environment_use_test_minus_star_partial;

    struct TestPaludisEnvironmentRepositories : TestCase
    {
        TestPaludisEnvironmentRepositories() : TestCase("repositories") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home4").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            TEST_CHECK(env->package_database()->fetch_repository(RepositoryName("first")));
            TEST_CHECK(env->package_database()->fetch_repository(RepositoryName("second")));
            TEST_CHECK(env->package_database()->fetch_repository(RepositoryName("third")));
            TEST_CHECK(env->package_database()->fetch_repository(RepositoryName("fourth")));
            TEST_CHECK(env->package_database()->fetch_repository(RepositoryName("fifth")));

            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("first"), RepositoryName("second")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("third")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("third")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("fifth")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("fifth")));
        }
    } paludis_environment_repositories;
}

