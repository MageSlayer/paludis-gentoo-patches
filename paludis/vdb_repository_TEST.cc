/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/vdb_repository.hh>
#include <paludis/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VDBRepository.
 *
 * \ingroup Test
 */

namespace test_cases
{
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "vdb"));
            keys.insert(std::make_pair("location", "vdb_repository_TEST_dir/repo1"));
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "vdb"));
            keys.insert(std::make_pair("location", "vdb_repository_TEST_dir/repo1"));
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            std::map<std::string, std::string> keys;
            keys.insert(std::make_pair("format",   "vdb"));
            keys.insert(std::make_pair("location", "vdb_repository_TEST_dir/repo1"));
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));

            PackageDatabaseEntry e1(QualifiedPackageName(CategoryNamePart("cat-one"), PackageNamePart("pkg-one")),
                    VersionSpec("1"), RepositoryName("installed"));
            PackageDatabaseEntry e2(QualifiedPackageName(CategoryNamePart("cat-one"), PackageNamePart("pkg-neither")),
                    VersionSpec("1"), RepositoryName("installed"));

            TEST_CHECK(repo->query_use(UseFlagName("flag1"), &e1) == use_enabled);
            TEST_CHECK(repo->query_use(UseFlagName("flag2"), &e1) == use_enabled);
            TEST_CHECK(repo->query_use(UseFlagName("flag3"), &e1) == use_disabled);

            TEST_CHECK(repo->query_use(UseFlagName("flag4"), &e2) == use_unspecified);
        }
    } test_vdb_repository_query_use;
}

