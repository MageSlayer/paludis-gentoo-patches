/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/source_uri_finder.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/package_database.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;
using namespace paludis::erepository;

namespace test_cases
{
    struct SourceURIFinderTest : TestCase
    {
        SourceURIFinderTest() : TestCase("source uri finder") { }

        void run()
        {
            TestEnvironment env;
            const std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("repo"))
                            )));
            env.package_database()->add_repository(1, repo);

            SourceURIFinder f(&env, repo.get(), "http://example.com/path/input", "output", "monkey");
            URIMirrorsThenListedLabel label("mirrors-then-listed");
            label.accept(f);

            SourceURIFinder::ConstIterator i(f.begin());

            TEST_CHECK(i != f.end());
            TEST_CHECK_EQUAL(i->first, "http://example.com/path/input");
            TEST_CHECK_EQUAL(i->second, "output");

            ++i;

            TEST_CHECK(i == f.end());
        }
    } test_source_uri_finder;

    struct SourceURIMirrorFinderTest : TestCase
    {
        SourceURIMirrorFinderTest() : TestCase("source uri mirror finder") { }

        void run()
        {
            TestEnvironment env;
            const std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("repo"))
                            )));
            env.package_database()->add_repository(1, repo);

            SourceURIFinder f(&env, repo.get(), "mirror://example/path/input", "output", "repo");
            URIMirrorsThenListedLabel label("mirrors-then-listed");
            label.accept(f);

            SourceURIFinder::ConstIterator i(f.begin());

            TEST_CHECK(i != f.end());
            TEST_CHECK_EQUAL(i->first, "http://fake-repo/fake-repo/output");
            TEST_CHECK_EQUAL(i->second, "output");

            ++i;

            TEST_CHECK(i != f.end());
            TEST_CHECK_EQUAL(i->first, "http://example-mirror-1/example-mirror-1/path/input");
            TEST_CHECK_EQUAL(i->second, "output");

            ++i;

            TEST_CHECK(i != f.end());
            TEST_CHECK_EQUAL(i->first, "http://example-mirror-2/example-mirror-2/path/input");
            TEST_CHECK_EQUAL(i->second, "output");

            ++i;

            TEST_CHECK(i != f.end());
            TEST_CHECK_EQUAL(i->first, "http://fake-example/fake-example/path/input");
            TEST_CHECK_EQUAL(i->second, "output");

            ++i;

            TEST_CHECK(i == f.end());
        }
    } test_source_uri_mirror_finder;
}

