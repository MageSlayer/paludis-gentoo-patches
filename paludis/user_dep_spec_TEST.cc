/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/package_database.hh>
#include <paludis/match_package.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/version_requirements.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    std::string dump_version_requirement(const VersionRequirement & v)
    {
        return stringify(v.version_operator()) + stringify(v.version_spec());
    }

    struct UserDepSpecTestCase : TestCase
    {
        UserDepSpecTestCase(const std::string & s) : TestCase(s) { }

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
                )
        {
            TestMessageSuffix s(stringify(spec), true);

            if (package.empty())
                TEST_CHECK(! spec.package_ptr());
            else
            {
                TEST_CHECK(bool(spec.package_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.package_ptr(), package);
            }

            if (category_name_part.empty())
                TEST_CHECK(! spec.category_name_part_ptr());
            else
            {
                TEST_CHECK(bool(spec.category_name_part_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.category_name_part_ptr(), category_name_part);
            }

            if (package_name_part.empty())
                TEST_CHECK(! spec.package_name_part_ptr());
            else
            {
                TEST_CHECK(bool(spec.package_name_part_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.package_name_part_ptr(), package_name_part);
            }

            if (! version_requirement_mode.empty())
                TEST_CHECK_STRINGIFY_EQUAL(spec.version_requirements_mode(), version_requirement_mode);

            if (version_requirements.empty())
                TEST_CHECK((! spec.version_requirements_ptr()) || spec.version_requirements_ptr()->empty());
            else
            {
                TEST_CHECK(bool(spec.version_requirements_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(join(spec.version_requirements_ptr()->begin(),
                            spec.version_requirements_ptr()->end(), ", ", &dump_version_requirement),
                        version_requirements);
            }

            if (slot_requirement.empty())
                TEST_CHECK(! spec.slot_requirement_ptr());
            else
            {
                TEST_CHECK(bool(spec.slot_requirement_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.slot_requirement_ptr(), slot_requirement);
            }

            if (from_repository.empty())
                TEST_CHECK(! spec.from_repository_ptr());
            else
            {
                TEST_CHECK(bool(spec.from_repository_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.from_repository_ptr(), from_repository);
            }

            if (in_repository.empty())
                TEST_CHECK(! spec.in_repository_ptr());
            else
            {
                TEST_CHECK(bool(spec.in_repository_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.in_repository_ptr(), in_repository);
            }

            if (additional_requirement.empty())
                TEST_CHECK((! spec.additional_requirements_ptr()) || spec.additional_requirements_ptr()->empty());
            else
            {
                TEST_CHECK(bool(spec.additional_requirements_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(join(indirect_iterator(spec.additional_requirements_ptr()->begin()),
                            indirect_iterator(spec.additional_requirements_ptr()->end()), ", "),
                        additional_requirement);
            }

            if (installed_at_path.empty())
                TEST_CHECK(! spec.installed_at_path_ptr());
            else
            {
                TEST_CHECK(bool(spec.installed_at_path_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(*spec.installed_at_path_ptr(),
                        installed_at_path);
            }

            if (installable_to_path_f.empty())
                TEST_CHECK(! spec.installable_to_path_ptr());
            else
            {
                TEST_CHECK(bool(spec.installable_to_path_ptr()));
                TEST_CHECK_STRINGIFY_EQUAL(spec.installable_to_path_ptr()->path(), installable_to_path_f);
                TEST_CHECK_EQUAL(spec.installable_to_path_ptr()->include_masked(), installable_to_path_s);
            }

        }
    };
}

namespace test_cases
{
    struct UserPackageDepSpecTest : UserDepSpecTestCase
    {
        UserPackageDepSpecTest() : UserDepSpecTestCase("user package dep spec") { }

        void run()
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

            TEST_CHECK_THROWS(parse_user_package_dep_spec("", &env, { }), PackageDepSpecError);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("=foo/bar-1.2[=1.3]", &env, { }), PackageDepSpecError);

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

            TEST_CHECK_THROWS(parse_user_package_dep_spec("=foo/bar--", &env, { }), PackageDepSpecError);

            PackageDepSpec o(parse_user_package_dep_spec("=foo/bar---1.2.3", &env, { }));
            check_spec(o, "foo/bar--", "", "", "=1.2.3", "", "", "", "", "");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("=foo/bar[.foo]", &env, { }), PackageDepSpecError);

            PackageDepSpec p(parse_user_package_dep_spec("foo/bar[.key=value]", &env, { }));
            check_spec(p, "foo/bar", "", "", "", "", "", "", "", "[.key=value]");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("=foo/bar[.foo?q]", &env, { }), PackageDepSpecError);

            PackageDepSpec q(parse_user_package_dep_spec("foo/bar[.foo?]", &env, { }));
            check_spec(q, "foo/bar", "", "", "", "", "", "", "", "[.foo?]");

            PackageDepSpec r(parse_user_package_dep_spec("foo/bar[.$short_description=value]", &env, { }));
            check_spec(r, "foo/bar", "", "", "", "", "", "", "", "[.$short_description=value]");
        }
    } test_user_package_dep_spec;

    struct UserPackageDepSpecUnspecificTest : UserDepSpecTestCase
    {
        UserPackageDepSpecUnspecificTest() : UserDepSpecTestCase("user package dep spec unspecific") { }

        void run()
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
    } test_user_package_dep_spec_unspecific;

    struct UserPackageDepSpecReposTest : UserDepSpecTestCase
    {
        UserPackageDepSpecReposTest() : UserDepSpecTestCase("user package dep spec repos") { }

        void run()
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
    } test_user_package_dep_spec_repo;

    struct UserPackageDepSpecDisambiguationTest : UserDepSpecTestCase
    {
        UserPackageDepSpecDisambiguationTest() : UserDepSpecTestCase("user package dep spec disambiguation") { }

        void run()
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
            env.package_database()->add_repository(1, fake);
            env.package_database()->add_repository(2, fake_inst);
            fake->add_version("cat", "pkg1", "1");
            fake->add_version("cat", "pkg2", "1");
            fake->add_version("dog", "pkg2", "1");
            fake->add_version("cat", "pkg3", "1");
            fake_inst->add_version("dog", "pkg3", "1");

            PackageDepSpec a(parse_user_package_dep_spec("pkg1", &env, { }));
            check_spec(a, "cat/pkg1", "", "", "", "", "", "", "", "");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg2", &env, { }), AmbiguousPackageNameError);
            PackageDepSpec b(parse_user_package_dep_spec("pkg3", &env, { }));
            check_spec(b, "dog/pkg3", "", "", "", "", "", "", "", "");

            PackageDepSpec c(parse_user_package_dep_spec("pkg3", &env, { }, filter::SupportsAction<InstallAction>()));
            check_spec(c, "cat/pkg3", "", "", "", "", "", "", "", "");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg4", &env, { }), NoSuchPackageError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg5", &env, { updso_no_disambiguation }), PackageDepSpecError);

            PackageDepSpec d(parse_user_package_dep_spec("=pkg1-1", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(d, "=cat/pkg1-1");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("=pkg1-42", &env, { }), NoSuchPackageError);

            PackageDepSpec e(parse_user_package_dep_spec("=pkg1-1:0", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(e, "=cat/pkg1-1:0");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("=pkg1-42:0", &env, { }), NoSuchPackageError);

            PackageDepSpec f(parse_user_package_dep_spec("pkg1:0", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(f, "cat/pkg1:0");

            PackageDepSpec g(parse_user_package_dep_spec("pkg1[-foo]", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(g, "cat/pkg1[-foo]");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg1[foo]", &env, { }), NoSuchPackageError);

            PackageDepSpec h(parse_user_package_dep_spec("pkg1[=1]", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(h, "=cat/pkg1-1");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg1[=42]", &env, { }), NoSuchPackageError);

            PackageDepSpec i(parse_user_package_dep_spec("pkg1::fake", &env, { }));
            TEST_CHECK_STRINGIFY_EQUAL(i, "cat/pkg1::fake");
        }
    } test_user_package_dep_spec_disambiguation;

    struct UserPackageDepSpecSetsTest : TestCase
    {
        UserPackageDepSpecSetsTest() : TestCase("user package dep spec sets") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake"))));
            env.package_database()->add_repository(1, fake);
            fake->add_version("cat", "world", "1");
            fake->add_version("cat", "moon", "1");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("world", &env, { updso_throw_if_set }), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, { updso_throw_if_set }), GotASetNotAPackageDepSpec);
            PackageDepSpec a(parse_user_package_dep_spec("moon", &env, { updso_throw_if_set }));
            TEST_CHECK_STRINGIFY_EQUAL(a, "cat/moon");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("mars", &env, { updso_throw_if_set }), NoSuchPackageError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("world", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("moon", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("mars", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("=world-123", &env, { updso_no_disambiguation, updso_throw_if_set }), PackageDepSpecError);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("world*", &env, { updso_no_disambiguation, updso_throw_if_set }), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("world**", &env, { updso_no_disambiguation, updso_throw_if_set }), PackageDepSpecError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, { }), NoSuchPackageError);
        }
    } test_user_package_dep_spec_sets;

    struct UserPackageDepSpecUserKeyReqTest : UserDepSpecTestCase
    {
        UserPackageDepSpecUserKeyReqTest() : UserDepSpecTestCase("user package dep spec user key requirements") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake"))));
            env.package_database()->add_repository(1, fake);

            std::shared_ptr<FakePackageID> pkg1(fake->add_version("cat", "pkg1", "1"));
            pkg1->keywords_key()->set_from_string("~a ~b");
            std::shared_ptr<FakePackageID> pkg2(fake->add_version("cat", "pkg1", "2"));
            pkg2->keywords_key()->set_from_string("~a ~c");
            std::shared_ptr<FakePackageID> pkg3(fake->add_version("cat", "pkg1", "3"));
            pkg3->keywords_key()->set_from_string("~d");

            PackageDepSpec a(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~a]", &env, { }));
            TEST_CHECK(match_package(env, a, *pkg1, { }));
            TEST_CHECK(match_package(env, a, *pkg2, { }));
            TEST_CHECK(! match_package(env, a, *pkg3, { }));

            PackageDepSpec b(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~b]", &env, { }));
            TEST_CHECK(match_package(env, b, *pkg1, { }));
            TEST_CHECK(! match_package(env, b, *pkg2, { }));
            TEST_CHECK(! match_package(env, b, *pkg3, { }));

            PackageDepSpec c(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS<~c]", &env, { }));
            TEST_CHECK(! match_package(env, c, *pkg1, { }));
            TEST_CHECK(match_package(env, c, *pkg2, { }));
            TEST_CHECK(! match_package(env, c, *pkg3, { }));

            PackageDepSpec d(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS>~a]", &env, { }));
            TEST_CHECK(! match_package(env, d, *pkg1, { }));
            TEST_CHECK(! match_package(env, d, *pkg2, { }));
            TEST_CHECK(! match_package(env, d, *pkg3, { }));

            PackageDepSpec e(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS=~d]", &env, { }));
            TEST_CHECK(! match_package(env, e, *pkg1, { }));
            TEST_CHECK(! match_package(env, e, *pkg2, { }));
            TEST_CHECK(match_package(env, e, *pkg3, { }));

            PackageDepSpec f(parse_user_package_dep_spec("cat/pkg1[.KEYWORDS=~a ~c]", &env, { }));
            TEST_CHECK(! match_package(env, f, *pkg1, { }));
            TEST_CHECK(match_package(env, f, *pkg2, { }));
            TEST_CHECK(! match_package(env, f, *pkg3, { }));

            PackageDepSpec g(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER=42]", &env, { }));
            TEST_CHECK(match_package(env, g, *pkg1, { }));

            PackageDepSpec h(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<41]", &env, { }));
            TEST_CHECK(! match_package(env, h, *pkg1, { }));

            PackageDepSpec i(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<42]", &env, { }));
            TEST_CHECK(! match_package(env, i, *pkg1, { }));

            PackageDepSpec j(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER<43]", &env, { }));
            TEST_CHECK(match_package(env, j, *pkg1, { }));

            PackageDepSpec k(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER>42]", &env, { }));
            TEST_CHECK(! match_package(env, k, *pkg1, { }));

            PackageDepSpec l(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER>41]", &env, { }));
            TEST_CHECK(match_package(env, l, *pkg1, { }));

            PackageDepSpec m(parse_user_package_dep_spec("cat/pkg1[.HITCHHIKER?]", &env, { }));
            TEST_CHECK(match_package(env, m, *pkg1, { }));

            PackageDepSpec n(parse_user_package_dep_spec("cat/pkg1[.SPOON?]", &env, { }));
            TEST_CHECK(! match_package(env, n, *pkg1, { }));

            PackageDepSpec o(parse_user_package_dep_spec("cat/pkg1[.$keywords<~a]", &env, { }));
            TEST_CHECK(match_package(env, o, *pkg1, { }));
            TEST_CHECK(match_package(env, o, *pkg2, { }));
            TEST_CHECK(! match_package(env, o, *pkg3, { }));

            PackageDepSpec p(parse_user_package_dep_spec("cat/pkg1[.::$format=fake]", &env, { }));
            TEST_CHECK(match_package(env, p, *pkg1, { }));

            PackageDepSpec q(parse_user_package_dep_spec("cat/pkg1[.::$format=e]", &env, { }));
            TEST_CHECK(! match_package(env, q, *pkg1, { }));

            PackageDepSpec r(parse_user_package_dep_spec("cat/pkg1[.::format=fake]", &env, { }));
            TEST_CHECK(match_package(env, r, *pkg1, { }));

            PackageDepSpec s(parse_user_package_dep_spec("cat/pkg1[.::format=e]", &env, { }));
            TEST_CHECK(! match_package(env, s, *pkg1, { }));
        }
    } test_user_package_dep_spec_user_key_req;
}

