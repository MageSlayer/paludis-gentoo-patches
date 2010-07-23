/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "repository_name_cache.hh"
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/package_database.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct NamesCacheEmptyTest : TestCase
    {
        NamesCacheEmptyTest() : TestCase("empty") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("/var/empty"), repo.get());
            TEST_CHECK(! cache.usable());
        }
    } test_names_cache_empty;

    struct NamesCacheNotGeneratedTest : TestCase
    {
        NamesCacheNotGeneratedTest() : TestCase("not generated") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/not_generated"), repo.get());
            TEST_CHECK(cache.usable());
            TEST_CHECK(! cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(! cache.usable());
        }
    } test_names_cache_not_generated;

    struct NamesCacheNotExistingTest : TestCase
    {
        NamesCacheNotExistingTest() : TestCase("not existing") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/not_existing"), repo.get());
            TEST_CHECK(cache.usable());
            TEST_CHECK(! cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(! cache.usable());
        }
    } test_names_cache_not_existing;

    struct NamesCacheOldFormatTest : TestCase
    {
        NamesCacheOldFormatTest() : TestCase("old format") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/old_format"), repo.get());
            TEST_CHECK(cache.usable());
            TEST_CHECK(! cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(! cache.usable());
        }
    } test_names_cache_old_format;

    struct NamesCacheBadRepoTest : TestCase
    {
        NamesCacheBadRepoTest() : TestCase("bad repo") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/bad_repo"), repo.get());
            TEST_CHECK(cache.usable());
            TEST_CHECK(! cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(! cache.usable());
        }
    } test_names_cache_bad_repo;

    struct NamesCacheGoodTest : TestCase
    {
        NamesCacheGoodTest() : TestCase("good") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/good_repo"), repo.get());
            TEST_CHECK(cache.usable());

            std::shared_ptr<const CategoryNamePartSet> foo(cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(cache.usable());
            TEST_CHECK(bool(foo));
            TEST_CHECK_EQUAL(join(foo->begin(), foo->end(), " "), "bar baz");

            std::shared_ptr<const CategoryNamePartSet> moo(cache.category_names_containing_package(PackageNamePart("moo")));
            TEST_CHECK(cache.usable());
            TEST_CHECK(bool(moo));
            TEST_CHECK(moo->empty());
        }
    } test_names_cache_good;

    struct NamesCacheGenerateTest : TestCase
    {
        NamesCacheGenerateTest() : TestCase("generate") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(10, repo);

            RepositoryNameCache cache(FSEntry("repository_name_cache_TEST_dir/generated"), repo.get());
            repo->add_package(QualifiedPackageName("bar/foo"));
            repo->add_package(QualifiedPackageName("baz/foo"));

            TEST_CHECK(cache.usable());
            cache.regenerate_cache();
            TEST_CHECK(cache.usable());

            std::shared_ptr<const CategoryNamePartSet> foo(cache.category_names_containing_package(PackageNamePart("foo")));
            TEST_CHECK(cache.usable());
            TEST_CHECK(bool(foo));
            TEST_CHECK_EQUAL(join(foo->begin(), foo->end(), " "), "bar baz");

            std::shared_ptr<const CategoryNamePartSet> moo(cache.category_names_containing_package(PackageNamePart("moo")));
            TEST_CHECK(cache.usable());
            TEST_CHECK(bool(moo));
            TEST_CHECK(moo->empty());
        }
    } test_names_cache_generate;
}

