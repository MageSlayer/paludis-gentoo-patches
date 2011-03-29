/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/version_requirements.hh>
#include <paludis/package_dep_spec_constraint.hh>

#include <paludis/util/clone-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string dump_version_requirement(const VersionRequirement & v)
    {
        return stringify(v.version_operator()) + stringify(v.version_spec());
    }

    class UserDepSpecTest :
        public testing::Test
    {
        protected:
            void check_spec(
                    const PackageDepSpec & spec,
                    const std::string & package,
                    const std::string & category_name_part,
                    const std::string & package_name_part,
                    const std::string & version_requirements,
                    const std::string & version_requirement_mode,
                    const std::string & slot_requirement,
                    const std::string & in_repository,
                    const std::string & from_repository,
                    const std::string & additional_requirement,
                    const std::string & installed_at_path = "",
                    const std::string & installable_to_path_f = "",
                    const bool installable_to_path_s = false
                    );
    };
}

void
UserDepSpecTest::check_spec(
        const PackageDepSpec & spec,
        const std::string & package,
        const std::string & category_name_part,
        const std::string & package_name_part,
        const std::string & version_requirements,
        const std::string & version_requirement_mode,
        const std::string & slot_requirement,
        const std::string & in_repository,
        const std::string & from_repository,
        const std::string & additional_requirement,
        const std::string & installed_at_path,
        const std::string & installable_to_path_f,
        const bool installable_to_path_s)
{

    if (package.empty())
        EXPECT_TRUE(! spec.package_name_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.package_name_constraint()));
        EXPECT_EQ(package, stringify(spec.package_name_constraint()->name()));
    }

    if (category_name_part.empty())
        EXPECT_TRUE(! spec.category_name_part_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.category_name_part_constraint()));
        EXPECT_EQ(category_name_part, stringify(spec.category_name_part_constraint()->name_part()));
    }

    if (package_name_part.empty())
        EXPECT_TRUE(! spec.package_name_part_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.package_name_part_constraint()));
        EXPECT_EQ(package_name_part, stringify(spec.package_name_part_constraint()->name_part()));
    }

    if (! version_requirement_mode.empty())
        EXPECT_EQ(version_requirement_mode, stringify(spec.version_requirements_mode()));

    if (version_requirements.empty())
        EXPECT_TRUE((! spec.version_requirements_ptr()) || spec.version_requirements_ptr()->empty());
    else
    {
        EXPECT_TRUE(bool(spec.version_requirements_ptr()));
        EXPECT_EQ(version_requirements, stringify(join(
                        spec.version_requirements_ptr()->begin(), spec.version_requirements_ptr()->end(), ", ", &dump_version_requirement)));
    }

    if (slot_requirement.empty())
        EXPECT_TRUE(! spec.slot_requirement_ptr());
    else
    {
        EXPECT_TRUE(bool(spec.slot_requirement_ptr()));
        EXPECT_EQ(slot_requirement, stringify(*spec.slot_requirement_ptr()));
    }

    if (from_repository.empty())
        EXPECT_TRUE(! spec.from_repository_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.from_repository_constraint()));
        EXPECT_EQ(from_repository, stringify(spec.from_repository_constraint()->name()));
    }

    if (in_repository.empty())
        EXPECT_TRUE(! spec.in_repository_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.in_repository_constraint()));
        EXPECT_EQ(in_repository, stringify(spec.in_repository_constraint()->name()));
    }

    if (additional_requirement.empty())
        EXPECT_TRUE((! spec.additional_requirements_ptr()) || spec.additional_requirements_ptr()->empty());
    else
    {
        EXPECT_TRUE(bool(spec.additional_requirements_ptr()));
        EXPECT_EQ(additional_requirement, stringify(join(
                        indirect_iterator(spec.additional_requirements_ptr()->begin()),
                        indirect_iterator(spec.additional_requirements_ptr()->end()), ", ")));
    }

    if (installed_at_path.empty())
        EXPECT_TRUE(! spec.installed_at_path_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.installed_at_path_constraint()));
        EXPECT_EQ(installed_at_path, stringify(spec.installed_at_path_constraint()->path()));
    }

    if (installable_to_path_f.empty())
        EXPECT_TRUE(! spec.installable_to_path_constraint());
    else
    {
        EXPECT_TRUE(bool(spec.installable_to_path_constraint()));
        EXPECT_EQ(installable_to_path_f, stringify(spec.installable_to_path_constraint()->path()));
        EXPECT_EQ(installable_to_path_s, spec.installable_to_path_constraint()->include_masked());
    }

}

