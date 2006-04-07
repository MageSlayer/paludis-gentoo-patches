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
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/profiles/profile"));
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
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/profiles/profile"));
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
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/profiles/profile"));
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
            keys.insert(std::make_pair("profile",  "portage_repository_TEST_dir/profiles/profile"));
            PortageRepository::Pointer repo(PortageRepository::make_portage_repository(
                        &env, env.package_database().raw_pointer(), keys));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-three")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-four")));
        }
    } test_portage_repository_has_category_named;
}



