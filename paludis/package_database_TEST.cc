/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/paludis.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for package_database.hh.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test PackageDatabase repository tests.
     *
     * \ingroup grptestcases
     */
    struct PackageDatabaseRepositoryTest : TestCase
    {
        PackageDatabaseRepositoryTest() : TestCase("package database repository") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            FakeRepository::Pointer r1(new FakeRepository(RepositoryName("repo1")));
            FakeRepository::Pointer r2(new FakeRepository(RepositoryName("repo2")));

            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo1")), PackageDatabaseLookupError);
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), PackageDatabaseLookupError);

            p.add_repository(r1);
            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), PackageDatabaseLookupError);

            TEST_CHECK_THROWS(p.add_repository(r1), DuplicateRepositoryError);

            p.add_repository(r2);
            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK(p.fetch_repository(RepositoryName("repo2")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo2"))->name(),
                    RepositoryName("repo2"));

            TEST_CHECK_THROWS(p.add_repository(r1), DuplicateRepositoryError);
            TEST_CHECK_THROWS(p.add_repository(r2), DuplicateRepositoryError);

            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK(p.fetch_repository(RepositoryName("repo2")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo2"))->name(),
                    RepositoryName("repo2"));

            TEST_CHECK_EQUAL(p.better_repository(RepositoryName("repo1"), RepositoryName("repo2")),
                    RepositoryName("repo2"));
            TEST_CHECK_EQUAL(p.better_repository(RepositoryName("repo2"), RepositoryName("repo1")),
                    RepositoryName("repo2"));
            TEST_CHECK_EQUAL(p.better_repository(RepositoryName("repo2"), RepositoryName("repo2")),
                    RepositoryName("repo2"));
            TEST_CHECK_EQUAL(p.better_repository(RepositoryName("repo1"), RepositoryName("repo1")),
                    RepositoryName("repo1"));
        }
    } package_database_repository_test;

    /**
     * \test PackageDatabase query tests.
     *
     * \ingroup grptestcases
     */
    struct PackageDatabaseQueryTest : TestCase
    {
        PackageDatabaseQueryTest() : TestCase("package database query") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            FakeRepository::Pointer r1(new FakeRepository(RepositoryName("repo1")));
            r1->add_version("r1c1", "r1c1p1", "1");
            r1->add_version("r1c1", "r1c1p2", "1");
            r1->add_version("r1c1", "r1c1p2", "2");
            r1->add_version("rac1", "rac1pa", "1");
            r1->add_version("rac1", "rac1pa", "2");
            p.add_repository(r1);
            TEST_CHECK(true);

            FakeRepository::Pointer r2(new FakeRepository(RepositoryName("repo2")));
            r2->add_version("rac1", "rac1pa", "1");
            r2->add_version("rac1", "rac1pa", "3");
            p.add_repository(r2);
            TEST_CHECK(true);

            PackageDepAtom d1("r1c1/r1c1p1");
            const PackageDatabaseEntryCollection::Pointer q1(p.query(d1, is_either));
            TEST_CHECK_EQUAL(q1->size(), 1);

            PackageDepAtom d2("r1c1/r1c1p2");
            const PackageDatabaseEntryCollection::Pointer q2(p.query(d2, is_either));
            TEST_CHECK_EQUAL(q2->size(), 2);

            PackageDepAtom d3(">=r1c1/r1c1p2-1");
            const PackageDatabaseEntryCollection::Pointer q3(p.query(d3, is_either));
            TEST_CHECK_EQUAL(q3->size(), 2);

            PackageDepAtom d4(">=r1c1/r1c1p2-2");
            const PackageDatabaseEntryCollection::Pointer q4(p.query(d4, is_either));
            TEST_CHECK_EQUAL(q4->size(), 1);

            PackageDepAtom d5(">=r1c1/r1c1p2-3");
            const PackageDatabaseEntryCollection::Pointer q5(p.query(d5, is_either));
            TEST_CHECK_EQUAL(q5->size(), 0);

            PackageDepAtom d6("<r1c1/r1c1p2-3");
            const PackageDatabaseEntryCollection::Pointer q6(p.query(d6, is_either));
            TEST_CHECK_EQUAL(q6->size(), 2);

            PackageDepAtom d7("rac1/rac1pa");
            const PackageDatabaseEntryCollection::Pointer q7(p.query(d7, is_either));
            TEST_CHECK_EQUAL(q7->size(), 4);

            PackageDepAtom d8("foo/bar");
            const PackageDatabaseEntryCollection::Pointer q8(p.query(d8, is_either));
            TEST_CHECK_EQUAL(q8->size(), 0);
        }
    } package_database_query_test;

    /**
     * \test PackageDatabase disambiguate tests.
     *
     * \ingroup grptestcases
     */
    struct PackageDatabaseDisambiguateTest : TestCase
    {
        PackageDatabaseDisambiguateTest() : TestCase("package database disambiguate") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            FakeRepository::Pointer r1(new FakeRepository(RepositoryName("repo1")));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-three"));
            p.add_repository(r1);
            TEST_CHECK(true);

            FakeRepository::Pointer r2(new FakeRepository(RepositoryName("repo2")));
            r1->add_package(CategoryNamePart("cat-three") + PackageNamePart("pkg-three"));
            r1->add_package(CategoryNamePart("cat-three") + PackageNamePart("pkg-four"));
            p.add_repository(r2);
            TEST_CHECK(true);

            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-one")),
                    "cat-one/pkg-one");
            TEST_CHECK_STRINGIFY_EQUAL(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-four")),
                    "cat-three/pkg-four");

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-two")),
                    AmbiguousPackageNameError);
            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-three")),
                    AmbiguousPackageNameError);

            TEST_CHECK_THROWS(p.fetch_unique_qualified_package_name(PackageNamePart("pkg-five")),
                    NoSuchPackageError);
        }
    } package_database_disambiguate_test;
}
