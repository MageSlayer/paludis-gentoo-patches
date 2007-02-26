/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
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
    /**
     * \test PackageDatabase repository tests.
     *
     */
    struct PackageDatabaseRepositoryTest : TestCase
    {
        PackageDatabaseRepositoryTest() : TestCase("package database repository") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            std::tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            std::tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));

            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo1")), NoSuchRepositoryError);
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

            p.add_repository(r1);
            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

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

            TEST_CHECK(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo2")));
            TEST_CHECK(p.more_important_than(RepositoryName("repo2"), RepositoryName("repo1")));
            TEST_CHECK(! p.more_important_than(RepositoryName("repo2"), RepositoryName("repo2")));
            TEST_CHECK(! p.more_important_than(RepositoryName("repo1"), RepositoryName("repo1")));
        }
    } package_database_repository_test;

    /**
     * \test PackageDatabase query tests.
     *
     */
    struct PackageDatabaseQueryTest : TestCase
    {
        PackageDatabaseQueryTest() : TestCase("package database query") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            std::tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_version("r1c1", "r1c1p1", "1");
            r1->add_version("r1c1", "r1c1p2", "1");
            r1->add_version("r1c1", "r1c1p2", "2");
            r1->add_version("rac1", "rac1pa", "1");
            r1->add_version("rac1", "rac1pa", "2");
            p.add_repository(r1);
            TEST_CHECK(true);

            std::tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
            r2->add_version("rac1", "rac1pa", "1");
            r2->add_version("rac1", "rac1pa", "3");
            p.add_repository(r2);
            TEST_CHECK(true);

            PackageDepSpec d1("r1c1/r1c1p1");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q1(p.query(
                        query::Matches(d1), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q1->begin(), q1->end()), 1);

            PackageDepSpec d2("r1c1/r1c1p2");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q2(p.query(
                        query::Matches(d2), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q2->begin(), q2->end()), 2);

            PackageDepSpec d3(">=r1c1/r1c1p2-1");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q3(p.query(
                        query::Matches(d3), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q3->begin(), q3->end()), 2);

            PackageDepSpec d4(">=r1c1/r1c1p2-2");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q4(p.query(
                        query::Matches(d4), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q4->begin(), q4->end()), 1);

            PackageDepSpec d5(">=r1c1/r1c1p2-3");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q5(p.query(
                        query::Matches(d5), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q5->begin(), q5->end()), 0);

            PackageDepSpec d6("<r1c1/r1c1p2-3");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q6(p.query(
                        query::Matches(d6), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q6->begin(), q6->end()), 2);

            PackageDepSpec d7("rac1/rac1pa");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q7(p.query(
                        query::Matches(d7), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q7->begin(), q7->end()), 4);

            PackageDepSpec d8("foo/bar");
            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q8(p.query(
                        query::Matches(d8), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q8->begin(), q8->end()), 0);
        }
    } package_database_query_test;

    struct PackageDatabaseQueryOrderTest : TestCase
    {
        PackageDatabaseQueryOrderTest() : TestCase("package database query order") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            std::tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_version("cat", "pkg", "1")->slot = SlotName("a");
            r1->add_version("cat", "pkg", "2")->slot = SlotName("c");
            r1->add_version("cat", "pkg", "3")->slot = SlotName("c");
            r1->add_version("cat", "pkg", "4")->slot = SlotName("a");
            p.add_repository(r1);
            TEST_CHECK(true);

            std::tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
            r2->add_version("cat", "pkg", "1")->slot = SlotName("a");
            r2->add_version("cat", "pkg", "3")->slot = SlotName("b");
            p.add_repository(r2);
            TEST_CHECK(true);

            PackageDepSpec d("cat/pkg");

            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q1(p.query(d, is_any, qo_order_by_version));
            TEST_CHECK_EQUAL(join(q1->begin(), q1->end(), " "),
                    "cat/pkg-1::repo1 cat/pkg-1::repo2 cat/pkg-2::repo1 cat/pkg-3::repo1 cat/pkg-3::repo2 cat/pkg-4::repo1");

            const std::tr1::shared_ptr<PackageDatabaseEntryCollection> q2(p.query(d, is_any, qo_group_by_slot));
            TEST_CHECK_EQUAL(join(q2->begin(), q2->end(), " "),
                    "cat/pkg-2::repo1 cat/pkg-3::repo1 cat/pkg-3::repo2 cat/pkg-1::repo1 cat/pkg-1::repo2 cat/pkg-4::repo1");
        }
    } package_database_query_order_test;

    /**
     * \test PackageDatabase disambiguate tests.
     *
     */
    struct PackageDatabaseDisambiguateTest : TestCase
    {
        PackageDatabaseDisambiguateTest() : TestCase("package database disambiguate") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            std::tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-three"));
            p.add_repository(r1);
            TEST_CHECK(true);

            std::tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
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
