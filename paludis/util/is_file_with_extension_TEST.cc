/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser
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

#include <algorithm>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/options.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

/** \file
 * Test cases for IsFileWithExtension.
 *
 */

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test IsFileWithExtension.
     *
     */
    struct IsFileWithExtensionTest : TestCase
    {
        IsFileWithExtensionTest() : TestCase("is file with extension") { }

        void run()
        {
            FSPath c("teh.foo");
            FSPath d("is_file_with_extension_TEST_file.goat");

            TEST_CHECK(d.stat().exists());

            TEST_CHECK(! is_file_with_extension(c, "foo", { }));
            TEST_CHECK(! is_file_with_extension(d, "foo", { }));
            TEST_CHECK(! is_file_with_extension(c, "goat", { }));
            TEST_CHECK(is_file_with_extension(d, "goat", { }));

        }
    } test_is_file_with_extension;

    /**
     * \test Test IsFileWithExtension with a prefix.
     *
     */
    struct IsFileWithExtensionPrefixTest : TestCase
    {
        IsFileWithExtensionPrefixTest() : TestCase("is file with extension (with prefix)") { }

        void run()
        {
            FSPath d("teh.foo");
            FSPath e("is_file_with_extension_TEST_file.goat");

            TEST_CHECK(e.stat().exists());

            TEST_CHECK(! is_file_with_prefix_extension(d, "teh", "foo", { }));
            TEST_CHECK(! is_file_with_prefix_extension(e, "teh", "foo", { }));
            TEST_CHECK(! is_file_with_prefix_extension(d, "is", "goat", { }));
            TEST_CHECK(is_file_with_prefix_extension(e, "is", "goat", { }));
            TEST_CHECK(! is_file_with_prefix_extension(e, "with", "goat", { }));
        }
    } test_is_file_with_extension_prefix;


}

