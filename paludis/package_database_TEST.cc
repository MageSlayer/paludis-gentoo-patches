/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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
#include <paludis/query.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/indirect_iterator-impl.hh>
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

            tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));

            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo1")), NoSuchRepositoryError);
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

            p.add_repository(10, r1);
            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK_THROWS(p.fetch_repository(RepositoryName("repo2")), NoSuchRepositoryError);

            TEST_CHECK_THROWS(p.add_repository(10, r1), DuplicateRepositoryError);

            p.add_repository(11, r2);
            TEST_CHECK(p.fetch_repository(RepositoryName("repo1")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo1"))->name(),
                    RepositoryName("repo1"));
            TEST_CHECK(p.fetch_repository(RepositoryName("repo2")));
            TEST_CHECK_EQUAL(p.fetch_repository(RepositoryName("repo2"))->name(),
                    RepositoryName("repo2"));

            TEST_CHECK_THROWS(p.add_repository(10, r1), DuplicateRepositoryError);
            TEST_CHECK_THROWS(p.add_repository(5, r2), DuplicateRepositoryError);

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

            tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_version("r1c1", "r1c1p1", "1");
            r1->add_version("r1c1", "r1c1p2", "1");
            r1->add_version("r1c1", "r1c1p2", "2");
            r1->add_version("rac1", "rac1pa", "1");
            r1->add_version("rac1", "rac1pa", "2");
            p.add_repository(11, r1);
            TEST_CHECK(true);

            tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
            r2->add_version("rac1", "rac1pa", "1");
            r2->add_version("rac1", "rac1pa", "3");
            p.add_repository(10, r2);
            TEST_CHECK(true);

            PackageDepSpec d1("r1c1/r1c1p1", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q1(p.query(
                        query::Matches(d1), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q1->begin(), q1->end()), 1);

            PackageDepSpec d2("r1c1/r1c1p2", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q2(p.query(
                        query::Matches(d2), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q2->begin(), q2->end()), 2);

            PackageDepSpec d3(">=r1c1/r1c1p2-1", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q3(p.query(
                        query::Matches(d3), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q3->begin(), q3->end()), 2);

            PackageDepSpec d4(">=r1c1/r1c1p2-2", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q4(p.query(
                        query::Matches(d4), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(q4->begin()), indirect_iterator(q4->end()), " "),
                    "r1c1/r1c1p2-2:0::repo1");
            TEST_CHECK_EQUAL(std::distance(q4->begin(), q4->end()), 1);

            PackageDepSpec d5(">=r1c1/r1c1p2-3", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q5(p.query(
                        query::Matches(d5), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q5->begin(), q5->end()), 0);

            PackageDepSpec d6("<r1c1/r1c1p2-3", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q6(p.query(
                        query::Matches(d6), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q6->begin(), q6->end()), 2);

            PackageDepSpec d7("rac1/rac1pa", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q7(p.query(
                        query::Matches(d7), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q7->begin(), q7->end()), 4);

            PackageDepSpec d8("foo/bar", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q8(p.query(
                        query::Matches(d8), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q8->begin(), q8->end()), 0);

            PackageDepSpec d9("r1c1/r1c1p1", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q9(p.query(
                        query::Matches(d9) & query::SupportsAction<InstallAction>(), qo_order_by_version));
            TEST_CHECK_EQUAL(std::distance(q9->begin(), q9->end()), 1);
        }
    } package_database_query_test;

    struct PackageDatabaseQueryOrderTest : TestCase
    {
        PackageDatabaseQueryOrderTest() : TestCase("package database query order") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_version("cat", "pkg", "1")->set_slot(SlotName("a"));
            r1->add_version("cat", "pkg", "2")->set_slot(SlotName("c"));
            r1->add_version("cat", "pkg", "3")->set_slot(SlotName("c"));
            r1->add_version("cat", "pkg", "4")->set_slot(SlotName("a"));
            p.add_repository(10, r1);
            TEST_CHECK(true);

            tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
            r2->add_version("cat", "pkg", "1")->set_slot(SlotName("a"));
            r2->add_version("cat", "pkg", "3")->set_slot(SlotName("b"));
            p.add_repository(5, r2);
            TEST_CHECK(true);

            PackageDepSpec d("cat/pkg", pds_pm_permissive);

            const tr1::shared_ptr<const PackageIDSequence> q1(p.query(query::Matches(d), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(q1->begin()), indirect_iterator(q1->end()), " "),
                    "cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-2:c::repo1 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q2(p.query(query::Matches(d), qo_group_by_slot));
            TEST_CHECK_EQUAL(join(indirect_iterator(q2->begin()), indirect_iterator(q2->end()), " "),
                    "cat/pkg-3:b::repo2 cat/pkg-2:c::repo1 cat/pkg-3:c::repo1 cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q3(p.query(query::Matches(d), qo_best_version_only));
            TEST_CHECK_EQUAL(join(indirect_iterator(q3->begin()), indirect_iterator(q3->end()), " "),
                    "cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q4(p.query(query::Matches(d), qo_best_version_in_slot_only));
            TEST_CHECK_EQUAL(join(indirect_iterator(q4->begin()), indirect_iterator(q4->end()), " "),
                    "cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1");

            tr1::shared_ptr<FakeRepository> r3(new FakeRepository(&e, RepositoryName("repo3")));
            r3->add_version("cat", "other", "1")->set_slot(SlotName("a"));
            p.add_repository(5, r3);
            TEST_CHECK(true);

            PackageDepSpec c("cat/*", pds_pm_unspecific);

            const tr1::shared_ptr<const PackageIDSequence> q5(p.query(query::Matches(c), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(q5->begin()), indirect_iterator(q5->end()), " "),
                    "cat/other-1:a::repo3 cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-2:c::repo1 "
                    "cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q6(p.query(query::Matches(c), qo_group_by_slot));
            TEST_CHECK_EQUAL(join(indirect_iterator(q6->begin()), indirect_iterator(q6->end()), " "),
                    "cat/other-1:a::repo3 cat/pkg-3:b::repo2 cat/pkg-2:c::repo1 cat/pkg-3:c::repo1 "
                    "cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q7(p.query(query::Matches(c), qo_best_version_only));
            TEST_CHECK_EQUAL(join(indirect_iterator(q7->begin()), indirect_iterator(q7->end()), " "),
                    "cat/other-1:a::repo3 cat/pkg-4:a::repo1");

            const tr1::shared_ptr<const PackageIDSequence> q8(p.query(query::Matches(c), qo_best_version_in_slot_only));
            TEST_CHECK_EQUAL(join(indirect_iterator(q8->begin()), indirect_iterator(q8->end()), " "),
                    "cat/other-1:a::repo3 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1");

            PackageDepSpec b("cat/pkg:a", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q9(p.query(query::Matches(b), qo_group_by_slot));
            TEST_CHECK_EQUAL(join(indirect_iterator(q9->begin()), indirect_iterator(q9->end()), " "),
                    "cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1");

            PackageDepSpec a("cat/pkg[=1|=3]", pds_pm_permissive);
            const tr1::shared_ptr<const PackageIDSequence> q10(p.query(query::Matches(a), qo_group_by_slot));
            TEST_CHECK_EQUAL(join(indirect_iterator(q10->begin()), indirect_iterator(q10->end()), " "),
                    "cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1");
        }
    } package_database_query_order_test;

    /**
     * \test PackageDatabase disambiguate tests.
     *
     */
    struct PackageDatabaseDisambiguateTest : TestCase
    {
        PackageDatabaseDisambiguateTest() : TestCase("package database disambiguate") { }

        struct CoolFakeRepository :
            FakeRepository
        {
            CoolFakeRepository(const Environment * const e, const RepositoryName & rn) :
                FakeRepository(e, rn)
            {
            }

            tr1::shared_ptr<const CategoryNamePartSet> unimportant_category_names() const
            {
                tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
                result->insert(CategoryNamePart("bad-cat1"));
                result->insert(CategoryNamePart("bad-cat2"));
                return result;
            }
        };

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            tr1::shared_ptr<FakeRepository> r1(new FakeRepository(&e, RepositoryName("repo1")));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-one"));
            r1->add_package(CategoryNamePart("cat-one") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-two"));
            r1->add_package(CategoryNamePart("cat-two") + PackageNamePart("pkg-three"));
            p.add_repository(10, r1);
            TEST_CHECK(true);

            tr1::shared_ptr<FakeRepository> r2(new FakeRepository(&e, RepositoryName("repo2")));
            r2->add_package(CategoryNamePart("cat-three") + PackageNamePart("pkg-three"));
            r2->add_package(CategoryNamePart("cat-three") + PackageNamePart("pkg-four"));
            p.add_repository(10, r2);
            TEST_CHECK(true);

            tr1::shared_ptr<FakeRepository> r3(new CoolFakeRepository(&e, RepositoryName("repo3")));
            r3->add_package(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-important"));
            r3->add_package(CategoryNamePart("good-cat1") + PackageNamePart("pkg-important"));

            r3->add_package(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"));
            r3->add_package(CategoryNamePart("good-cat2") + PackageNamePart("pkg-installed"));

            r3->add_package(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail1"));
            r3->add_package(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail1"));

            r3->add_package(CategoryNamePart("bad-cat1") + PackageNamePart("pkg-fail2"));
            r3->add_package(CategoryNamePart("bad-cat2") + PackageNamePart("pkg-fail2"));

            r3->add_package(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail3"));
            r3->add_package(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail3"));

            r3->add_package(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"));
            r3->add_package(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"));
            p.add_repository(10, r3);
            TEST_CHECK(true);

            tr1::shared_ptr<FakeInstalledRepository> r4(new FakeInstalledRepository(&e, RepositoryName("repo4")));
            r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-installed"), VersionSpec("0"));
            r4->add_version(CategoryNamePart("good-cat1") + PackageNamePart("pkg-fail4"), VersionSpec("0"));
            r4->add_version(CategoryNamePart("good-cat2") + PackageNamePart("pkg-fail4"), VersionSpec("0"));
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

        }
    } package_database_disambiguate_test;
}

