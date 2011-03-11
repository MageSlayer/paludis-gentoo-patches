/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/util/make_named_values.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;
using namespace paludis::erepository;

/** \file
 * Test cases for DepParser.
 *
 */

namespace test_cases
{
    /**
     * \test Test DepParser with an empty input.
     *
     */
    struct DepParserEmptyTest : TestCase
    {
        DepParserEmptyTest() : TestCase("empty") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            UnformattedPrettyPrinter ff;
            SpecTreePrettyPrinter d(ff, { });
            parse_depend("",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_empty;

    /**
     * \test Test DepParser with a blank input.
     *
     */
    struct DepParserBlankTest : TestCase
    {
        DepParserBlankTest() : TestCase("blank") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            UnformattedPrettyPrinter ff;
            SpecTreePrettyPrinter d(ff, { });
            parse_depend("   \n   \t",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_blank;

    /**
     * \test Test DepParser with a package.
     *
     */
    struct DepParserPackageTest : TestCase
    {
        DepParserPackageTest() : TestCase("package") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            UnformattedPrettyPrinter ff;
            SpecTreePrettyPrinter d(ff, { });
            parse_depend("app-editors/vim",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim");
        }
    } test_dep_spec_parser_package;

    /**
     * \test Test DepParser with a decorated package.
     *
     */
    struct DepParserDecoratedPackageTest : TestCase
    {
        DepParserDecoratedPackageTest() : TestCase("decorated package") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d1(ff, { });
            parse_depend(">=app-editors/vim-6.4_alpha",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d1);
            TEST_CHECK_EQUAL(stringify(d1), ">=app-editors/vim-6.4_alpha");

            SpecTreePrettyPrinter d2(ff, { });
            parse_depend("=app-editors/vim-6.4_alpha-r1",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d2);
            TEST_CHECK_EQUAL(stringify(d2), "=app-editors/vim-6.4_alpha-r1");

            SpecTreePrettyPrinter d3(ff, { });
            parse_depend(">=app-editors/vim-6.4_alpha:one",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d3);
            TEST_CHECK_EQUAL(stringify(d3), ">=app-editors/vim-6.4_alpha:one");
        }
    } test_dep_spec_parser_decorated_package;

    /**
     * \test Test DepParser with a sequence of packages.
     *
     */
    struct DepParserPackagesTest : TestCase
    {
        DepParserPackagesTest() : TestCase("packages") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("app-editors/vim app-misc/hilite   \nsys-apps/findutils",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim app-misc/hilite sys-apps/findutils");
        }
    } test_dep_spec_parser_packages;

    struct DepParserAnyTest : TestCase
    {
        DepParserAnyTest() : TestCase("any") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("|| ( one/one two/two )",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one two/two )");
        }
    } test_dep_spec_parser_any;

    struct DepParserAnyUseTest : TestCase
    {
        DepParserAnyUseTest() : TestCase("any use") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("|| ( one/one foo? ( two/two ) )",
                    &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one foo? ( two/two ) )");

            TEST_CHECK_THROWS(parse_depend("|| ( one/one foo? ( two/two ) )",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);

            SpecTreePrettyPrinter e(ff, { });
            parse_depend("|| ( one/one ( foo? ( two/two ) ) )",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "|| ( one/one ( foo? ( two/two ) ) )");
        }
    } test_dep_spec_parser_any_use;

    /**
     * \test Test DepParser with an all group.
     *
     */
    struct DepParserAllTest : TestCase
    {
        DepParserAllTest() : TestCase("all") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend(" ( one/one two/two )    ",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "one/one two/two");
        }
    } test_dep_spec_parser_all;

    /**
     * \test Test DepParser with a use group.
     *
     */
    struct DepParserUseTest : TestCase
    {
        DepParserUseTest() : TestCase("use") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("foo? ( one/one )", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "foo? ( one/one )");
        }
    } test_dep_spec_parser_use;

