/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
 * Copyright (c) 2011 Ciaran McCreesh
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
#include <paludis/util/stringify.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::shared_ptr<FakeRepository>
    make_fake_repo(const Environment & env, const RepositoryName & name)
    {
        return std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                n::environment() = &env, n::name() = RepositoryName(name)));
    };
}

TEST(FuzzyCandidatesFinder, Works)
{
    TestEnvironment e;

    const auto r1 = make_fake_repo(e, RepositoryName("r1"));
    r1->add_version("some-cat", "foo", "1");
    r1->add_version("other-cat", "foo", "1");
    r1->add_version("some-cat", "bar", "1");
    r1->add_version("some-cat", "one-two-three", "1");
    e.add_repository(1, r1);

    const auto r2 = make_fake_repo(e, RepositoryName("r2"));
    e.add_repository(2, r2);

    FuzzyCandidatesFinder f1(e, std::string("some-cat/one-two-thee"), filter::All());
    EXPECT_EQ(1, std::distance(f1.begin(), f1.end()));

    FuzzyCandidatesFinder f2(e, std::string("fio"), filter::All());
    EXPECT_EQ(2, std::distance(f2.begin(), f2.end()));

    FuzzyCandidatesFinder f3(e, std::string("bra"), filter::All());
    EXPECT_EQ(1, std::distance(f3.begin(), f3.end()));

    FuzzyCandidatesFinder f4(e, std::string("foobarandfriends"), filter::All());
    EXPECT_EQ(0, std::distance(f4.begin(), f4.end()));

    FuzzyCandidatesFinder f5(e, std::string("some-cat/foo::r2"), filter::All());
    EXPECT_EQ(0, std::distance(f5.begin(), f5.end()));

    FuzzyCandidatesFinder f6(e, std::string("some-cat/OnE-tWo-THEE"), filter::All());
    EXPECT_EQ(1, std::distance(f6.begin(), f6.end()));

}

TEST(FuzzyRepositoriesFinder, Works)
{
    TestEnvironment e;

    e.add_repository(1, make_fake_repo(e, RepositoryName("my-main-repository")));
    e.add_repository(1, make_fake_repo(e, RepositoryName("x-new-repository")));
    e.add_repository(1, make_fake_repo(e, RepositoryName("bar-overlay")));
    e.add_repository(1, make_fake_repo(e, RepositoryName("baz-overlay")));
    e.add_repository(1, make_fake_repo(e, RepositoryName("sunrise")));

    FuzzyRepositoriesFinder f1(e, "my-main-respository");
    EXPECT_EQ(1, std::distance(f1.begin(), f1.end()));

    FuzzyRepositoriesFinder f2(e, "new-repository");
    EXPECT_EQ(1, std::distance(f2.begin(), f2.end()));
    EXPECT_EQ("x-new-repository", stringify(*f2.begin()));

    FuzzyRepositoriesFinder f3(e, "sunric3");
    EXPECT_EQ(1, std::distance(f3.begin(), f3.end()));

    FuzzyRepositoriesFinder f4(e, "bar-overlay");
    EXPECT_EQ(2, std::distance(f4.begin(), f4.end()));

    FuzzyRepositoriesFinder f5(e, "foo");
    EXPECT_EQ(0, std::distance(f5.begin(), f5.end()));

    FuzzyRepositoriesFinder f6(e, "new-repositori");
    EXPECT_EQ(1, std::distance(f6.begin(), f6.end()));
}

