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

#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/user_dep_spec.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    struct TestInfo
    {
        std::shared_ptr<Filter> filter;
        std::string expected;
    };

    struct FilterTestCaseBase :
        testing::TestWithParam<TestInfo>
    {
        TestEnvironment env;
        TestInfo info;

        void SetUp() override
        {
            info = GetParam();
        }
    };
}

TEST_P(FilterTestCaseBase, Works)
{
    auto repo1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo1")
                    )));
    auto repo2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo2")
                    )));
    auto inst_repo1(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("inst_repo1"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));

    env.add_repository(1, repo1);
    env.add_repository(10, repo2);
    env.add_repository(0, inst_repo1);

    repo1->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", { }));
    repo1->add_version(CategoryNamePart("cat") + PackageNamePart("b"), VersionSpec("2", { }));

    repo2->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", { }));
    repo2->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("2", { }))->keywords_key()->set_from_string("");
    repo2->add_version(CategoryNamePart("cat") + PackageNamePart("c"), VersionSpec("3", { }));

    inst_repo1->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", { }));

    std::shared_ptr<const PackageIDSequence> got(env[selection::AllVersionsSorted(generator::All() | *info.filter)]);

    ASSERT_TRUE(bool(got));
    EXPECT_EQ(info.expected, join(indirect_iterator(got->begin()), indirect_iterator(got->end()), ", "));

    std::shared_ptr<const PackageIDSequence> got_none(env[selection::AllVersionsSorted(
                generator::Matches(parse_user_package_dep_spec("not/exist", &env,
                        { }), nullptr, { }) | *info.filter)]);
    ASSERT_TRUE(bool(got_none));
    EXPECT_EQ("", join(indirect_iterator(got_none->begin()), indirect_iterator(got_none->end()), ", "));
}

INSTANTIATE_TEST_CASE_P(FilterTest, FilterTestCaseBase, testing::Values(
            TestInfo{ std::make_shared<filter::All>(), std::string(
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2") },

            TestInfo{ std::make_shared<filter::SupportsAction<InstallAction> >(), std::string(
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2") },

            TestInfo{ std::make_shared<filter::InstalledAtRoot>(FSPath("/")), std::string(
                "cat/a-1:0::inst_repo1") },

            TestInfo{ std::make_shared<filter::InstalledAtRoot>(FSPath("/blah")), std::string(
                "") },

            TestInfo{ std::make_shared<filter::NotMasked>(), std::string(
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2") },

            TestInfo{ std::make_shared<filter::And>(filter::SupportsAction<InstallAction>(), filter::NotMasked()), std::string(
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2") },

            TestInfo{ std::make_shared<filter::And>(filter::NotMasked(), filter::SupportsAction<InstallAction>()), std::string(
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2") },

            TestInfo{ std::make_shared<filter::Matches>(envless_parse_package_dep_spec_for_tests(
                        "cat/a"), nullptr, MatchPackageOptions()), std::string(
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2") },

            TestInfo{ std::make_shared<filter::Matches>(envless_parse_package_dep_spec_for_tests(
                        "*/a"), nullptr, MatchPackageOptions()), std::string(
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2") },

            TestInfo{ std::make_shared<filter::Matches>(envless_parse_package_dep_spec_for_tests(
                        "cat/*"), nullptr, MatchPackageOptions()), std::string(
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2" ) },

            TestInfo{ std::make_shared<filter::Matches>(envless_parse_package_dep_spec_for_tests(
                        ">=*/*-2"), nullptr, MatchPackageOptions()), std::string(
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2" ) }
            ));

