/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/clone-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/version_requirements.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct UserPackageDepSpecTest : TestCase
    {
        UserPackageDepSpecTest() : TestCase("user package dep spec") { }

        void run()
        {
            TestEnvironment env;

            PackageDepSpec a(parse_user_package_dep_spec("foo/bar", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(a, "foo/bar");
            TEST_CHECK_STRINGIFY_EQUAL(*a.package_ptr(), "foo/bar");
            TEST_CHECK(! a.slot_requirement_ptr());
            TEST_CHECK(! a.version_requirements_ptr());

            PackageDepSpec b(parse_user_package_dep_spec(">=foo/bar-1.2.3", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(b, ">=foo/bar-1.2.3");
            TEST_CHECK_STRINGIFY_EQUAL(*b.package_ptr(), "foo/bar");
            TEST_CHECK(! b.slot_requirement_ptr());
            TEST_CHECK(b.version_requirements_ptr());
            TEST_CHECK_EQUAL(std::distance(b.version_requirements_ptr()->begin(),
                        b.version_requirements_ptr()->end()), 1);
            TEST_CHECK_STRINGIFY_EQUAL(b.version_requirements_ptr()->begin()->version_spec, "1.2.3");
            TEST_CHECK_EQUAL(b.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            PackageDepSpec c(parse_user_package_dep_spec("foo/bar:baz", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(c, "foo/bar:baz");
            TEST_CHECK_STRINGIFY_EQUAL(*c.package_ptr(), "foo/bar");
            TEST_CHECK(c.slot_requirement_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*c.slot_requirement_ptr(), ":baz");
            TEST_CHECK(! c.version_requirements_ptr());

            PackageDepSpec d(parse_user_package_dep_spec("=foo/bar-1.2*:1.2.1", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(d, "=foo/bar-1.2*:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(*d.package_ptr(), "foo/bar");
            TEST_CHECK(d.slot_requirement_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*d.slot_requirement_ptr(), ":1.2.1");
            TEST_CHECK(d.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(d.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(d.version_requirements_ptr()->begin()->version_operator, vo_equal_star);

            PackageDepSpec e(parse_user_package_dep_spec("foo/bar:1.2.1", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(e, "foo/bar:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(*e.package_ptr(), "foo/bar");
            TEST_CHECK(e.slot_requirement_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*e.slot_requirement_ptr(), ":1.2.1");
            TEST_CHECK(! e.version_requirements_ptr());

            PackageDepSpec f(parse_user_package_dep_spec("foo/bar:0", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(f, "foo/bar:0");
            TEST_CHECK_STRINGIFY_EQUAL(*f.package_ptr(), "foo/bar");
            TEST_CHECK(f.slot_requirement_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*f.slot_requirement_ptr(), ":0");
            TEST_CHECK(! f.version_requirements_ptr());

            PackageDepSpec g(parse_user_package_dep_spec("foo/bar-100dpi", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(g, "foo/bar-100dpi");
            TEST_CHECK_STRINGIFY_EQUAL(*g.package_ptr(), "foo/bar-100dpi");

            PackageDepSpec h(parse_user_package_dep_spec(">=foo/bar-100dpi-1.23", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(h, ">=foo/bar-100dpi-1.23");
            TEST_CHECK_STRINGIFY_EQUAL(*h.package_ptr(), "foo/bar-100dpi");
            TEST_CHECK(h.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(h.version_requirements_ptr()->begin()->version_spec, "1.23");
            TEST_CHECK_EQUAL(h.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("", &env, UserPackageDepSpecOptions()), PackageDepSpecError);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("=foo/bar-1.2[=1.3]", &env, UserPackageDepSpecOptions()), PackageDepSpecError);

            PackageDepSpec i(parse_user_package_dep_spec("foo/bar[one][-two]", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(i, "foo/bar[-two][one]");
            TEST_CHECK_STRINGIFY_EQUAL(*i.package_ptr(), "foo/bar");
            TEST_CHECK(! i.version_requirements_ptr());
            TEST_CHECK(! i.repository_ptr());
            TEST_CHECK(! i.slot_requirement_ptr());
            TEST_CHECK(i.additional_requirements_ptr());

            PackageDepSpec j(parse_user_package_dep_spec("=foo/bar-scm-r3", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(j, "=foo/bar-scm-r3");
            TEST_CHECK_STRINGIFY_EQUAL(*j.package_ptr(), "foo/bar");
            TEST_CHECK(j.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(j.version_requirements_ptr()->begin()->version_spec, "scm-r3");
            TEST_CHECK_EQUAL(j.version_requirements_ptr()->begin()->version_operator, vo_equal);

            PackageDepSpec k(parse_user_package_dep_spec("=foo/bar-scm", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(k, "=foo/bar-scm");
            TEST_CHECK_STRINGIFY_EQUAL(*k.package_ptr(), "foo/bar");
            TEST_CHECK(k.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(k.version_requirements_ptr()->begin()->version_spec, "scm");
            TEST_CHECK_EQUAL(k.version_requirements_ptr()->begin()->version_operator, vo_equal);

            PackageDepSpec l(parse_user_package_dep_spec("foo/bar[one][-two][>=1.2&<2.0]", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(l, "foo/bar[>=1.2&<2.0][-two][one]");
            TEST_CHECK_STRINGIFY_EQUAL(*l.package_ptr(), "foo/bar");
            TEST_CHECK(l.version_requirements_ptr());
            TEST_CHECK(! l.repository_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(l.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(l.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);
            TEST_CHECK_STRINGIFY_EQUAL(next(l.version_requirements_ptr()->begin())->version_spec, "2.0");
            TEST_CHECK_EQUAL(next(l.version_requirements_ptr()->begin())->version_operator, vo_less);
            TEST_CHECK(! l.slot_requirement_ptr());

            PackageDepSpec m(parse_user_package_dep_spec("foo/bar[=1.2|=1.3*|~1.4]", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(m, "foo/bar[=1.2|=1.3*|~1.4]");
            TEST_CHECK_STRINGIFY_EQUAL(*m.package_ptr(), "foo/bar");
            TEST_CHECK(m.version_requirements_ptr());
            TEST_CHECK(! m.repository_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(m.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(m.version_requirements_ptr()->begin()->version_operator, vo_equal);
            TEST_CHECK_STRINGIFY_EQUAL(next(m.version_requirements_ptr()->begin())->version_spec, "1.3");
            TEST_CHECK_EQUAL(next(m.version_requirements_ptr()->begin())->version_operator, vo_equal_star);
            TEST_CHECK_STRINGIFY_EQUAL(next(next(m.version_requirements_ptr()->begin()))->version_spec, "1.4");
            TEST_CHECK_EQUAL(next(next(m.version_requirements_ptr()->begin()))->version_operator, vo_tilde);
            TEST_CHECK(! m.slot_requirement_ptr());
        }
    } test_user_package_dep_spec;

    struct UserPackageDepSpecUnspecificTest : TestCase
    {
        UserPackageDepSpecUnspecificTest() : TestCase("user package dep spec unspecific") { }

        void run()
        {
            TestEnvironment env;

            PackageDepSpec a(parse_user_package_dep_spec("*/*", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(a, "*/*");
            TEST_CHECK(! a.package_ptr());
            TEST_CHECK(! a.package_name_part_ptr());
            TEST_CHECK(! a.category_name_part_ptr());

            PackageDepSpec b(parse_user_package_dep_spec("foo/*", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(b, "foo/*");
            TEST_CHECK(! b.package_ptr());
            TEST_CHECK(! b.package_name_part_ptr());
            TEST_CHECK(b.category_name_part_ptr());
            TEST_CHECK_EQUAL(*b.category_name_part_ptr(), CategoryNamePart("foo"));

            PackageDepSpec c(parse_user_package_dep_spec("*/foo", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(c, "*/foo");
            TEST_CHECK(! c.package_ptr());
            TEST_CHECK(c.package_name_part_ptr());
            TEST_CHECK_EQUAL(*c.package_name_part_ptr(), PackageNamePart("foo"));
            TEST_CHECK(! c.category_name_part_ptr());

            PackageDepSpec d(parse_user_package_dep_spec("~*/*-0", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(d, "~*/*-0");
            TEST_CHECK(! d.package_ptr());
            TEST_CHECK(! d.package_name_part_ptr());
            TEST_CHECK(! d.category_name_part_ptr());

            PackageDepSpec e(parse_user_package_dep_spec(">=foo/*-1.23", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(e, ">=foo/*-1.23");
            TEST_CHECK(! e.package_ptr());
            TEST_CHECK(! e.package_name_part_ptr());
            TEST_CHECK(e.category_name_part_ptr());
            TEST_CHECK_EQUAL(*e.category_name_part_ptr(), CategoryNamePart("foo"));

            PackageDepSpec f(parse_user_package_dep_spec("=*/foo-1*", &env, UserPackageDepSpecOptions() + updso_allow_wildcards));
            TEST_CHECK_STRINGIFY_EQUAL(f, "=*/foo-1*");
            TEST_CHECK(! f.package_ptr());
            TEST_CHECK(f.package_name_part_ptr());
            TEST_CHECK_EQUAL(*f.package_name_part_ptr(), PackageNamePart("foo"));
            TEST_CHECK(! f.category_name_part_ptr());
        }
    } test_user_package_dep_spec_unspecific;

    struct UserPackageDepSpecDisambiguationTest : TestCase
    {
        UserPackageDepSpecDisambiguationTest() : TestCase("user package dep spec disambiguation") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> fake(new FakeRepository(&env, RepositoryName("fake")));
            std::tr1::shared_ptr<FakeInstalledRepository> fake_inst(new FakeInstalledRepository(&env, RepositoryName("fake_inst")));
            env.package_database()->add_repository(1, fake);
            env.package_database()->add_repository(2, fake_inst);
            fake->add_version("cat", "pkg1", "1");
            fake->add_version("cat", "pkg2", "1");
            fake->add_version("dog", "pkg2", "1");
            fake->add_version("cat", "pkg3", "1");
            fake_inst->add_version("dog", "pkg3", "1");

            PackageDepSpec a(parse_user_package_dep_spec("pkg1", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(a, "cat/pkg1");
            TEST_CHECK_STRINGIFY_EQUAL(*a.package_ptr(), "cat/pkg1");
            TEST_CHECK(! a.package_name_part_ptr());
            TEST_CHECK(! a.category_name_part_ptr());

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg2", &env, UserPackageDepSpecOptions()), AmbiguousPackageNameError);
            PackageDepSpec b(parse_user_package_dep_spec("pkg3", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(b, "dog/pkg3");
            TEST_CHECK_STRINGIFY_EQUAL(*b.package_ptr(), "dog/pkg3");
            TEST_CHECK(! b.package_name_part_ptr());
            TEST_CHECK(! b.category_name_part_ptr());
            PackageDepSpec c(parse_user_package_dep_spec("pkg3", &env, UserPackageDepSpecOptions(), filter::SupportsAction<InstallAction>()));
            TEST_CHECK_STRINGIFY_EQUAL(c, "cat/pkg3");
            TEST_CHECK_STRINGIFY_EQUAL(*c.package_ptr(), "cat/pkg3");
            TEST_CHECK(! c.package_name_part_ptr());
            TEST_CHECK(! c.category_name_part_ptr());

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg4", &env, UserPackageDepSpecOptions()), NoSuchPackageError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("pkg5", &env, UserPackageDepSpecOptions() + updso_no_disambiguation), PackageDepSpecError);

            PackageDepSpec d(parse_user_package_dep_spec("=pkg1-42", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(d, "=cat/pkg1-42");
            PackageDepSpec e(parse_user_package_dep_spec("=pkg1-42:0", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(e, "=cat/pkg1-42:0");
            PackageDepSpec f(parse_user_package_dep_spec("pkg1:0", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(f, "cat/pkg1:0");
            PackageDepSpec g(parse_user_package_dep_spec("pkg1[foo]", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(g, "cat/pkg1[foo]");
            PackageDepSpec h(parse_user_package_dep_spec("pkg1[=42]", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(h, "=cat/pkg1-42");
            PackageDepSpec i(parse_user_package_dep_spec("pkg1::fake", &env, UserPackageDepSpecOptions()));
            TEST_CHECK_STRINGIFY_EQUAL(i, "cat/pkg1::fake");
        }
    } test_user_package_dep_spec_disambiguation;

    struct UserPackageDepSpecSetsTest : TestCase
    {
        UserPackageDepSpecSetsTest() : TestCase("user package dep spec sets") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> fake(new FakeRepository(&env, RepositoryName("fake")));
            env.package_database()->add_repository(1, fake);
            fake->add_version("cat", "world", "1");
            fake->add_version("cat", "moon", "1");

            TEST_CHECK_THROWS(parse_user_package_dep_spec("world", &env, UserPackageDepSpecOptions() + updso_throw_if_set), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, UserPackageDepSpecOptions() + updso_throw_if_set), GotASetNotAPackageDepSpec);
            PackageDepSpec a(parse_user_package_dep_spec("moon", &env, UserPackageDepSpecOptions() + updso_throw_if_set));
            TEST_CHECK_STRINGIFY_EQUAL(a, "cat/moon");
            TEST_CHECK_THROWS(parse_user_package_dep_spec("mars", &env, UserPackageDepSpecOptions() + updso_throw_if_set), NoSuchPackageError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("world", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("moon", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("mars", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), GotASetNotAPackageDepSpec);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("=world-123", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), PackageDepSpecError);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("world*", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), GotASetNotAPackageDepSpec);
            TEST_CHECK_THROWS(parse_user_package_dep_spec("world**", &env, UserPackageDepSpecOptions() + updso_no_disambiguation + updso_throw_if_set), PackageDepSpecError);

            TEST_CHECK_THROWS(parse_user_package_dep_spec("system", &env, UserPackageDepSpecOptions()), NoSuchPackageError);
        }
    } test_user_package_dep_spec_sets;
}

