/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "package_dep_atom.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test PackageDepAtom.
     *
     * \ingroup Test
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
        }
    } test_package_dep_atom;
}

