/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/util/realpath.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>

#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct RealpathWithCurrentAndRootTest : TestCase
    {
        RealpathWithCurrentAndRootTest() : TestCase("realpath_with_current_and_root") {}

        FSPath resolve(const FSPath & symlink, const FSPath & root)
        {
            return realpath_with_current_and_root(FSPath((root / symlink).readlink()), symlink.dirname(), root);
        }

        void run()
        {
            FSPath root("realpath_TEST_dir");

            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libfoo.so"), root), "/usr/lib64/libfoo.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libbar.so"), root), "/usr/lib64/libbar.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libbaz.so"), root), "/usr/lib64/libbaz.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libxyzzy.so"), root), "/usr/lib64/libxyzzy.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libplugh.so"), root), "/usr/lib64/libplugh.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libplover.so"), root), "/usr/lib64/libplover.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libblast.so"), root), "/usr/lib64/libblast.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libquux.so"), root), "/usr/lib64/libquux.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libnarf.so"), root), "/usr/lib64/libnarf.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libblech.so"), root), "/usr/lib64/libblech.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libstab.so"), root), "/usr/lib64/libstab.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libsnark.so"), root), "/usr/lib64/libsnark.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libfool.so"), root), "/usr/lib32/libfool.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libbarf.so"), root), "/usr/lib64/barf/libbarf.so.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libblip.so"), root), "/usr/lib64/libblip.so.1.0.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/libpoing.so"), root), "/usr/lib64/libpoing.so.1.0.1");
            TEST_CHECK_STRINGIFY_EQUAL(resolve(FSPath("/usr/lib64/x/liby.so"), root), "/usr/lib64/x/liby.so.1");

            TEST_CHECK_THROWS(resolve(FSPath("/usr/lib64/libouch.so"), root), FSError);
            TEST_CHECK_THROWS(resolve(FSPath("/usr/lib64/libping.so"), root), FSError);
        }
    } realpath_with_current_and_root_test;
}