    /**
     * \test Test DepParser with an inverse use group.
     *
     */
    struct DepParserInvUseTest : TestCase
    {
        DepParserInvUseTest() : TestCase("!use") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("!foo? ( one/one )", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "!foo? ( one/one )");
        }
    } test_dep_spec_parser_inv_use;

    struct DepParserURITest : TestCase
    {
        DepParserURITest() : TestCase("uri") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { ppo_multiline_allowed });
            parse_fetchable_uri("a\n->\tb", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "a -> b\n");

            SpecTreePrettyPrinter e(ff, { ppo_multiline_allowed });
            parse_fetchable_uri("a-> b", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "a->\nb\n");

            TEST_CHECK_THROWS(parse_fetchable_uri("a -> b",
                        &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), Exception);
        }
    } test_dep_spec_parser_uri;

    /**
     * \test Test DepParser nesting errors.
     *
     */
    struct DepParserBadNestingTest : TestCase
    {
        DepParserBadNestingTest() : TestCase("bad nesting") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            TEST_CHECK_THROWS(parse_depend("!foo? ( one/one",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("!foo? ( one/one ) )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("( ( ( ) )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("( ( ( ) ) ) )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend(")",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
        }
    } test_dep_spec_parser_bad_nesting;

    /**
     * \test Test DepParser weird errors.
     *
     */
    struct DepParserBadValuesTest : TestCase
    {
        DepParserBadValuesTest() : TestCase("bad values") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            TEST_CHECK_THROWS(parse_depend("||",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("|| ",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("foo?",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("!? ( )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("!foo? ||",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("(((",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend(")",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_depend("(foo/bar)",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_license("a -> b",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);

            TEST_CHECK_THROWS(parse_fetchable_uri("( -> )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("( -> )",
                        &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("foo? -> bar",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("a ->",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("a -> ( )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("a -> )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
            TEST_CHECK_THROWS(parse_fetchable_uri("a -> || ( )",
                        &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
        }
    } test_dep_spec_parser_bad_values;

    /**
     * \test Test DepParser label handling
     */
    struct DepParserLabelsTest : TestCase
    {
        DepParserLabelsTest() : TestCase("label handling") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_depend("build: one/one",
                    &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "build: one/one");
            TEST_CHECK_THROWS(parse_depend("build: one/one",
                        &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), EDepParseError);
        }
    } test_dep_spec_parser_labels;

    struct DepParserExheres0URILabelsTest : TestCase
    {
        DepParserExheres0URILabelsTest() : TestCase("exheres-0 uri label handling") { }

        void run()
        {
            UnformattedPrettyPrinter ff;
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            SpecTreePrettyPrinter d(ff, { });
            parse_fetchable_uri("http://foo/bar manual: two",
                    &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "http://foo/bar manual: two");
            TEST_CHECK_THROWS(parse_fetchable_uri("http://foo/bar monkey: two",
                        &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d), EDepParseError);
        }
    } test_dep_spec_parser_exheres_0_uri_labels;

    struct AnnotationsTest : TestCase
    {
        AnnotationsTest() : TestCase("annotations") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            UnformattedPrettyPrinter ff;
            SpecTreePrettyPrinter d(ff, { });
            parse_depend("cat/first [[ foo = bar bar = baz ]] cat/second cat/third [[ moo = oink ]]",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "cat/first [[ bar = [ baz ] foo = [ bar ] ]] cat/second cat/third [[ moo = [ oink ] ]]");

            SpecTreePrettyPrinter e(ff, { });
            parse_depend("bar? ( foo? ( cat/first [[ for = first ]] ) [[ for = foo ]] baz? ( ) [[ for = baz ]] ) [[ for = bar ]]",
                    &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "bar? ( foo? ( cat/first [[ for = [ first ] ]] ) "
                    "[[ for = [ foo ] ]] baz? ( ) [[ for = [ baz ] ]] ) [[ for = [ bar ] ]]");
        }
    } test_annotations;

    struct StarAnnotationsTest : TestCase
    {
        StarAnnotationsTest() : TestCase("*annotations") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, repo);
            std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            UnformattedPrettyPrinter ff;
            SpecTreePrettyPrinter d(ff, { ppo_include_special_annotations });
            parse_depend("cat/outer1 ( cat/mid1 ( cat/inner1 cat/inner2 ) [[ *note = [ first-inner ] ]] cat/mid2 ( cat/inner3 cat/inner4 ) "
                    "[[ *note = [ second-inner ] ]] cat/mid3 ) [[ *description = [ mid ] ]] cat/outer2",
                    &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);

            TEST_CHECK_EQUAL(stringify(d), "cat/outer1 ( cat/mid1 [[ description = [ mid ] ]] ( cat/inner1 [[ note = [ first-inner ] description = [ mid ] ]] "
                    "cat/inner2 [[ note = [ first-inner ] description = [ mid ] ]] ) [[ *note = [ first-inner ] ]] cat/mid2 [[ description = [ mid ] ]] "
                    "( cat/inner3 [[ note = [ second-inner ] description = [ mid ] ]] cat/inner4 [[ note = [ second-inner ] description = [ mid ] ]] ) "
                    "[[ *note = [ second-inner ] ]] cat/mid3 [[ description = [ mid ] ]] ) [[ *description = [ mid ] ]] cat/outer2");
        }
    } test_star_annotations;
}

