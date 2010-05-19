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
    TESTCASE_SEMIREGULAR(Generator, generator::All());
    TESTCASE_STRINGIFYABLE(Generator, generator::All(), "all packages");

    struct GeneratorTestCaseBase : TestCase
    {
        Generator generator;
        TestEnvironment env;
        std::tr1::shared_ptr<FakeRepository> repo1;
        std::tr1::shared_ptr<FakeRepository> repo2;
        std::tr1::shared_ptr<FakeInstalledRepository> inst_repo1;

        GeneratorTestCaseBase(const std::string & s, const Generator & f) :
            TestCase("generator " + s + " with " + stringify(f)),
            generator(f),
            repo1(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo1")))),
            repo2(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo2")))),
            inst_repo1(new FakeInstalledRepository(
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
            std::tr1::shared_ptr<const PackageIDSequence> got(env[selection::AllVersionsSorted(generator)]);

            TEST_CHECK(got);
            TEST_CHECK_EQUAL(join(indirect_iterator(got->begin()), indirect_iterator(got->end()), ", "), expected);

            std::tr1::shared_ptr<const PackageIDSequence> got_none(env[selection::AllVersionsSorted(generator |
                        filter::SupportsAction<InstallAction>() | filter::SupportsAction<UninstallAction>())]);
            TEST_CHECK(got_none);
            TEST_CHECK_EQUAL(join(indirect_iterator(got_none->begin()), indirect_iterator(got_none->end()), ", "), "");
        }
    };

    struct AllGeneratorTestCase : GeneratorTestCaseBase
    {
        AllGeneratorTestCase() :
            GeneratorTestCaseBase("all", generator::All())
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
    } all_generator_test;

    struct MatchesGeneratorTestCase : GeneratorTestCaseBase
    {
        MatchesGeneratorTestCase() :
            GeneratorTestCaseBase("matches", generator::Matches(parse_user_package_dep_spec("cat/a",
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
    } matches_generator_test;

    struct MatchesCatWildcardGeneratorTestCase : GeneratorTestCaseBase
    {
        MatchesCatWildcardGeneratorTestCase() :
            GeneratorTestCaseBase("matches cat wildcard", generator::Matches(parse_user_package_dep_spec("*/a",
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
    } matches_cat_wildcard_generator_test;

    struct MatchesPkgWildcardGeneratorTestCase : GeneratorTestCaseBase
    {
        MatchesPkgWildcardGeneratorTestCase() :
            GeneratorTestCaseBase("matches pkg wildcard", generator::Matches(parse_user_package_dep_spec("cat/*",
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
    } matches_pkg_wildcard_generator_test;

    struct MatchesAllWildcardGeneratorTestCase : GeneratorTestCaseBase
    {
        MatchesAllWildcardGeneratorTestCase() :
            GeneratorTestCaseBase("matches all wildcard", generator::Matches(
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
    } matches_all_wildcard_generator_test;

    struct PackageGeneratorTestCase : GeneratorTestCaseBase
    {
        PackageGeneratorTestCase() :
            GeneratorTestCaseBase("package", generator::Package(QualifiedPackageName("cat/a")))
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
    } package_generator_test;

    struct NoPackageGeneratorTestCase : GeneratorTestCaseBase
    {
        NoPackageGeneratorTestCase() :
            GeneratorTestCaseBase("package", generator::Package(QualifiedPackageName("cat/d")))
        {
        }

        virtual std::string get_expected() const
        {
            return "";
        }
    } no_package_generator_test;

    struct RepositoryGeneratorTestCase : GeneratorTestCaseBase
    {
        RepositoryGeneratorTestCase() :
            GeneratorTestCaseBase("repository", generator::InRepository(RepositoryName("repo1")))
        {
        }

        virtual std::string get_expected() const
        {
            return
                "cat/a-1:0::repo1, "
                "cat/b-2:0::repo1";
        }
    } repository_generator_test;

    struct NoRepositoryGeneratorTestCase : GeneratorTestCaseBase
    {
        NoRepositoryGeneratorTestCase() :
            GeneratorTestCaseBase("no repository", generator::InRepository(RepositoryName("repo3")))
        {
        }

        virtual std::string get_expected() const
        {
            return "";
        }
    } no_repository_generator_test;

    struct CategoryGeneratorTestCase : GeneratorTestCaseBase
    {
        CategoryGeneratorTestCase() :
            GeneratorTestCaseBase("category", generator::Category(CategoryNamePart("cat")))
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
    } category_generator_test;

    struct NoCategoryGeneratorTestCase : GeneratorTestCaseBase
    {
        NoCategoryGeneratorTestCase() :
            GeneratorTestCaseBase("no category", generator::Category(CategoryNamePart("a")))
        {
        }

        virtual std::string get_expected() const
        {
            return "";
        }
    } no_category_generator_test;

    struct IntersectionGeneratorTestCase : GeneratorTestCaseBase
    {
        IntersectionGeneratorTestCase() :
            GeneratorTestCaseBase("intersection", generator::Intersection(
                    generator::Matches(parse_user_package_dep_spec("*/a",
                            &env, UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions()),
                    generator::Matches(parse_user_package_dep_spec("cat/*",
                            &env, UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions())
                    ))
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
    } intersection_generator_test;

    struct SomeIDsMightSupportInstallActionGeneratorTestCase : GeneratorTestCaseBase
    {
        SomeIDsMightSupportInstallActionGeneratorTestCase() :
            GeneratorTestCaseBase("some IDs might support install",
                    generator::SomeIDsMightSupportAction<InstallAction>())
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
    } some_ids_might_support_install_action_generator_test;
}

