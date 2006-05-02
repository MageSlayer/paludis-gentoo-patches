/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_atom.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

/** \file
 * Test cases for dep_atom.hh classes.
 *
 * \ingroup grptestcases
 */

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test DepAtom as_ functions.
     *
     * \ingroup grptestcases
     */
    struct DepAtomAsTest : TestCase
    {
        DepAtomAsTest() : TestCase("dep atom as") { }

        void run()
        {
            PackageDepAtom::Pointer x(new PackageDepAtom("foo/bar"));
            TEST_CHECK(0 == x->as_use_dep_atom());

            UseDepAtom::Pointer y(new UseDepAtom(UseFlagName("foo"), x));
            TEST_CHECK(0 != y->as_use_dep_atom());
            TEST_CHECK(y.raw_pointer() == y->as_use_dep_atom());
        }
    } test_dep_atom_as;

    /**
     * \test Test DepAtom composite functions.
     *
     * \ingroup grptestcases
     */
    struct DepAtomCompositeTest : TestCase
    {
        DepAtomCompositeTest() : TestCase("dep atom composite") { }

        void run()
        {
            AllDepAtom::Pointer x(new AllDepAtom);
            TEST_CHECK(x->begin() == x->end());

            x->add_child(PackageDepAtom::Pointer(new PackageDepAtom("x/y")));
            TEST_CHECK(x->begin() != x->end());
            TEST_CHECK_EQUAL(1, std::distance(x->begin(), x->end()));

            x->add_child(PackageDepAtom::Pointer(new PackageDepAtom("x/y")));
            TEST_CHECK(x->begin() != x->end());
            TEST_CHECK_EQUAL(2, std::distance(x->begin(), x->end()));
        }
    } test_dep_atom_composite;

    /**
     * \test Test PackageDepAtom.
     *
     * \ingroup grptestcases
     */
    struct PackageDepAtomTest : TestCase
    {
        PackageDepAtomTest() : TestCase("package dep atom") { }

        void run()
        {
            PackageDepAtom a("foo/bar");
            TEST_CHECK_STRINGIFY_EQUAL(a.package(), "foo/bar");
            TEST_CHECK(! a.slot_ptr());
            TEST_CHECK(! a.version_spec_ptr());

            PackageDepAtom b(">=foo/bar-1.2.3");
            TEST_CHECK_STRINGIFY_EQUAL(b.package(), "foo/bar");
            TEST_CHECK(! b.slot_ptr());
            TEST_CHECK(b.version_spec_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*b.version_spec_ptr(), "1.2.3");
            TEST_CHECK_EQUAL(b.version_operator(), vo_greater_equal);

            PackageDepAtom c("foo/bar:baz");
            TEST_CHECK_STRINGIFY_EQUAL(c.package(), "foo/bar");
            TEST_CHECK(c.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*c.slot_ptr(), "baz");
            TEST_CHECK(! c.version_spec_ptr());

            PackageDepAtom d("=foo/bar-1.2*:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(d.package(), "foo/bar");
            TEST_CHECK(d.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*d.slot_ptr(), "1.2.1");
            TEST_CHECK(d.version_spec_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*d.version_spec_ptr(), "1.2");
            TEST_CHECK_EQUAL(d.version_operator(), vo_equal_star);

            PackageDepAtom e("foo/bar:1.2.1");
            TEST_CHECK_STRINGIFY_EQUAL(e.package(), "foo/bar");
            TEST_CHECK(e.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*e.slot_ptr(), "1.2.1");
            TEST_CHECK(! e.version_spec_ptr());

            PackageDepAtom f("foo/bar:0");
            TEST_CHECK_STRINGIFY_EQUAL(f.package(), "foo/bar");
            TEST_CHECK(f.slot_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*f.slot_ptr(), "0");
            TEST_CHECK(! f.version_spec_ptr());

            PackageDepAtom g("foo/bar-100dpi");
            TEST_CHECK_STRINGIFY_EQUAL(g.package(), "foo/bar-100dpi");

            PackageDepAtom h(">=foo/bar-100dpi-1.23");
            TEST_CHECK_STRINGIFY_EQUAL(h.package(), "foo/bar-100dpi");
            TEST_CHECK(h.version_spec_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*h.version_spec_ptr(), "1.23");
            TEST_CHECK_EQUAL(h.version_operator(), vo_greater_equal);

            TEST_CHECK_THROWS(PackageDepAtom(""), PackageDepAtomError);

            PackageDepAtom i("foo/bar[one][-two]");
            TEST_CHECK_STRINGIFY_EQUAL(i.package(), "foo/bar");
            TEST_CHECK(! i.version_spec_ptr());
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

            PackageDepAtom j("=foo/bar-scm-r3");
            TEST_CHECK_STRINGIFY_EQUAL(j.package(), "foo/bar");
            TEST_CHECK(j.version_spec_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*j.version_spec_ptr(), "scm-r3");
            TEST_CHECK_EQUAL(j.version_operator(), vo_equal);

            PackageDepAtom k("=foo/bar-scm");
            TEST_CHECK_STRINGIFY_EQUAL(k.package(), "foo/bar");
            TEST_CHECK(k.version_spec_ptr());
            TEST_CHECK_STRINGIFY_EQUAL(*k.version_spec_ptr(), "scm");
            TEST_CHECK_EQUAL(k.version_operator(), vo_equal);
        }
    } test_package_dep_atom;
}

