/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/util/iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/version_requirements.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
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
            tr1::shared_ptr<PackageDepSpec> x(new PackageDepSpec("foo/bar", pds_pm_permissive));
            TEST_CHECK(0 == x->as_use_dep_spec());

            tr1::shared_ptr<UseDepSpec> y(new UseDepSpec(UseFlagName("foo"), x));
            TEST_CHECK(0 != y->as_use_dep_spec());
            TEST_CHECK(y.get() == y->as_use_dep_spec());
        }
    } test_dep_spec_as;

    /**
     * \test Test PackageDepSpec.
     *
     */
    struct PackageDepSpecTest : TestCase
    {
        PackageDepSpecTest() : TestCase("package dep spec") { }

        void run()
        {
            PackageDepSpec a("foo/bar", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(a, "foo/bar");
            TEST_CHECK_STRINGIFY_EQUAL(*a.package_ptr(), "foo/bar");
            TEST_CHECK(! a.slot_ptr());
            TEST_CHECK(! a.version_requirements_ptr());

            PackageDepSpec b(">=foo/bar-1.2.3", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(b, ">=foo/bar-1.2.3");
            TEST_CHECK_STRINGIFY_EQUAL(*b.package_ptr(), "foo/bar");
            TEST_CHECK(! b.slot_ptr());
            TEST_CHECK(b.version_requirements_ptr());
            TEST_CHECK_EQUAL(std::distance(b.version_requirements_ptr()->begin(),
                        b.version_requirements_ptr()->end()), 1);
            TEST_CHECK_STRINGIFY_EQUAL(b.version_requirements_ptr()->begin()->version_spec, "1.2.3");
            TEST_CHECK_EQUAL(b.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            PackageDepSpec c("foo/bar:baz", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(c, "foo/bar:baz");
            TEST_CHECK_STRINGIFY_EQUAL(*c.package_ptr(), "foo/bar");
            TEST_CHECK(c.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*c.slot_ptr(), "baz");
            TEST_CHECK(! c.version_requirements_ptr());

            PackageDepSpec d("=foo/bar-1.2*:1.2.1", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(d, "=foo/bar-1.2*:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(*d.package_ptr(), "foo/bar");
            TEST_CHECK(d.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*d.slot_ptr(), "1.2.1");
            TEST_CHECK(d.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(d.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(d.version_requirements_ptr()->begin()->version_operator, vo_equal_star);

            PackageDepSpec e("foo/bar:1.2.1", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(e, "foo/bar:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(*e.package_ptr(), "foo/bar");
            TEST_CHECK(e.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*e.slot_ptr(), "1.2.1");
            TEST_CHECK(! e.version_requirements_ptr());

            PackageDepSpec f("foo/bar:0", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(f, "foo/bar:0");
            TEST_CHECK_STRINGIFY_EQUAL(*f.package_ptr(), "foo/bar");
            TEST_CHECK(f.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*f.slot_ptr(), "0");
            TEST_CHECK(! f.version_requirements_ptr());

            PackageDepSpec g("foo/bar-100dpi", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(g, "foo/bar-100dpi");
            TEST_CHECK_STRINGIFY_EQUAL(*g.package_ptr(), "foo/bar-100dpi");

            PackageDepSpec h(">=foo/bar-100dpi-1.23", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(h, ">=foo/bar-100dpi-1.23");
            TEST_CHECK_STRINGIFY_EQUAL(*h.package_ptr(), "foo/bar-100dpi");
            TEST_CHECK(h.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(h.version_requirements_ptr()->begin()->version_spec, "1.23");
            TEST_CHECK_EQUAL(h.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);

            TEST_CHECK_THROWS(PackageDepSpec("", pds_pm_permissive), PackageDepSpecError);
            TEST_CHECK_THROWS(PackageDepSpec("=foo/bar-1.2[=1.3]", pds_pm_permissive), PackageDepSpecError);

            PackageDepSpec i("foo/bar[one][-two]", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(i, "foo/bar[one][-two]");
            TEST_CHECK_STRINGIFY_EQUAL(*i.package_ptr(), "foo/bar");
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

            PackageDepSpec j("=foo/bar-scm-r3", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(j, "=foo/bar-scm-r3");
            TEST_CHECK_STRINGIFY_EQUAL(*j.package_ptr(), "foo/bar");
            TEST_CHECK(j.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(j.version_requirements_ptr()->begin()->version_spec, "scm-r3");
            TEST_CHECK_EQUAL(j.version_requirements_ptr()->begin()->version_operator, vo_equal);

            PackageDepSpec k("=foo/bar-scm", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(k, "=foo/bar-scm");
            TEST_CHECK_STRINGIFY_EQUAL(*k.package_ptr(), "foo/bar");
            TEST_CHECK(k.version_requirements_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(k.version_requirements_ptr()->begin()->version_spec, "scm");
            TEST_CHECK_EQUAL(k.version_requirements_ptr()->begin()->version_operator, vo_equal);

            PackageDepSpec l("foo/bar[one][-two][>=1.2&<2.0]", pds_pm_permissive);
            TEST_CHECK_STRINGIFY_EQUAL(l, "foo/bar[>=1.2&<2.0][one][-two]");
            TEST_CHECK_STRINGIFY_EQUAL(*l.package_ptr(), "foo/bar");
            TEST_CHECK(l.version_requirements_ptr());
            TEST_CHECK(! l.repository_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(l.version_requirements_ptr()->begin()->version_spec, "1.2");
            TEST_CHECK_EQUAL(l.version_requirements_ptr()->begin()->version_operator, vo_greater_equal);
            TEST_CHECK_STRINGIFY_EQUAL(next(l.version_requirements_ptr()->begin())->version_spec, "2.0");
            TEST_CHECK_EQUAL(next(l.version_requirements_ptr()->begin())->version_operator, vo_less);
            TEST_CHECK(! l.slot_ptr());
            TEST_CHECK(l.use_requirements_ptr());
            TEST_CHECK(l.use_requirements_ptr()->find(UseFlagName("one")) !=
                    l.use_requirements_ptr()->end());
            TEST_CHECK(l.use_requirements_ptr()->find(UseFlagName("two")) !=
                    l.use_requirements_ptr()->end());
            TEST_CHECK(l.use_requirements_ptr()->find(UseFlagName("three")) ==
                    l.use_requirements_ptr()->end());
            TEST_CHECK(l.use_requirements_ptr()->state(UseFlagName("one")) == use_enabled);
            TEST_CHECK(l.use_requirements_ptr()->state(UseFlagName("two")) == use_disabled);
            TEST_CHECK(l.use_requirements_ptr()->state(UseFlagName("moo")) == use_unspecified);

            PackageDepSpec m("foo/bar[=1.2|=1.3*|~1.4]", pds_pm_permissive);
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
            TEST_CHECK(! m.slot_ptr());
        }
    } test_package_dep_spec;

    struct FetchableURIDepSpecTest : TestCase
    {
        FetchableURIDepSpecTest() : TestCase("fetchable uri dep spec") { }

        void run()
        {
            FetchableURIDepSpec a("foo");
            TEST_CHECK_EQUAL(a.original_url(), "foo");
            TEST_CHECK_EQUAL(a.renamed_url_suffix(), "");
            TEST_CHECK_EQUAL(a.filename(), "foo");

            FetchableURIDepSpec b("fnord -> bar");
            TEST_CHECK_EQUAL(b.original_url(), "fnord");
            TEST_CHECK_EQUAL(b.renamed_url_suffix(), "bar");
            TEST_CHECK_EQUAL(b.filename(), "bar");

            FetchableURIDepSpec c("http://example.com/download/baz");
            TEST_CHECK_EQUAL(c.filename(), "baz");
        }
    } test_fetchable_uri_dep_spec;

    struct PackageDepSpecUnspecificTest : TestCase
    {
        PackageDepSpecUnspecificTest() : TestCase("package dep spec unspecific") { }

        void run()
        {
            PackageDepSpec a("*/*", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(a, "*/*");
            TEST_CHECK(! a.package_ptr());
            TEST_CHECK(! a.package_name_part_ptr());
            TEST_CHECK(! a.category_name_part_ptr());

            PackageDepSpec b("foo/*", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(b, "foo/*");
            TEST_CHECK(! b.package_ptr());
            TEST_CHECK(! b.package_name_part_ptr());
            TEST_CHECK(b.category_name_part_ptr());
            TEST_CHECK_EQUAL(*b.category_name_part_ptr(), CategoryNamePart("foo"));

            PackageDepSpec c("*/foo", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(c, "*/foo");
            TEST_CHECK(! c.package_ptr());
            TEST_CHECK(c.package_name_part_ptr());
            TEST_CHECK_EQUAL(*c.package_name_part_ptr(), PackageNamePart("foo"));
            TEST_CHECK(! c.category_name_part_ptr());

            PackageDepSpec d("~*/*-0", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(d, "~*/*-0");
            TEST_CHECK(! d.package_ptr());
            TEST_CHECK(! d.package_name_part_ptr());
            TEST_CHECK(! d.category_name_part_ptr());

            PackageDepSpec e(">=foo/*-1.23", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(e, ">=foo/*-1.23");
            TEST_CHECK(! e.package_ptr());
            TEST_CHECK(! e.package_name_part_ptr());
            TEST_CHECK(e.category_name_part_ptr());
            TEST_CHECK_EQUAL(*e.category_name_part_ptr(), CategoryNamePart("foo"));

            PackageDepSpec f("=*/foo-1*", pds_pm_unspecific);
            TEST_CHECK_STRINGIFY_EQUAL(f, "=*/foo-1*");
            TEST_CHECK(! f.package_ptr());
            TEST_CHECK(f.package_name_part_ptr());
            TEST_CHECK_EQUAL(*f.package_name_part_ptr(), PackageNamePart("foo"));
            TEST_CHECK(! f.category_name_part_ptr());
        }
    } test_package_dep_spec_unspecific;

    struct DepSpecCloneTest : TestCase
    {
        DepSpecCloneTest() : TestCase("dep spec clone") { }

        void run()
        {
            PackageDepSpec a("cat/pkg:1::repo[=1|>3.2][foo]", pds_pm_permissive);

            tr1::shared_ptr<PackageDepSpec> b(tr1::static_pointer_cast<PackageDepSpec>(a.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(a, *b);
            b->set_version_requirements_mode(vr_and);
            TEST_CHECK(stringify(a) != stringify(*b));

            tr1::shared_ptr<PackageDepSpec> c(tr1::static_pointer_cast<PackageDepSpec>(a.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(a, *c);
            c->version_requirements_ptr()->push_back(VersionRequirement(vo_tilde, VersionSpec("1.5")));
            TEST_CHECK(stringify(a) != stringify(*c));

            BlockDepSpec d(c);
            tr1::shared_ptr<BlockDepSpec> e(tr1::static_pointer_cast<BlockDepSpec>(d.clone()));
            TEST_CHECK_STRINGIFY_EQUAL(*(d.blocked_spec()), *(e->blocked_spec()));
            c->set_version_requirements_mode(vr_and);
            TEST_CHECK(stringify(*(d.blocked_spec())) != stringify(*(e->blocked_spec())));
        }
    } test_dep_spec_clone;
}

