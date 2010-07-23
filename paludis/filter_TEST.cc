/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <test/test_concepts.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    TESTCASE_SEMIREGULAR(Filter, filter::All());
    TESTCASE_STRINGIFYABLE(Filter, filter::All(), "all matches");

    struct FilterTestCaseBase : TestCase
    {
        Filter filter;
        TestEnvironment env;
        std::shared_ptr<FakeRepository> repo1;
        std::shared_ptr<FakeRepository> repo2;
        std::shared_ptr<FakeInstalledRepository> inst_repo1;

        FilterTestCaseBase(const std::string & s, const Filter & f) :
            TestCase("filter " + s + " with " + stringify(f)),
            filter(f),
            repo1(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo1")
                            ))),
            repo2(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo2")
                            ))),
            inst_repo1(std::make_shared<FakeInstalledRepository>(
                        make_named_values<FakeInstalledRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("inst_repo1"),
                            n::suitable_destination() = true,
                            n::supports_uninstall() = true
                            )))
        {
            env.package_database()->add_repository(1, repo1);
            env.package_database()->add_repository(10, repo2);
            env.package_database()->add_repository(0, inst_repo1);

            repo1->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", VersionSpecOptions()));
            repo1->add_version(CategoryNamePart("cat") + PackageNamePart("b"), VersionSpec("2", VersionSpecOptions()));

            repo2->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", VersionSpecOptions()));
            repo2->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("2", VersionSpecOptions()))->keywords_key()->set_from_string("");
            repo2->add_version(CategoryNamePart("cat") + PackageNamePart("c"), VersionSpec("3", VersionSpecOptions()));

            inst_repo1->add_version(CategoryNamePart("cat") + PackageNamePart("a"), VersionSpec("1", VersionSpecOptions()));
        }

        virtual std::string get_expected() const = 0;

        void run()
        {
            const std::string expected(get_expected());
            std::shared_ptr<const PackageIDSequence> got(env[selection::AllVersionsSorted(generator::All() | filter)]);

            TEST_CHECK(bool(got));
            TEST_CHECK_EQUAL(join(indirect_iterator(got->begin()), indirect_iterator(got->end()), ", "), expected);

            std::shared_ptr<const PackageIDSequence> got_none(env[selection::AllVersionsSorted(
                        generator::Matches(parse_user_package_dep_spec("not/exist", &env,
                                UserPackageDepSpecOptions()), MatchPackageOptions()) | filter)]);
            TEST_CHECK(bool(got_none));
            TEST_CHECK_EQUAL(join(indirect_iterator(got_none->begin()), indirect_iterator(got_none->end()), ", "), "");
        }
    };

    struct AllFilterTestCase : FilterTestCaseBase
    {
        AllFilterTestCase() :
            FilterTestCaseBase("all", filter::All())
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } all_filter_test;

    struct SupportsInstallActionFilterTestCase : FilterTestCaseBase
    {
        SupportsInstallActionFilterTestCase() :
            FilterTestCaseBase("supports install action", filter::SupportsAction<InstallAction>())
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } supports_install_action_filter_test;

    struct SupportsInstalledAtRootFilterTestCase : FilterTestCaseBase
    {
        SupportsInstalledAtRootFilterTestCase() :
            FilterTestCaseBase("installed at root /", filter::InstalledAtRoot(FSEntry("/")))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1";
        }
    } supports_installed_at_root_slash;

    struct SupportsInstalledAtRootOtherFilterTestCase : FilterTestCaseBase
    {
        SupportsInstalledAtRootOtherFilterTestCase() :
            FilterTestCaseBase("installed at root /blah", filter::InstalledAtRoot(FSEntry("/blah")))
        {
        }

        virtual std::string get_expected() const
        {
            return "";
        }
    } supports_installed_at_root_other;

    struct NotMaskedFilterTestCase : FilterTestCaseBase
    {
        NotMaskedFilterTestCase() :
            FilterTestCaseBase("not masked", filter::NotMasked())
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } not_masked_filter_test;

    struct InstallableAndNotMaskedFilterTestCase : FilterTestCaseBase
    {
        InstallableAndNotMaskedFilterTestCase() :
            FilterTestCaseBase("installable and not masked", filter::And(filter::SupportsAction<InstallAction>(), filter::NotMasked()))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } installable_and_not_masked_filter_test;

    struct NotMaskedAndInstallableFilterTestCase : FilterTestCaseBase
    {
        NotMaskedAndInstallableFilterTestCase() :
            FilterTestCaseBase("not masked and installable", filter::And(filter::NotMasked(), filter::SupportsAction<InstallAction>()))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } not_masked_and_installable_filter_test;

    struct MatchesFilterTestCase : FilterTestCaseBase
    {
        MatchesFilterTestCase() :
            FilterTestCaseBase("matches", filter::Matches(parse_user_package_dep_spec("cat/a",
                            &env, UserPackageDepSpecOptions()), MatchPackageOptions()))
        {
        }        

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2";
        }
    } matches_filter_test;

    struct MatchesCatWildcardFilterTestCase : FilterTestCaseBase
    {
        MatchesCatWildcardFilterTestCase() :
            FilterTestCaseBase("matches cat wildcard", filter::Matches(parse_user_package_dep_spec("*/a",
                            &env, UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions()))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2";
        }
    } matches_cat_wildcard_filter_test;

    struct MatchesPkgWildcardFilterTestCase : FilterTestCaseBase
    {
        MatchesPkgWildcardFilterTestCase() :
            FilterTestCaseBase("matches pkg wildcard", filter::Matches(parse_user_package_dep_spec("cat/*",
                            &env, UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions()))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::inst_repo1, "
                "cat/a-1:0::repo1, "
                "cat/a-1:0::repo2, "
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } matches_pkg_wildcard_filter_test;

    struct MatchesAllWildcardFilterTestCase : FilterTestCaseBase
    {
        MatchesAllWildcardFilterTestCase() :
            FilterTestCaseBase("matches all wildcard", filter::Matches(
                        parse_user_package_dep_spec(">=*/*-2",
                            &env, UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions()))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-2:0::repo2, "
                "cat/b-2:0::repo1, "
                "cat/c-3:0::repo2";
        }
    } matches_all_wildcard_filter_test;

}