TEST_F(UserDepSpecTest, Parsing)
{
    TestEnvironment env;

    PackageDepSpec a(parse_user_package_dep_spec("foo/bar", &env, { }));
    check_spec(a, "foo/bar", "", "", "", "", "", "", "", "");

    PackageDepSpec b(parse_user_package_dep_spec(">=foo/bar-1.2.3", &env, { }));
    check_spec(b, "foo/bar", "", "", ">=1.2.3", "", "", "", "", "");

    PackageDepSpec c(parse_user_package_dep_spec("foo/bar:baz", &env, { }));
    check_spec(c, "foo/bar", "", "", "", "", ":baz", "", "", "");

    PackageDepSpec d(parse_user_package_dep_spec("=foo/bar-1.2*:1.2.1", &env, { }));
    check_spec(d, "foo/bar", "", "", "=*1.2", "", ":1.2.1", "", "", "");

    PackageDepSpec e(parse_user_package_dep_spec("foo/bar:1.2.1", &env, { }));
    check_spec(e, "foo/bar", "", "", "", "", ":1.2.1", "", "", "");

    PackageDepSpec f(parse_user_package_dep_spec("foo/bar:0", &env, { }));
    check_spec(f, "foo/bar", "", "", "", "", ":0", "", "", "");

    PackageDepSpec g(parse_user_package_dep_spec("foo/bar-100dpi", &env, { }));
    check_spec(g, "foo/bar-100dpi", "", "", "", "", "", "", "", "");

    PackageDepSpec h(parse_user_package_dep_spec(">=foo/bar-100dpi-1.23", &env, { }));
    check_spec(h, "foo/bar-100dpi", "", "", ">=1.23", "", "", "", "", "");

    EXPECT_THROW(parse_user_package_dep_spec("", &env, { }), PackageDepSpecError);
    EXPECT_THROW(parse_user_package_dep_spec("=foo/bar-1.2[=1.3]", &env, { }), PackageDepSpecError);

    PackageDepSpec i(parse_user_package_dep_spec("foo/bar[one][-two]", &env, { }));
    check_spec(i, "foo/bar", "", "", "", "", "", "", "", "[-two], [one]");

    PackageDepSpec j(parse_user_package_dep_spec("=foo/bar-scm-r3", &env, { }));
    check_spec(j, "foo/bar", "", "", "=scm-r3", "", "", "", "", "");

    PackageDepSpec k(parse_user_package_dep_spec("=foo/bar-scm", &env, { }));
    check_spec(k, "foo/bar", "", "", "=scm", "", "", "", "", "");

    PackageDepSpec l(parse_user_package_dep_spec("foo/bar[one][-two][>=1.2&<2.0]", &env, { }));
    check_spec(l, "foo/bar", "", "", ">=1.2, <2.0", "and", "", "", "", "[-two], [one]");

    PackageDepSpec m(parse_user_package_dep_spec("foo/bar[=1.2|=1.3*|~1.4]", &env, { }));
    check_spec(m, "foo/bar", "", "", "=1.2, =*1.3, ~1.4", "or", "", "", "", "");

    PackageDepSpec n(parse_user_package_dep_spec("=foo/bar--1.2.3", &env, { }));
    check_spec(n, "foo/bar-", "", "", "=1.2.3", "", "", "", "", "");

    EXPECT_THROW(parse_user_package_dep_spec("=foo/bar--", &env, { }), PackageDepSpecError);

    PackageDepSpec o(parse_user_package_dep_spec("=foo/bar---1.2.3", &env, { }));
    check_spec(o, "foo/bar--", "", "", "=1.2.3", "", "", "", "", "");

    EXPECT_THROW(parse_user_package_dep_spec("=foo/bar[.foo]", &env, { }), PackageDepSpecError);

    PackageDepSpec p(parse_user_package_dep_spec("foo/bar[.key=value]", &env, { }));
    check_spec(p, "foo/bar", "", "", "", "", "", "", "", "[.key=value]");

    EXPECT_THROW(parse_user_package_dep_spec("=foo/bar[.foo?q]", &env, { }), PackageDepSpecError);

    PackageDepSpec q(parse_user_package_dep_spec("foo/bar[.foo?]", &env, { }));
    check_spec(q, "foo/bar", "", "", "", "", "", "", "", "[.foo?]");

    PackageDepSpec r(parse_user_package_dep_spec("foo/bar[.$short_description=value]", &env, { }));
    check_spec(r, "foo/bar", "", "", "", "", "", "", "", "[.$short_description=value]");
}

