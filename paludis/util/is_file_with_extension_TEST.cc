/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser <halcy0n@gentoo.org>
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

#include <paludis/util/is_file_with_extension.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test IsFileWithExtension.
     *
     * \ingroup Test
     */
    struct IsFileWithExtensionTest : TestCase
    {
        IsFileWithExtensionTest() : TestCase("is file with extension") { }

        void run()
        {
            IsFileWithExtension a("foo");
            IsFileWithExtension b("goat");

            FSEntry c("teh.foo");
            FSEntry d("is_file_with_extension_TEST_file.goat");

            TEST_CHECK(d.exists());

            TEST_CHECK( !a(c) );
            TEST_CHECK( !a(d) );
            TEST_CHECK( !b(c) );
            TEST_CHECK( b(d) );

        }
    } test_is_file_with_extension;

    /**
     * \test Test IsFileWithExtension with a prefix.
     *
     * \ingroup Test
     */
    struct IsFileWithExtensionPrefixTest : TestCase
    {
        IsFileWithExtensionPrefixTest() : TestCase("is file with extension (with prefix)") { }

        void run()
        {
            IsFileWithExtension a("teh","foo");
            IsFileWithExtension b("is", "goat");
            IsFileWithExtension c("with", "goat");

            FSEntry d("teh.foo");
            FSEntry e("is_file_with_extension_TEST_file.goat");

            TEST_CHECK(e.exists());

            TEST_CHECK( !a(d) );
            TEST_CHECK( !a(e) );
            TEST_CHECK( !b(d) );
            TEST_CHECK( b(e) );
            TEST_CHECK( !c(e) );
        }
    } test_is_file_with_extension_prefix;


}

