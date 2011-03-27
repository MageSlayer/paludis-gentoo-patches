/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/user_dep_spec.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>

#include <gtest/gtest.h>

using namespace paludis;

TEST(Selection, Basic)
{
    TestEnvironment env;

    std::shared_ptr<FakeRepository> r1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo1"))));
    r1->add_version("r1c1", "r1c1p1", "1");
    r1->add_version("r1c1", "r1c1p2", "1");
    r1->add_version("r1c1", "r1c1p2", "2");
    r1->add_version("rac1", "rac1pa", "1");
    r1->add_version("rac1", "rac1pa", "2");
    env.add_repository(11, r1);

    std::shared_ptr<FakeRepository> r2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo2"))));
    r2->add_version("rac1", "rac1pa", "1");
    r2->add_version("rac1", "rac1pa", "3");
    env.add_repository(10, r2);

    PackageDepSpec d1(parse_user_package_dep_spec("r1c1/r1c1p1", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q1(env[selection::AllVersionsSorted(generator::Matches(d1, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(1, std::distance(q1->begin(), q1->end()));

    PackageDepSpec d2(parse_user_package_dep_spec("r1c1/r1c1p2", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q2(env[selection::AllVersionsSorted(generator::Matches(d2, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(2, std::distance(q2->begin(), q2->end()));

    PackageDepSpec d3(parse_user_package_dep_spec(">=r1c1/r1c1p2-1", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q3(env[selection::AllVersionsSorted(generator::Matches(d3, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(2, std::distance(q3->begin(), q3->end()));

    PackageDepSpec d4(parse_user_package_dep_spec(">=r1c1/r1c1p2-2", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q4(env[selection::AllVersionsSorted(generator::Matches(d4, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("r1c1/r1c1p2-2:0::repo1", join(indirect_iterator(q4->begin()), indirect_iterator(q4->end()), " "));
    EXPECT_EQ(1, std::distance(q4->begin(), q4->end()));

    PackageDepSpec d5(parse_user_package_dep_spec(">=r1c1/r1c1p2-3", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q5(env[selection::AllVersionsSorted(generator::Matches(d5, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(0, std::distance(q5->begin(), q5->end()));

    PackageDepSpec d6(parse_user_package_dep_spec("<r1c1/r1c1p2-3", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q6(env[selection::AllVersionsSorted(generator::Matches(d6, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(2, std::distance(q6->begin(), q6->end()));

    PackageDepSpec d7(parse_user_package_dep_spec("rac1/rac1pa", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q7(env[selection::AllVersionsSorted(generator::Matches(d7, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(4, std::distance(q7->begin(), q7->end()));

    PackageDepSpec d8(parse_user_package_dep_spec("foo/bar", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q8(env[selection::AllVersionsSorted(generator::Matches(d8, make_null_shared_ptr(), { }))]);
    EXPECT_EQ(0, std::distance(q8->begin(), q8->end()));

    PackageDepSpec d9(parse_user_package_dep_spec("r1c1/r1c1p1", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q9(env[selection::AllVersionsSorted(generator::Matches(d9, make_null_shared_ptr(), { })
                | filter::SupportsAction<InstallAction>())]);
    EXPECT_EQ(1, std::distance(q9->begin(), q9->end()));
}

TEST(Selection, Fiddly)
{
    TestEnvironment env;

    std::shared_ptr<FakeRepository> r1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo1"))));
    r1->add_version("cat", "pkg", "1")->set_slot(SlotName("a"));
    r1->add_version("cat", "pkg", "2")->set_slot(SlotName("c"));
    r1->add_version("cat", "pkg", "3")->set_slot(SlotName("c"));
    r1->add_version("cat", "pkg", "4")->set_slot(SlotName("a"));
    env.add_repository(10, r1);

    std::shared_ptr<FakeRepository> r2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo2"))));
    r2->add_version("cat", "pkg", "1")->set_slot(SlotName("a"));
    r2->add_version("cat", "pkg", "3")->set_slot(SlotName("b"));
    env.add_repository(5, r2);

    PackageDepSpec d(parse_user_package_dep_spec("cat/pkg", &env, { }));

    const std::shared_ptr<const PackageIDSequence> q1(env[selection::AllVersionsSorted(generator::Matches(d, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-2:c::repo1 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q1->begin()), indirect_iterator(q1->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q2(env[selection::AllVersionsGroupedBySlot(generator::Matches(d, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-3:b::repo2 cat/pkg-2:c::repo1 cat/pkg-3:c::repo1 cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q2->begin()), indirect_iterator(q2->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q3(env[selection::BestVersionOnly(generator::Matches(d, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-4:a::repo1", join(indirect_iterator(q3->begin()), indirect_iterator(q3->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q4(env[selection::BestVersionInEachSlot(generator::Matches(d, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q4->begin()), indirect_iterator(q4->end()), " "));

    std::shared_ptr<FakeRepository> r3(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo3"))));
    r3->add_version("cat", "other", "1")->set_slot(SlotName("a"));
    env.add_repository(5, r3);

    PackageDepSpec c(parse_user_package_dep_spec("cat/*", &env, { updso_allow_wildcards }));

    const std::shared_ptr<const PackageIDSequence> q5(env[selection::AllVersionsSorted(generator::Matches(c, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/other-1:a::repo3 cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-2:c::repo1 "
            "cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q5->begin()), indirect_iterator(q5->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q6(env[selection::AllVersionsGroupedBySlot(generator::Matches(c, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/other-1:a::repo3 cat/pkg-3:b::repo2 cat/pkg-2:c::repo1 cat/pkg-3:c::repo1 "
            "cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q6->begin()), indirect_iterator(q6->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q7(env[selection::BestVersionOnly(generator::Matches(c, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/other-1:a::repo3 cat/pkg-4:a::repo1",
            join(indirect_iterator(q7->begin()), indirect_iterator(q7->end()), " "));

    const std::shared_ptr<const PackageIDSequence> q8(env[selection::BestVersionInEachSlot(generator::Matches(c, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/other-1:a::repo3 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q8->begin()), indirect_iterator(q8->end()), " "));

    PackageDepSpec b(parse_user_package_dep_spec("cat/pkg:a", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q9(env[selection::AllVersionsGroupedBySlot(generator::Matches(b, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-4:a::repo1",
            join(indirect_iterator(q9->begin()), indirect_iterator(q9->end()), " "));

    PackageDepSpec a(parse_user_package_dep_spec("cat/pkg[=1|=3]", &env, { }));
    const std::shared_ptr<const PackageIDSequence> q10(env[selection::AllVersionsGroupedBySlot(generator::Matches(a, make_null_shared_ptr(), { }))]);
    EXPECT_EQ("cat/pkg-1:a::repo2 cat/pkg-1:a::repo1 cat/pkg-3:b::repo2 cat/pkg-3:c::repo1",
            join(indirect_iterator(q10->begin()), indirect_iterator(q10->end()), " "));
}

