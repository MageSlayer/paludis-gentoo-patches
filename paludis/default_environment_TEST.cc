/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "default_environment.hh"
#include "default_config.hh"
#include <paludis/util/fs_entry.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <cstdlib>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TestDefaultEnvironmentUse : TestCase
    {
        TestDefaultEnvironmentUse() : TestCase("use") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "default_environment_TEST_dir" / "home1").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");
            DefaultConfig::destroy_instance();
            DefaultEnvironment::destroy_instance();

            Environment * env(DefaultEnvironment::get_instance());

            TEST_CHECK(env->query_use(UseFlagName("foo"), 0));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), 0));

            PackageDatabaseEntry e(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), &e));
            TEST_CHECK(env->query_use(UseFlagName("moo"), &e));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), &e));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), &e));
            TEST_CHECK(env->query_use(UseFlagName("exp_one"), &e));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), &e));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), &f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), &f));
        }
    } default_environment_use_test;

    struct TestDefaultEnvironmentUseMinusStar : TestCase
    {
        TestDefaultEnvironmentUseMinusStar() : TestCase("use -*") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "default_environment_TEST_dir" / "home2").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");
            DefaultConfig::destroy_instance();
            DefaultEnvironment::destroy_instance();

            Environment * env(DefaultEnvironment::get_instance());

            TEST_CHECK(env->query_use(UseFlagName("foo"), 0));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), 0));

            PackageDatabaseEntry e(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("moo"), &e));

            TEST_CHECK(! env->query_use(UseFlagName("more_exp_one"), &e));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), &e));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), &f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), &f));
        }
    } default_environment_use_test_minus_star;

    struct TestDefaultEnvironmentUseMinusPartialStar : TestCase
    {
        TestDefaultEnvironmentUseMinusPartialStar() : TestCase("use -* partial") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "default_environment_TEST_dir" / "home3").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");
            DefaultConfig::destroy_instance();
            DefaultEnvironment::destroy_instance();

            Environment * env(DefaultEnvironment::get_instance());

            TEST_CHECK(env->query_use(UseFlagName("foo"), 0));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), 0));

            PackageDatabaseEntry e(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), RepositoryName("foo"));
            TEST_CHECK(env->query_use(UseFlagName("foo"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("foofoo"), &e));
            TEST_CHECK(env->query_use(UseFlagName("moo"), &e));

            TEST_CHECK(env->query_use(UseFlagName("more_exp_one"), &e));
            TEST_CHECK(env->query_use(UseFlagName("exp_two"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("exp_one"), &e));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_one"), &e));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_two"), &e));

            PackageDatabaseEntry f(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("3"), RepositoryName("foo"));
            TEST_CHECK(! env->query_use(UseFlagName("third_exp_one"), &f));
            TEST_CHECK(env->query_use(UseFlagName("third_exp_two"), &f));
        }
    } default_environment_use_test_minus_star_partial;
}

