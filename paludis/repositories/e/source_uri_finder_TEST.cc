/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/eapi.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    const std::shared_ptr<MirrorsSequence>
    get_mirrors_fn(const std::string & m)
    {
        const std::shared_ptr<MirrorsSequence> result(std::make_shared<MirrorsSequence>());
        if (m == "example")
            result->push_back("http://fake-example/fake-example/");
        if (m == "repo")
            result->push_back("http://fake-repo/fake-repo/");
        return result;
    }
}

TEST(SourceURIFinder, Works)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"));

    SourceURIFinder f(&env, repo.get(), *eapi, "output", "monkey",
            get_mirrors_fn, "http://example.com/path/input");
    URIMirrorsThenListedLabel label("mirrors-then-listed");
    label.accept(f);

    SourceURIFinder::ConstIterator i(f.begin());

    ASSERT_TRUE(i != f.end());
    EXPECT_EQ("http://example.com/path/input", i->first);
    EXPECT_EQ("output", i->second);

    ++i;

    ASSERT_TRUE(i == f.end());
}

TEST(SourceURIFinder, Mirrors)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    const std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"));

    SourceURIFinder f(&env, repo.get(), *eapi, "output", "repo", get_mirrors_fn, "mirror://example/path/input");
    URIMirrorsThenListedLabel label("mirrors-then-listed");
    label.accept(f);

    SourceURIFinder::ConstIterator i(f.begin());

    ASSERT_TRUE(i != f.end());
    EXPECT_EQ("http://fake-repo/fake-repo/output", i->first);
    EXPECT_EQ("output", i->second);

    ++i;

    ASSERT_TRUE(i != f.end());
    EXPECT_EQ("http://example-mirror-1/example-mirror-1/path/input", i->first);
    EXPECT_EQ("output", i->second);

    ++i;

    ASSERT_TRUE(i != f.end());
    EXPECT_EQ("http://example-mirror-2/example-mirror-2/path/input", i->first);
    EXPECT_EQ("output", i->second);

    ++i;

    ASSERT_TRUE(i != f.end());
    EXPECT_EQ("http://fake-example/fake-example/path/input", i->first);
    EXPECT_EQ("output", i->second);

    ++i;

    ASSERT_TRUE(i == f.end());
}

