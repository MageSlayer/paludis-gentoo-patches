/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/portage_repository.hh>
#include <paludis/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageRepository.
 *
 * \ingroup Test
 */

namespace test_cases
{
    struct PortageRepositoryRepoNameTest : TestCase
    {
        PortageRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo1"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo1/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_portage_repository_repo_name;

    struct PortageRepositoryNoRepoNameTest : TestCase
    {
        PortageRepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo2"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo2/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK(0 == repo->name().data().substr(
                        repo->name().data().length() - 33).compare("portage_repository_TEST_dir-repo2"));
        }
    } test_portage_repository_no_repo_name;

    struct PortageRepositoryEmptyRepoNameTest : TestCase
    {
        PortageRepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo3"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo3/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK(0 == repo->name().data().substr(
                        repo->name().data().length() - 33).compare("portage_repository_TEST_dir-repo3"));
        }
    } test_portage_repository_empty_repo_name;

    struct PortageRepositoryHasCategoryNamedTest : TestCase
    {
        PortageRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo1"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo1/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-three")));
                TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-four")));
            }
        }
    } test_portage_repository_has_category_named;

    struct PortageRepositoryCategoryNamesTest : TestCase
    {
        PortageRepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo1"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo1/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                CategoryNamePartCollection::ConstPointer c(repo->category_names());
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-one")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-two")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-three")));
                TEST_CHECK(c->end() == c->find(CategoryNamePart("cat-four")));
                TEST_CHECK_EQUAL(3, std::distance(c->begin(), c->end()));
            }
        }
    } test_portage_repository_category_names;

    struct PortageRepositoryHasPackageNamedTest : TestCase
    {
        PortageRepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo4"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo4/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_portage_repository_has_package_named;

    struct PortageRepositoryHasPackageNamedCachedTest : TestCase
    {
        PortageRepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo4"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo4/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            repo->package_names(CategoryNamePart("cat-one"));
            repo->package_names(CategoryNamePart("cat-two"));
            repo->package_names(CategoryNamePart("cat-three"));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_portage_repository_has_package_named_cached;

    struct PortageRepositoryPackageNamesTest : TestCase
    {
        PortageRepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo4"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo4/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            QualifiedPackageNameCollection::ConstPointer names(0);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-two"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-three"));
                TEST_CHECK(names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-neither")));
                TEST_CHECK_EQUAL(0, std::distance(names->begin(), names->end()));
            }
        }
    } test_portage_repository_package_names;

    struct PortageRepositoryBadPackageNamesTest : TestCase
    {
        PortageRepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo5"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo5/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            QualifiedPackageNameCollection::ConstPointer names(0);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK_EQUAL(1, std::distance(names->begin(), names->end()));
            }
        }
    } test_portage_repository_bad_package_names;

    struct PortageRepositoryHasVersionTest : TestCase
    {
        PortageRepositoryHasVersionTest() : TestCase("has version") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "portage"));
            keys.insert(std::make_pair("location", "portage_repository_TEST_dir/repo4"));
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/repo4/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_version(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1")));
                TEST_CHECK(repo->has_version(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1.1-r1")));
                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("2")));

                TEST_CHECK(repo->has_version(QualifiedPackageName("cat-one/pkg-both"), VersionSpec("3.45")));
                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-one/pkg-both"), VersionSpec("1")));
                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-one/pkg-both"), VersionSpec("1.23")));

                TEST_CHECK(repo->has_version(QualifiedPackageName("cat-two/pkg-two"), VersionSpec("2")));
                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-two/pkg-two"), VersionSpec("1")));

                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-two/pkg-both"), VersionSpec("3.45")));
                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-two/pkg-both"), VersionSpec("1")));
                TEST_CHECK(repo->has_version(QualifiedPackageName("cat-two/pkg-both"), VersionSpec("1.23")));

                TEST_CHECK(! repo->has_version(QualifiedPackageName("cat-two/pkg-neither"), VersionSpec("1")));
            }
        }
    } test_portage_repository_has_version;

}

