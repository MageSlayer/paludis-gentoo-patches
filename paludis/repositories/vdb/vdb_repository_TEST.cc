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

#include <paludis/repositories/vdb/vdb_repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/environment/test/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <fstream>
#include <iterator>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VDBRepository.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test VDBRepository repo names
     *
     * \ingroup grptestcases
     */
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    /**
     * \test Test VDBRepository has_category_named
     *
     * \ingroup grptestcases
     */
    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    /**
     * \test Test VDBRepository query_use
     *
     * \ingroup grptestcases
     */
    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));

            PackageDatabaseEntry e1(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"),
                    VersionSpec("1"), RepositoryName("installed"));
            PackageDatabaseEntry e2(CategoryNamePart("cat-one") + PackageNamePart("pkg-neither"),
                    VersionSpec("1"), RepositoryName("installed"));

            TEST_CHECK(repo->query_use(UseFlagName("flag1"), &e1) == use_enabled);
            TEST_CHECK(repo->query_use(UseFlagName("flag2"), &e1) == use_enabled);
            TEST_CHECK(repo->query_use(UseFlagName("flag3"), &e1) == use_disabled);

            TEST_CHECK(repo->query_use(UseFlagName("flag4"), &e2) == use_unspecified);
        }
    } test_vdb_repository_query_use;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNewFileTest : TestCase
    {
        VDBRepositoryAddToWorldNewFileTest() : TestCase("add to world (new file)") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-new-file");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            repo->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-new-file");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_new_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldEmptyFileTest : TestCase
    {
        VDBRepositoryAddToWorldEmptyFileTest() : TestCase("add to world (empty file)") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-empty");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            repo->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-empty");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_empty_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchTest() : TestCase("add to world (no match)") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            repo->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldMatchTest : TestCase
    {
        VDBRepositoryAddToWorldMatchTest() : TestCase("add to world (match)") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-match");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            repo->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/foofoo\ncat-one/bar\n");
        }
    } test_vdb_repository_add_to_world_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchNoEOLTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchNoEOLTest() : TestCase("add to world (no match, no trailing eol)") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "vdb");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match-no-eol");
            VDBRepository::Pointer repo(VDBRepository::make_vdb_repository(
                        &env, env.package_database().raw_pointer(), keys));
            repo->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match-no-eol");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match_no_eol;
}

