/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/e/qa/spec_keys.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/qa.hh>
#include <paludis/util/options.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::erepository;
using namespace test;

namespace
{
    struct TestReporter :
        QAReporter
    {
        unsigned count;

        TestReporter() :
            count(0)
        {
        }

        void message(const QAMessage &)
        {
            ++count;
        }

        void status(const std::string &)
        {
        }
    };
}

namespace test_cases
{
    struct GoodTest : TestCase
    {
        GoodTest() : TestCase("good") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("cat/other");

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 0u);
        }
    } test_good;

    struct EmptyBlockTest : TestCase
    {
        EmptyBlockTest() : TestCase("empty block") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakePackageID> id1(repo->add_version("cat", "pkg", "1"));
            id1->build_dependencies_key()->set_from_string("( ( ) )");
            TestReporter r1;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r1, id1, "spec keys"));
            TEST_CHECK_EQUAL(r1.count, 1u);

            tr1::shared_ptr<FakePackageID> id2(repo->add_version("cat", "pkg", "2"));
            id2->build_dependencies_key()->set_from_string("|| ( )");
            TestReporter r2;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r2, id2, "spec keys"));
            TEST_CHECK_EQUAL(r2.count, 1u);

            tr1::shared_ptr<FakePackageID> id3(repo->add_version("cat", "pkg", "3"));
            id3->build_dependencies_key()->set_from_string("x? ( )");
            id3->iuse_key()->set_from_string("x", IUseFlagParseOptions());
            TestReporter r3;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r3, id3, "spec keys"));
            TEST_CHECK_EQUAL(r3.count, 1u);

            tr1::shared_ptr<FakePackageID> id4(repo->add_version("cat", "pkg", "4"));
            id4->build_dependencies_key()->set_from_string("x? ( ) ( y? ( || ( ) ) )");
            id4->iuse_key()->set_from_string("x y", IUseFlagParseOptions());
            TestReporter r4;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r4, id4, "spec keys"));
            TEST_CHECK_EQUAL(r4.count, 2u);
        }
    } test_empty_block;

    struct AnyUseTest : TestCase
    {
        AnyUseTest() : TestCase("|| ( use? ( ) )") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("|| ( v/w x? ( x/y ) )");
            id->iuse_key()->set_from_string("x", IUseFlagParseOptions());

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_any_use;

    struct AnyOneTest : TestCase
    {
        AnyOneTest() : TestCase("|| ( one )") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("|| ( x/x )");

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_any_one;

    struct AnyBlockTest : TestCase
    {
        AnyBlockTest() : TestCase("|| ( !block )") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("|| ( x/x !y/y z/z )");

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_any_block;

    struct DeprecatedTest : TestCase
    {
        DeprecatedTest() : TestCase("deprecated") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("virtual", "libc", "1");

            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("virtual/libc");

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_deprecated;

    struct RecursiveUseTest : TestCase
    {
        RecursiveUseTest() : TestCase("recursive use") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakePackageID> id1(repo->add_version("cat", "pkg", "1"));
            id1->build_dependencies_key()->set_from_string("x? ( x? ( cat/pkg ) )");
            id1->iuse_key()->set_from_string("x", IUseFlagParseOptions());

            TestReporter r1;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r1, id1, "spec keys"));
            TEST_CHECK_EQUAL(r1.count, 1u);

            tr1::shared_ptr<FakePackageID> id2(repo->add_version("cat", "pkg", "2"));
            id2->build_dependencies_key()->set_from_string("x? ( !x? ( cat/pkg ) )");
            id2->iuse_key()->set_from_string("x", IUseFlagParseOptions());

            TestReporter r2;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r2, id2, "spec keys"));
            TEST_CHECK_EQUAL(r2.count, 1u);
        }
    } test_recursive_use;

    struct MissingIUseTest : TestCase
    {
        MissingIUseTest() : TestCase("missing iuse") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("foo? ( cat/pkg1 )");

            TestReporter r;
            TEST_CHECK(spec_keys_check(FSEntry("/var/empty"), r, id, "spec keys"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_missing_iuse;
}

