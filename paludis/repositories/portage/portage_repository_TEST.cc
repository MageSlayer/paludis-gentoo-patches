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

#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/repositories/portage/make_ebuild_repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/environment/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageRepository.
 *
 */

namespace test_cases
{
    /**
     * \test Test PortageRepository repository names.
     *
     */
    struct PortageRepositoryRepoNameTest : TestCase
    {
        PortageRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profile", "portage_repository_TEST_dir/repo1/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_portage_repository_repo_name;

    /**
     * \test Test PortageRepository repository with no names.
     *
     */
    struct PortageRepositoryNoRepoNameTest : TestCase
    {
        PortageRepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo2");
            keys->insert("profile", "portage_repository_TEST_dir/repo2/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo2");
        }
    } test_portage_repository_no_repo_name;

    /**
     * \test Test PortageRepository repository empty names.
     *
     */
    struct PortageRepositoryEmptyRepoNameTest : TestCase
    {
        PortageRepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo3");
            keys->insert("profile", "portage_repository_TEST_dir/repo3/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo3");
        }
    } test_portage_repository_empty_repo_name;

    /**
     * \test Test PortageRepository repository has_category_named.
     *
     */
    struct PortageRepositoryHasCategoryNamedTest : TestCase
    {
        PortageRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profile", "portage_repository_TEST_dir/repo1/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository category_names.
     *
     */
    struct PortageRepositoryCategoryNamesTest : TestCase
    {
        PortageRepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profile", "portage_repository_TEST_dir/repo1/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository has_package_named.
     *
     */
    struct PortageRepositoryHasPackageNamedTest : TestCase
    {
        PortageRepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profile", "portage_repository_TEST_dir/repo4/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository has_package_named cached.
     *
     */
    struct PortageRepositoryHasPackageNamedCachedTest : TestCase
    {
        PortageRepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profile", "portage_repository_TEST_dir/repo4/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository package_names.
     *
     */
    struct PortageRepositoryPackageNamesTest : TestCase
    {
        PortageRepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profile", "portage_repository_TEST_dir/repo4/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository bad package names.
     *
     */
    struct PortageRepositoryBadPackageNamesTest : TestCase
    {
        PortageRepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo5");
            keys->insert("profile", "portage_repository_TEST_dir/repo5/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository has_version.
     *
     */
    struct PortageRepositoryHasVersionTest : TestCase
    {
        PortageRepositoryHasVersionTest() : TestCase("has version") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profile", "portage_repository_TEST_dir/repo4/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
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

    /**
     * \test Test PortageRepository versions.
     *
     */
    struct PortageRepositoryVersionsTest : TestCase
    {
        PortageRepositoryVersionsTest() : TestCase("versions") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profile", "portage_repository_TEST_dir/repo4/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                VersionSpecCollection::ConstPointer versions(0);

                versions = repo->version_specs(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(versions->end() != versions->find(VersionSpec("1")));
                TEST_CHECK(versions->end() != versions->find(VersionSpec("1.1-r1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("2")));

                versions = repo->version_specs(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("1.1-r1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("2")));
            }
        }
    } test_portage_repository_versions;

    /**
     * \test Test PortageRepository duff versions.
     *
     */
    struct PortageRepositoryDuffVersionsTest : TestCase
    {
        PortageRepositoryDuffVersionsTest() : TestCase("duff versions") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo8");
            keys->insert("profile", "portage_repository_TEST_dir/repo8/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                VersionSpecCollection::ConstPointer versions(0);

                versions = repo->version_specs(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(versions->end() != versions->find(VersionSpec("1")));
                TEST_CHECK(versions->end() != versions->find(VersionSpec("1.1-r1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("2")));

                versions = repo->version_specs(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("1.1-r1")));
                TEST_CHECK(versions->end() == versions->find(VersionSpec("2")));
            }
        }
    } test_portage_repository_duff_versions;

    /**
     * \test Test PortageRepository cached metadata.
     *
     */
    struct PortageRepositoryMetadataCachedTest : TestCase
    {
        PortageRepositoryMetadataCachedTest() : TestCase("metadata cached") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo6");
            keys->insert("profile", "portage_repository_TEST_dir/repo6/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                VersionMetadata::ConstPointer m(0);

                m = repo->version_metadata(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"));
                TEST_CHECK_EQUAL(m->description, "the-description");

                TEST_CHECK_THROWS(repo->version_metadata(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("2")),
                        NoSuchPackageError);
                TEST_CHECK_THROWS(repo->version_metadata(QualifiedPackageName("cat-two/pkg-one"), VersionSpec("1")),
                        NoSuchPackageError);
            }
        }
    } test_portage_repository_metadata_cached;

    /**
     * \test Test PortageRepository uncached metadata.
     *
     */
    struct PortageRepositoryMetadataUncachedTest : TestCase
    {
        PortageRepositoryMetadataUncachedTest() : TestCase("metadata uncached") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo7");
            keys->insert("profile", "portage_repository_TEST_dir/repo7/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                VersionMetadata::ConstPointer m(0);

                m = repo->version_metadata(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"));
                TEST_CHECK_EQUAL(m->description, "The Description");
                TEST_CHECK_EQUAL(m->eapi, "0");
            }
        }
    } test_portage_repository_metadata_uncached;

    /**
     * \test Test PortageRepository unparsable metadata.
     *
     */
    struct PortageRepositoryMetadataUnparsableTest : TestCase
    {
        PortageRepositoryMetadataUnparsableTest() : TestCase("metadata unparsable") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo7");
            keys->insert("profile", "portage_repository_TEST_dir/repo7/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                VersionMetadata::ConstPointer m(0);

                m = repo->version_metadata(QualifiedPackageName("cat-one/pkg-two"), VersionSpec("1"));
                TEST_CHECK_EQUAL(m->eapi, "UNKNOWN");
            }
        }
    } test_portage_repository_metadata_unparsable;

    /**
     * \test Test PortageRepository query_use and query_use_mask functions.
     *
     */
    struct PortageRepositoryQueryUseTest : TestCase
    {
        PortageRepositoryQueryUseTest() : TestCase("USE query") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo9");
            keys->insert("profile", "portage_repository_TEST_dir/repo9/profiles/profile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                PackageDatabaseEntry p1(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"),
                        RepositoryName("test-repo-9"));
                PackageDatabaseEntry p2(QualifiedPackageName("cat-two/pkg-two"), VersionSpec("1"),
                        RepositoryName("test-repo-9"));
                PackageDatabaseEntry p3(QualifiedPackageName("cat-one/pkg-none"), VersionSpec("1"),
                        RepositoryName("test-repo-9"));
                PackageDatabaseEntry p4(QualifiedPackageName("cat-one/pkg-one"), VersionSpec("2"),
                        RepositoryName("test-repo-9"));

                TEST_CHECK(repo->query_use(UseFlagName("flag1"), &p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag2"), &p1) == use_disabled);
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag2"), &p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag2"), &p3));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), &p2));
                TEST_CHECK(! repo->query_use_mask(UseFlagName("flag3"), &p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), &p4));
                TEST_CHECK(repo->query_use(UseFlagName("flag3"), &p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag4"), &p3) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), &p2) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), &p1) == use_disabled);
            }
        }
    } test_portage_repository_query_use;

    /**
     * \test Test PortageRepository query_profile_masks functions.
     *
     */
    struct PortageRepositoryQueryProfileMasksTest : TestCase
    {
        PortageRepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "portage");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo10");
            keys->insert("profiles", "portage_repository_TEST_dir/repo10/profiles/profile/subprofile");
            PortageRepository::Pointer repo(make_ebuild_repository(
                        &env, env.package_database().raw_pointer(), keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->query_profile_masks(QualifiedPackageName("cat/masked"),
                            VersionSpec("0")));
                TEST_CHECK(! repo->query_profile_masks(QualifiedPackageName("cat/not_masked"),
                            VersionSpec("0")));
                TEST_CHECK(! repo->query_profile_masks(QualifiedPackageName("cat/was_masked"),
                            VersionSpec("0")));
            }
        }
    } test_portage_repository_query_profile_masks;
}

