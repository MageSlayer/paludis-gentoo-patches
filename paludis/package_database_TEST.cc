/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for package_database.hh.
 *
 */

namespace test_cases
{
    struct PackageDatabaseRepositoryTest : TestCase
    {
        PackageDatabaseRepositoryTest() : TestCase("package database repository") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            const std::shared_ptr<FakeRepository> r1(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &e,
                            n::name() = RepositoryName("repo1")
                            )));
            const std::shared_ptr<FakeRepository> r2(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &e,
                            n::name() = RepositoryName("repo2")
                            )));

            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo1")), NoSuchRepositoryError);
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

            p.add_repository(10, r1);
            TEST_CHECK(bool(p.fetch_repository(RepositoryName("repo1"))));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

            TEST_CHECK_THROWS(p.add_repository(10, r1), DuplicateRepositoryError);

            p.add_repository(11, r2);
            TEST_CHECK(bool(p.fetch_repository(RepositoryName("repo1"))));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK(bool(p.fetch_repository(RepositoryName("repo2"))));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo2"))->name(),
                    RepositoryName("repo2"));

            TEST_CHECK_THROWS(p.add_repository(10, r1), DuplicateRepositoryError);
            TEST_CHECK_THROWS(p.add_repository(5, r2), DuplicateRepositoryError);

            TEST_CHECK(bool(p.fetch_repository(RepositoryName("repo1"))));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK(bool(p.fetch_repository(RepositoryName("repo2"))));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo2"))->name(),
                    RepositoryName("repo2"));

            TEST_CHECK(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo2")));
            TEST_CHECK(p.more_important_than(RepositoryName("repo2"), RepositoryName("repo1")));
            TEST_CHECK(! p.more_important_than(RepositoryName("repo2"), RepositoryName("repo2")));
            TEST_CHECK(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo1")));
        }
    } package_database_repository_test;

    struct PackageDatabaseDisambiguateTest : TestCase
    {
        PackageDatabaseDisambiguateTest() : TestCase("package database disambiguate") { }

        struct CoolFakeRepository :
            FakeRepository
        {
            CoolFakeRepository(const Environment * const e, const RepositoryName & rn) :
                FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = e,
                            n::name() = rn
                            ))
            {
            }

            std::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const
            {
                std::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                result->insert(CategoryNamePart("bad-cat1"));
                result->insert(CategoryNamePart("bad-cat2"));
                return result;
            }
        };

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            std::shared_ptr<FakeRepository> r1(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &e,
                            n::name() = RepositoryName("repo1"))));
            r1->add_version(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"), VersionSpec("0", VersionSpecOptions()));
            r1->add_version(CategoryNamePart("cat-one") + PackageNamePart("pkg-two"), VersionSpec("0", VersionSpecOptions()));
            r1->add_version(CategoryNamePart("cat-two") + PackageNamePart("pkg-two"), VersionSpec("0", VersionSpecOptions()));
            r1->add_version(CategoryNamePart("cat-two") + PackageNamePart("pkg-three"), VersionSpec("0", VersionSpecOptions()));
            p.add_repository(10, r1);
            TEST_CHECK(true);

            std::shared_ptr<FakeRepository> r2(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &e,
                            n::name() = RepositoryName("repo2"))));
            r2->add_version(CategoryNamePart("cat-three") + PackageNamePart("pkg-three"), VersionSpec("0", VersionSpecOptions()));
            r2->add_version(CategoryNamePart("cat-three") + PackageNamePart("pkg-four"), VersionSpec("0", VersionSpecOptions()));
            p.add_repository(10, r2);
            TEST_CHECK(true);

            std::shared_ptr<FakeRepository> r3(new CoolFakeRepository(&e, RepositoryName("repo3")));
            r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-important"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-important"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-installed"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail1"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail1"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail2"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail2"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail3"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail3"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"), VersionSpec("0", VersionSpecOptions()));
            r3->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"), VersionSpec("0", VersionSpecOptions()));

            r3->add_version(CategoryNamePart("avail-cat") + PackageNamePart("pkg-foo"), VersionSpec("0", VersionSpecOptions()));
            p.add_repository(10, r3);
            TEST_CHECK(true);

            std::shared_ptr<FakeInstalledRepository> r4(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            n::environment() = &e,
                            n::name() = RepositoryName("repo4"),
                            n::suitable_destination() = true,
                            n::supports_uninstall() = true
                            )));
            r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"), VersionSpec("0", VersionSpecOptions()));
            r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"), VersionSpec("0", VersionSpecOptions()));
            r4->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"), VersionSpec("0", VersionSpecOptions()));
            r4->add_version(CategoryNamePart("inst-cat") + PackageNamePart("pkg-foo"), VersionSpec("0", VersionSpecOptions()));
            p.add_repository(10, r4);

            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-one")),
                    "cat-one/pkg-one");
            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-four")),
                    "cat-three/pkg-four");

            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-important")),
                    "good-cat1/pkg-important");

            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-installed")),
                    "good-cat1/pkg-installed");

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-two")),
                    AmbiguousPackageNameError);
            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-three")),
                    AmbiguousPackageNameError);

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail1")),
                    AmbiguousPackageNameError);
            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail2")),
                    AmbiguousPackageNameError);
            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail3")),
                    AmbiguousPackageNameError);
            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-fail4")),
                    AmbiguousPackageNameError);

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-five")),
                    NoSuchPackageError);

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-one"),
                        filter::SupportsAction<ConfigAction>()),
                    NoSuchPackageError);
            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-foo")),
                    "inst-cat/pkg-foo");
            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-foo"),
                        filter::SupportsAction<InstallAction>()),
                    "avail-cat/pkg-foo");

        }
    } package_database_disambiguate_test;
}

