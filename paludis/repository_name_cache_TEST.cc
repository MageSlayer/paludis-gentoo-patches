/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repository_name_cache.hh>

#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>

#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(RepositoryNameCache, Empty)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("/var/empty"), repo.get());
    EXPECT_TRUE(! cache.usable());
}

TEST(RepositoryNameCache, NotGenerated)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/not_generated"), repo.get());
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(! cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(! cache.usable());
}

TEST(RepositoryNameCache, NotExisting)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/not_existing"), repo.get());
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(! cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(! cache.usable());
}

TEST(RepositoryNameCache, OldFormat)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/old_format"), repo.get());
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(! cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(! cache.usable());
}

TEST(RepositoryNameCache, BadRepo)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/bad_repo"), repo.get());
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(! cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(! cache.usable());
}

TEST(RepositoryNameCache, Good)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/good_repo"), repo.get());
    EXPECT_TRUE(cache.usable());

    std::shared_ptr<const CategoryNamePartSet> foo(cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(bool(foo));
    EXPECT_EQ("bar baz", join(foo->begin(), foo->end(), " "));

    std::shared_ptr<const CategoryNamePartSet> moo(cache.category_names_containing_package(PackageNamePart("moo")));
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(bool(moo));
    EXPECT_TRUE(moo->empty());
}

TEST(RepositoryNameCache, Generate)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(10, repo);

    RepositoryNameCache cache(FSPath("repository_name_cache_TEST_dir/generated"), repo.get());
    repo->add_package(QualifiedPackageName("bar/foo"));
    repo->add_package(QualifiedPackageName("baz/foo"));

    EXPECT_TRUE(cache.usable());
    cache.regenerate_cache();
    EXPECT_TRUE(cache.usable());

    std::shared_ptr<const CategoryNamePartSet> foo(cache.category_names_containing_package(PackageNamePart("foo")));
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(bool(foo));
    EXPECT_EQ("bar baz", join(foo->begin(), foo->end(), " "));

    std::shared_ptr<const CategoryNamePartSet> moo(cache.category_names_containing_package(PackageNamePart("moo")));
    EXPECT_TRUE(cache.usable());
    EXPECT_TRUE(bool(moo));
    EXPECT_TRUE(moo->empty());
}

