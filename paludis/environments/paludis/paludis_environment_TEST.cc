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
#include <test/test_runner.hh>
#include <test/test_framework.hh>
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

            std::tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            PackageDatabaseEntry x(QualifiedPackageName("x/x"), VersionSpec("0"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), x));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), x));

            PackageDatabaseEntry pde(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), pde));
            TEST_CHECK(env->query_use(UseFlagName("moo"), pde));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), pde));
            TEST_CHECK(env->query_use(UseFlagName("exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), pde));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), f));
        }
    } paludis_environment_use_test;

    struct TestPaludisEnvironmentUseMinusStar : TestCase
    {
        TestPaludisEnvironmentUseMinusStar() : TestCase("use -*") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home2").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            PackageDatabaseEntry x(QualifiedPackageName("x/x"), VersionSpec("0"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), x));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), x));

            PackageDatabaseEntry pde(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("moo"), pde));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), pde));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), f));
        }
    } paludis_environment_use_test_minus_star;

    struct TestPaludisEnvironmentUseMinusPartialStar : TestCase
    {
        TestPaludisEnvironmentUseMinusPartialStar() : TestCase("use -* partial") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home3").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

            PackageDatabaseEntry x(QualifiedPackageName("x/x"), VersionSpec("0"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), x));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), x));

            PackageDatabaseEntry pde(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), pde));
            TEST_CHECK(env->query_use(UseFlagName("moo"), pde));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), pde));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), pde));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), pde));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), f));
        }
    } paludis_environment_use_test_minus_star_partial;

    struct TestPaludisEnvironmentRepositories : TestCase
    {
        TestPaludisEnvironmentRepositories() : TestCase("repositories") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home4").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::tr1::shared_ptr<Environment> env(new PaludisEnvironment(""));

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