TEST_F(UserDepSpecTest, Unspecified)
{
    TestEnvironment env;

    PackageDepSpec a(parse_user_package_dep_spec("*/*", &env, { updso_allow_wildcards }));
    check_spec(a, "", "", "", "", "", "", "", "", "");

    PackageDepSpec b(parse_user_package_dep_spec("foo/*", &env, { updso_allow_wildcards }));
    check_spec(b, "", "foo", "", "", "", "", "", "", "");

    PackageDepSpec c(parse_user_package_dep_spec("*/foo", &env, { updso_allow_wildcards }));
    check_spec(c, "", "", "foo", "", "", "", "", "", "");

    PackageDepSpec d(parse_user_package_dep_spec("~*/*-0", &env, { updso_allow_wildcards }));
    check_spec(d, "", "", "", "~0", "", "", "", "", "");

    PackageDepSpec e(parse_user_package_dep_spec(">=foo/*-1.23", &env, { updso_allow_wildcards }));
    check_spec(e, "", "foo", "", ">=1.23", "", "", "", "", "");

    PackageDepSpec f(parse_user_package_dep_spec("=*/foo-1*", &env, { updso_allow_wildcards }));
    check_spec(f, "", "", "foo", "=*1", "", "", "", "", "");
}

TEST_F(UserDepSpecTest, Repos)
{
    TestEnvironment env;

    PackageDepSpec a(parse_user_package_dep_spec("cat/pkg::repo",
                &env, { updso_allow_wildcards }));
    check_spec(a, "cat/pkg", "", "", "", "", "", "repo", "", "");

    PackageDepSpec b(parse_user_package_dep_spec("cat/pkg::->repo",
                &env, { updso_allow_wildcards }));
    check_spec(b, "cat/pkg", "", "", "", "", "", "repo", "", "");

    PackageDepSpec c(parse_user_package_dep_spec("cat/pkg::repo->",
                &env, { updso_allow_wildcards }));
    check_spec(c, "cat/pkg", "", "", "", "", "", "", "repo", "");

    PackageDepSpec d(parse_user_package_dep_spec("cat/pkg::r1->r2",
                &env, { updso_allow_wildcards }));
    check_spec(d, "cat/pkg", "", "", "", "", "", "r2", "r1", "");

    PackageDepSpec e(parse_user_package_dep_spec("cat/pkg::/",
                &env, { updso_allow_wildcards }));
    check_spec(e, "cat/pkg", "", "", "", "", "", "", "", "", "/");

    PackageDepSpec f(parse_user_package_dep_spec("cat/pkg::/path",
                &env, { updso_allow_wildcards }));
    check_spec(f, "cat/pkg", "", "", "", "", "", "", "", "", "/path");

    PackageDepSpec g(parse_user_package_dep_spec("cat/pkg::/?",
                &env, { updso_allow_wildcards }));
    check_spec(g, "cat/pkg", "", "", "", "", "", "", "", "", "", "/", false);

    PackageDepSpec h(parse_user_package_dep_spec("cat/pkg::/path?",
                &env, { updso_allow_wildcards }));
    check_spec(h, "cat/pkg", "", "", "", "", "", "", "", "", "", "/path", false);

    PackageDepSpec i(parse_user_package_dep_spec("cat/pkg::/??",
                &env, { updso_allow_wildcards }));
    check_spec(i, "cat/pkg", "", "", "", "", "", "", "", "", "", "/", true);

    PackageDepSpec j(parse_user_package_dep_spec("cat/pkg::/path??",
                &env, { updso_allow_wildcards }));
    check_spec(j, "cat/pkg", "", "", "", "", "", "", "", "", "", "/path", true);
}

