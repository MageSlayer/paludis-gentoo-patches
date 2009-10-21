/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include <paludis/fuzzy_finder.hh>
#include <paludis/filter.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/package_database.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct FuzzyCandidatesFinderTest : TestCase
    {
        FuzzyCandidatesFinderTest() : TestCase("fuzzy candidates finder") { }

        void run()
        {
            TestEnvironment e;

            const std::tr1::shared_ptr<FakeRepository> r1(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&e),
                            value_for<n::name>(RepositoryName("r1"))
                            )));
            r1->add_version("some-cat", "foo", "1");
            r1->add_version("other-cat", "foo", "1");
            r1->add_version("some-cat", "bar", "1");
            r1->add_version("some-cat", "one-two-three", "1");
            e.package_database()->add_repository(1, r1);

            const std::tr1::shared_ptr<FakeRepository> r2(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&e),
                            value_for<n::name>(RepositoryName("r2"))
                            )));
            e.package_database()->add_repository(2, r2);

            FuzzyCandidatesFinder f1(e, std::string("some-cat/one-two-thee"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f1.begin(), f1.end()), 1);

            FuzzyCandidatesFinder f2(e, std::string("fio"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f2.begin(), f2.end()), 2);

            FuzzyCandidatesFinder f3(e, std::string("bra"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f3.begin(), f3.end()), 1);

            FuzzyCandidatesFinder f4(e, std::string("foobarandfriends"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f4.begin(), f4.end()), 0);

            FuzzyCandidatesFinder f5(e, std::string("some-cat/foo::r2"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f5.begin(), f5.end()), 0);

            FuzzyCandidatesFinder f6(e, std::string("some-cat/OnE-tWo-THEE"), filter::All());
            TEST_CHECK_EQUAL(std::distance(f6.begin(), f6.end()), 1);

        }
    } fuzzy_candidates_finder_test;

    struct FuzzyRepositoriesFinderTest : TestCase
    {
        FuzzyRepositoriesFinderTest() : TestCase("fuzzy repositories finder") { }

        void run()
        {
            TestEnvironment e;
            PackageDatabase & p(*e.package_database());

            p.add_repository(1, std::tr1::shared_ptr<FakeRepository>(new FakeRepository(make_named_values<FakeRepositoryParams>(
                                value_for<n::environment>(&e),
                                value_for<n::name>(RepositoryName("my-main-repository"))))));
            p.add_repository(1, std::tr1::shared_ptr<FakeRepository>(new FakeRepository(make_named_values<FakeRepositoryParams>(
                                value_for<n::environment>(&e),
                                value_for<n::name>(RepositoryName("x-new-repository"))))));
            p.add_repository(1, std::tr1::shared_ptr<FakeRepository>(new FakeRepository(make_named_values<FakeRepositoryParams>(
                                value_for<n::environment>(&e),
                                value_for<n::name>(RepositoryName("bar-overlay"))))));
            p.add_repository(1, std::tr1::shared_ptr<FakeRepository>(new FakeRepository(make_named_values<FakeRepositoryParams>(
                                value_for<n::environment>(&e),
                                value_for<n::name>(RepositoryName("baz-overlay"))))));
            p.add_repository(1, std::tr1::shared_ptr<FakeRepository>(new FakeRepository(make_named_values<FakeRepositoryParams>(
                                value_for<n::environment>(&e),
                                value_for<n::name>(RepositoryName("sunrise"))))));

            FuzzyRepositoriesFinder f1(e, "my-main-respository");
            TEST_CHECK_EQUAL(std::distance(f1.begin(), f1.end()), 1);

            FuzzyRepositoriesFinder f2(e, "new-repository");
            TEST_CHECK_EQUAL(std::distance(f2.begin(), f2.end()), 1);
            TEST_CHECK_EQUAL(stringify(*f2.begin()), "x-new-repository");

            FuzzyRepositoriesFinder f3(e, "sunric3");
            TEST_CHECK_EQUAL(std::distance(f3.begin(), f3.end()), 1);

            FuzzyRepositoriesFinder f4(e, "bar-overlay");
            TEST_CHECK_EQUAL(std::distance(f4.begin(), f4.end()), 2);

            FuzzyRepositoriesFinder f5(e, "foo");
            TEST_CHECK_EQUAL(std::distance(f5.begin(), f5.end()), 0);

            FuzzyRepositoriesFinder f6(e, "new-repositori");
            TEST_CHECK_EQUAL(std::distance(f6.begin(), f6.end()), 1);
        }
    } fuzzy_repositories_finder_test;
}
