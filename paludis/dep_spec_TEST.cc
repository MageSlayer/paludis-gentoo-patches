/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <test/test_framework.hh>
#include <test/test_runner.hh>

/** \file
 * Test cases for dep_spec.hh classes.
 *
 */

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test DepSpec as_ functions.
     *
     */
    struct DepSpecAsTest : TestCase
    {
        DepSpecAsTest() : TestCase("dep spec as") { }

        void run()
        {
            std::tr1::shared_ptr<PackageDepSpec> x(new PackageDepSpec("foo/bar"));
            TEST_CHECK(0 == x->as_use_dep_spec());

            std::tr1::shared_ptr<UseDepSpec> y(new UseDepSpec(UseFlagName("foo"), x));
            TEST_CHECK(0 != y->as_use_dep_spec());
            TEST_CHECK(y.get() == y->as_use_dep_spec());
        }
    } test_dep_spec_as;

    /**
     * \test Test DepSpec composite functions.
     *
     */
    struct DepSpecCompositeTest : TestCase
    {
        DepSpecCompositeTest() : TestCase("dep spec composite") { }

        void run()
        {
            std::tr1::shared_ptr<AllDepSpec> x(new AllDepSpec);
            TEST_CHECK(x->begin() == x->end());

            x->add_child(std::tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("x/y")));
            TEST_CHECK(x->begin() != x->end());
            TEST_CHECK_EQUAL(1, std::distance(x->begin(), x->end()));

            x->add_child(std::tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("x/y")));
            TEST_CHECK(x->begin() != x->end());
            TEST_CHECK_EQUAL(2, std::distance(x->begin(), x->end()));
        }
    } test_dep_spec_composite;

    /**
     * \test Test PackageDepSpec.
     *
     */
    struct PackageDepSpecTest : TestCase
    {
        PackageDepSpecTest() : TestCase("package dep spec") { }

        void run()
        {
            PackageDepSpec a("foo/bar");
            TEST_CHECK_STRINGIFY_EQUAL(a.package(), "foo/bar");
            TEST_CHECK(! a.slot_ptr());
            TEST_CHECK(! a.version_requirements_ptr());

            PackageDepSpec b(">=foo/bar-1.2.3");
            TEST_CHECK_STRINGIFY_EQUAL(b.package(), "foo/bar");
            TEST_CHECK(! b.slot_ptr());
            TEST_CHECK(b.version_requirements_ptr());
            TEST_CHECK_EQUAL(std::distance(b.version_requirements_ptr()->begin(),
                        b.version_requirements_ptr()->end()), 1);
            TEST_CHECK_STRINGIFY_EQUAL(b.version_requirements_ptr()->begin()->version_spec, "1.2.3");
            TEST_CHECK_EQUAL(b.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            PackageDepSpec c("foo/bar:baz");
            TEST_CHECK_STRINGIFY_EQUAL(c.package(), "foo/bar");
            TEST_CHECK(c.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*c.slot_ptr(), "baz");
            TEST_CHECK(! c.version_requirements_ptr());

            PackageDepSpec d("=foo/bar-1.2*:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(d.package(), "foo/bar");
            TEST_CHECK(d.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*d.slot_ptr(), "1.2.1");
            TEST_CHECK(d.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(d.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(d.version_requirements_ptr()->begin()->version_operator, vo_equal_star);

            PackageDepSpec e("foo/bar:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(e.package(), "foo/bar");
            TEST_CHECK(e.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*e.slot_ptr(), "1.2.1");
            TEST_CHECK(! e.version_requirements_ptr());

            PackageDepSpec f("foo/bar:0");
            TEST_CHECK_STRINGIFY_EQUAL(f.package(), "foo/bar");
            TEST_CHECK(f.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*f.slot_ptr(), "0");
            TEST_CHECK(! f.version_requirements_ptr());

            PackageDepSpec g("foo/bar-100dpi");
            TEST_CHECK_STRINGIFY_EQUAL(g.package(), "foo/bar-100dpi");

            PackageDepSpec h(">=foo/bar-100dpi-1.23");
            TEST_CHECK_STRINGIFY_EQUAL(h.package(), "foo/bar-100dpi");
            TEST_CHECK(h.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(h.version_requirements_ptr()->begin()->version_spec, "1.23");
            TEST_CHECK_EQUAL(h.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            TEST_CHECK_THROWS(PackageDepSpec(""), PackageDepSpecError);

            PackageDepSpec i("foo/bar[one][-two]");
            TEST_CHECK_STRINGIFY_EQUAL(i.package(), "foo/bar");
            TEST_CHECK(! i.version_requirements_ptr());
            TEST_CHECK(! i.repository_ptr());
            TEST_CHECK(! i.slot_ptr());
            TEST_CHECK(i.use_requirements_ptr());
            TEST_CHECK(i.use_requirements_ptr()->find(UseFlagName("one")) !=
                    i.use_requirements_ptr()->end());
            TEST_CHECK(i.use_requirements_ptr()->find(UseFlagName("two")) !=
                    i.use_requirements_ptr()->end());
            TEST_CHECK(i.use_requirements_ptr()->find(UseFlagName("three")) ==
                    i.use_requirements_ptr()->end());
            TEST_CHECK(i.use_requirements_ptr()->state(UseFlagName("one")) == use_enabled);
            TEST_CHECK(i.use_requirements_ptr()->state(UseFlagName("two")) == use_disabled);
            TEST_CHECK(i.use_requirements_ptr()->state(UseFlagName("moo")) == use_unspecified);

            PackageDepSpec j("=foo/bar-scm-r3");
            TEST_CHECK_STRINGIFY_EQUAL(j.package(), "foo/bar");
            TEST_CHECK(j.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(j.version_requirements_ptr()->begin()->version_spec, "scm-r3");
            TEST_CHECK_EQUAL(j.version_requirements_ptr()->begin()->version_operator, vo_equal);

            PackageDepSpec k("=foo/bar-scm");
            TEST_CHECK_STRINGIFY_EQUAL(k.package(), "foo/bar");
            TEST_CHECK(k.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(k.version_requirements_ptr()->begin()->version_spec, "scm");
            TEST_CHECK_EQUAL(k.version_requirements_ptr()->begin()->version_operator, vo_equal);
        }
    } test_package_dep_spec;
}