TEST_F(UserDepSpecTest, Disambiguation)
{
    TestEnvironment env;
    std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake"))));
    std::shared_ptr<FakeInstalledRepository> fake_inst(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake_inst"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(1, fake);
    env.add_repository(2, fake_inst);
    fake->add_version("cat", "pkg1", "1");
    fake->add_version("cat", "pkg2", "1");
    fake->add_version("dog", "pkg2", "1");
    fake->add_version("cat", "pkg3", "1");
    fake_inst->add_version("dog", "pkg3", "1");

    PackageDepSpec a(parse_user_package_dep_spec("pkg1", &env, { }));
    check_spec(a, "cat/pkg1", "", "", "", "", "", "", "", "");

    EXPECT_THROW(parse_user_package_dep_spec("pkg2", &env, { }), AmbiguousPackageNameError);
    PackageDepSpec b(parse_user_package_dep_spec("pkg3", &env, { }));
    check_spec(b, "dog/pkg3", "", "", "", "", "", "", "", "");

    PackageDepSpec c(parse_user_package_dep_spec("pkg3", &env, { }, filter::SupportsAction<InstallAction>()));
    check_spec(c, "cat/pkg3", "", "", "", "", "", "", "", "");

    EXPECT_THROW(parse_user_package_dep_spec("pkg4", &env, { }), NoSuchPackageError);

    EXPECT_THROW(parse_user_package_dep_spec("pkg5", &env, { updso_no_disambiguation }), PackageDepSpecError);

    PackageDepSpec d(parse_user_package_dep_spec("=pkg1-1", &env, { }));
    EXPECT_EQ("=cat/pkg1-1", stringify(d));
    EXPECT_THROW(parse_user_package_dep_spec("=pkg1-42", &env, { }), NoSuchPackageError);

    PackageDepSpec e(parse_user_package_dep_spec("=pkg1-1:0", &env, { }));
    EXPECT_EQ("=cat/pkg1-1:0", stringify(e));
    EXPECT_THROW(parse_user_package_dep_spec("=pkg1-42:0", &env, { }), NoSuchPackageError);

    PackageDepSpec f(parse_user_package_dep_spec("pkg1:0", &env, { }));
    EXPECT_EQ("cat/pkg1:0", stringify(f));

    PackageDepSpec g(parse_user_package_dep_spec("pkg1[-foo]", &env, { }));
    EXPECT_EQ("cat/pkg1[-foo]", stringify(g));
    EXPECT_THROW(parse_user_package_dep_spec("pkg1[foo]", &env, { }), NoSuchPackageError);

    PackageDepSpec h(parse_user_package_dep_spec("pkg1[=1]", &env, { }));
    EXPECT_EQ("=cat/pkg1-1", stringify(h));
    EXPECT_THROW(parse_user_package_dep_spec("pkg1[=42]", &env, { }), NoSuchPackageError);

    PackageDepSpec i(parse_user_package_dep_spec("pkg1::fake", &env, { }));
    EXPECT_EQ("cat/pkg1::fake", stringify(i));
}

TEST(UserPackageDepSpec, Sets)
{
    TestEnvironment env;
    std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake"))));
    env.add_repository(1, fake);
    fake->add_version("cat", "world", "1");
    fake->add_version("cat", "moon", "1");

    EXPECT_THROW(parse_user_package_dep_spec("world", &env, { updso_throw_if_set }), GotASetNotAPackageDepSpec);
    EXPECT_THROW(parse_user_package_dep_spec("system", &env, { updso_throw_if_set }), GotASetNotAPackageDepSpec);
    PackageDepSpec a(parse_user_package_dep_spec("moon", &env, { updso_throw_if_set }));
    EXPECT_EQ("cat/moon", stringify(a));
    EXPECT_THROW(parse_user_package_dep_spec("mars", &env, { updso_throw_if_set }), NoSuchPackageError);

    EXPECT_THROW(parse_user_package_dep_spec("world", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
    EXPECT_THROW(parse_user_package_dep_spec("system", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
    EXPECT_THROW(parse_user_package_dep_spec("moon", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
    EXPECT_THROW(parse_user_package_dep_spec("mars", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);

    EXPECT_THROW(parse_user_package_dep_spec("=world-123", &env, { updso_no_disambiguation, updso_throw_if_set }), PackageDepSpecError);
    EXPECT_THROW(parse_user_package_dep_spec("world*", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
    EXPECT_THROW(parse_user_package_dep_spec("world**", &env, { updso_no_disambiguation, updso_throw_if_set }), PackageDepSpecError);

    EXPECT_THROW(parse_user_package_dep_spec("system", &env, { }), NoSuchPackageError);
}

TEST_F(UserDepSpecTest, Keys)
{
    TestEnvironment env;
    std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake"))));
    env.add_repository(1, fake);

    std::shared_ptr<FakePackageID> pkg1(fake->add_version("cat", "pkg1", "1"));
    pkg1->keywords_key()->set_from_string("~a ~b");
    std::shared_ptr<FakePackageID> pkg2(fake->add_version("cat", "pkg1", "2"));
    pkg2->keywords_key()->set_from_string("~a ~c");
    std::shared_ptr<FakePackageID> pkg3(fake->add_version("cat", "pkg1", "3"));
    pkg3->keywords_key()->set_from_string("~d");

    PackageDepSpec a(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~a]", &env, { }));
    EXPECT_TRUE(match_package(env, a, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(match_package(env, a, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, a, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec b(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~b]", &env, { }));
    EXPECT_TRUE(match_package(env, b, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, b, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, b, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec c(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~c]", &env, { }));
    EXPECT_TRUE(! match_package(env, c, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(match_package(env, c, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, c, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec d(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS>~a]", &env, { }));
    EXPECT_TRUE(! match_package(env, d, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, d, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, d, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec e(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS=~d]", &env, { }));
    EXPECT_TRUE(! match_package(env, e, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, e, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(match_package(env, e, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec f(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS=~a ~c]", &env, { }));
    EXPECT_TRUE(! match_package(env, f, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(match_package(env, f, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, f, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec g(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER=42]", &env, { }));
    EXPECT_TRUE(match_package(env, g, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec h(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<41]", &env, { }));
    EXPECT_TRUE(! match_package(env, h, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec i(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<42]", &env, { }));
    EXPECT_TRUE(! match_package(env, i, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec j(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<43]", &env, { }));
    EXPECT_TRUE(match_package(env, j, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec k(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER>42]", &env, { }));
    EXPECT_TRUE(! match_package(env, k, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec l(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER>41]", &env, { }));
    EXPECT_TRUE(match_package(env, l, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec m(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER?]", &env, { }));
    EXPECT_TRUE(match_package(env, m, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec n(parse_user_package_dep_spec("cat/pkg1[.SPOON?]", &env, { }));
    EXPECT_TRUE(! match_package(env, n, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec o(parse_user_package_dep_spec("cat/pkg1[.$keywords<~a]", &env, { }));
    EXPECT_TRUE(match_package(env, o, pkg1, make_null_shared_ptr(), { }));
    EXPECT_TRUE(match_package(env, o, pkg2, make_null_shared_ptr(), { }));
    EXPECT_TRUE(! match_package(env, o, pkg3, make_null_shared_ptr(), { }));

    PackageDepSpec p(parse_user_package_dep_spec("cat/pkg1[.::$format=fake]", &env, { }));
    EXPECT_TRUE(match_package(env, p, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec q(parse_user_package_dep_spec("cat/pkg1[.::$format=e]", &env, { }));
    EXPECT_TRUE(! match_package(env, q, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec r(parse_user_package_dep_spec("cat/pkg1[.::format=fake]", &env, { }));
    EXPECT_TRUE(match_package(env, r, pkg1, make_null_shared_ptr(), { }));

    PackageDepSpec s(parse_user_package_dep_spec("cat/pkg1[.::format=e]", &env, { }));
    EXPECT_TRUE(! match_package(env, s, pkg1, make_null_shared_ptr(), { }));
}

