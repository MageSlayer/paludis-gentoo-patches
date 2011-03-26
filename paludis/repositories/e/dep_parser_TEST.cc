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

#include <paludis/unformatted_pretty_printer.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>

#include <sstream>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::erepository;

TEST(Depparser, Empty)
{
    TestEnvironment env;
    std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter d(ff, { });
    parse_depend("",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("", stringify(d));
}

TEST(DepParser, Blank)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter d(ff, { });
    parse_depend("   \n   \t",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("", stringify(d));
}

TEST(DepParser, Package)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter d(ff, { });
    parse_depend("app-editors/vim",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("app-editors/vim", stringify(d));
}

TEST(DepParser, DecoratedPackage)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d1(ff, { });
    parse_depend(">=app-editors/vim-6.4_alpha",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d1);
    EXPECT_EQ(">=app-editors/vim-6.4_alpha", stringify(d1));

    SpecTreePrettyPrinter d2(ff, { });
    parse_depend("=app-editors/vim-6.4_alpha-r1",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d2);
    EXPECT_EQ("=app-editors/vim-6.4_alpha-r1", stringify(d2));

    SpecTreePrettyPrinter d3(ff, { });
    parse_depend(">=app-editors/vim-6.4_alpha:one",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d3);
    EXPECT_EQ(">=app-editors/vim-6.4_alpha:one", stringify(d3));
}

TEST(DepParser, PackageSequence)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("app-editors/vim app-misc/hilite   \nsys-apps/findutils",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("app-editors/vim app-misc/hilite sys-apps/findutils", stringify(d));
}

TEST(DepParser, Any)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("|| ( one/one two/two )",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("|| ( one/one two/two )", stringify(d));
}

TEST(DepParser, AnyUse)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("|| ( one/one foo? ( two/two ) )",
            &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d);
    EXPECT_EQ("|| ( one/one foo? ( two/two ) )", stringify(d));

    EXPECT_THROW(parse_depend("|| ( one/one foo? ( two/two ) )",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);

    SpecTreePrettyPrinter e(ff, { });
    parse_depend("|| ( one/one ( foo? ( two/two ) ) )",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
    EXPECT_EQ("|| ( one/one ( foo? ( two/two ) ) )", stringify(e));
}

TEST(DepParser, All)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend(" ( one/one two/two )    ",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("one/one two/two", stringify(d));
}

TEST(DepParser, Use)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("foo? ( one/one )", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("foo? ( one/one )", stringify(d));
}

TEST(DepParser, InverseUse)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("!foo? ( one/one )", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("!foo? ( one/one )", stringify(d));
}

TEST(DepParser, URI)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { ppo_multiline_allowed });
    parse_fetchable_uri("a\n->\tb", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("a -> b\n", stringify(d));

    SpecTreePrettyPrinter e(ff, { ppo_multiline_allowed });
    parse_fetchable_uri("a-> b", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
    EXPECT_EQ("a->\nb\n", stringify(e));

    EXPECT_THROW(parse_fetchable_uri("a -> b",
                &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), Exception);
}

TEST(DepParser, NestingErrors)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    EXPECT_THROW(parse_depend("!foo? ( one/one",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("!foo? ( one/one ) )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("( ( ( ) )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("( ( ( ) ) ) )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend(")",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
}

TEST(DepParser, WeirdErrors)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    EXPECT_THROW(parse_depend("||",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("|| ",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("foo?",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("!? ( )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("!foo? ||",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("(((",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend(")",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_depend("(foo/bar)",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_license("a -> b",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);

    EXPECT_THROW(parse_fetchable_uri("( -> )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("( -> )",
                &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("foo? -> bar",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("a ->",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("a -> ( )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("a -> )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
    EXPECT_THROW(parse_fetchable_uri("a -> || ( )",
                &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d), Exception);
}

TEST(DepParser, Labels)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_depend("build: one/one",
            &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);
    EXPECT_EQ("build: one/one", stringify(d));
    EXPECT_THROW(parse_depend("build: one/one",
                &env, *EAPIData::get_instance()->eapi_from_string("0"), false)->top()->accept(d), EDepParseError);
}

TEST(DepParser, Exheres0URILables)
{
    UnformattedPrettyPrinter ff;
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    SpecTreePrettyPrinter d(ff, { });
    parse_fetchable_uri("http://foo/bar manual: two",
            &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);
    EXPECT_EQ("http://foo/bar manual: two", stringify(d));
    EXPECT_THROW(parse_fetchable_uri("http://foo/bar monkey: two",
                &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d), EDepParseError);
}

TEST(DepParser, Annotations)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter d(ff, { });
    parse_depend("cat/first [[ foo = bar bar = baz ]] cat/second cat/third [[ moo = oink ]]",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(d);
    EXPECT_EQ("cat/first [[ bar = [ baz ] foo = [ bar ] ]] cat/second cat/third [[ moo = [ oink ] ]]", stringify(d));

    SpecTreePrettyPrinter e(ff, { });
    parse_depend("bar? ( foo? ( cat/first [[ for = first ]] ) [[ for = foo ]] baz? ( ) [[ for = baz ]] ) [[ for = bar ]]",
            &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(e);
    EXPECT_EQ("bar? ( foo? ( cat/first [[ for = [ first ] ]] ) "
            "[[ for = [ foo ] ]] baz? ( ) [[ for = [ baz ] ]] ) [[ for = [ bar ] ]]", stringify(e));
}

TEST(DepParser, StarAnnotations)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter d(ff, { ppo_include_special_annotations });
    parse_depend("cat/outer1 ( cat/mid1 ( cat/inner1 cat/inner2 ) [[ *note = [ first-inner ] ]] cat/mid2 ( cat/inner3 cat/inner4 ) "
            "[[ *note = [ second-inner ] ]] cat/mid3 ) [[ *description = [ mid ] ]] cat/outer2",
            &env, *EAPIData::get_instance()->eapi_from_string("exheres-0"), false)->top()->accept(d);

    EXPECT_EQ("cat/outer1 ( cat/mid1 [[ description = [ mid ] ]] ( cat/inner1 [[ note = [ first-inner ] description = [ mid ] ]] "
            "cat/inner2 [[ note = [ first-inner ] description = [ mid ] ]] ) [[ *note = [ first-inner ] ]] cat/mid2 [[ description = [ mid ] ]] "
            "( cat/inner3 [[ note = [ second-inner ] description = [ mid ] ]] cat/inner4 [[ note = [ second-inner ] description = [ mid ] ]] ) "
            "[[ *note = [ second-inner ] ]] cat/mid3 [[ description = [ mid ] ]] ) [[ *description = [ mid ] ]] cat/outer2", stringify(d));
}

