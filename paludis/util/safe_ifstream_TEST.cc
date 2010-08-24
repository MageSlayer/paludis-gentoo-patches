/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_path.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <unistd.h>
#include <sys/types.h>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct ExistingTest : TestCase
    {
        ExistingTest() : TestCase("existing file") { }

        void run()
        {
            SafeIFStream s(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing");
            TEST_CHECK(s);
            std::string t;
            s >> t;
            TEST_CHECK(s);
            TEST_CHECK_EQUAL(t, "first");
            s >> t;
            TEST_CHECK_EQUAL(t, std::string(1000, 'x'));

            s.clear();
            s.seekg(0, std::ios::beg);

            TEST_CHECK(s);
            s >> t;
            TEST_CHECK(s);
            TEST_CHECK_EQUAL(t, "first");
            s >> t;
            TEST_CHECK_EQUAL(t, std::string(1000, 'x'));
        }
    } test_existing;

    struct ExistingSymTest : TestCase
    {
        ExistingSymTest() : TestCase("existing sym") { }

        void run()
        {
            SafeIFStream s(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing");
            TEST_CHECK(s);
            std::string t;
            s >> t;
            TEST_CHECK(s);
            TEST_CHECK_EQUAL(t, "first");
            s >> t;
            TEST_CHECK_EQUAL(t, std::string(1000, 'x'));
        }
    } test_existing_sym;

    struct ExistingDirTest : TestCase
    {
        ExistingDirTest() : TestCase("existing dir") { }

        void run()
        {
            TEST_CHECK_THROWS(SafeIFStream(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing_dir"), SafeIFStreamError);
        }
    } test_existing_dir;

    struct ExistingPermTest : TestCase
    {
        ExistingPermTest() : TestCase("existing unreadable file") { }

        bool skip() const
        {
            return 0 == getuid();
        }

        void run()
        {
            TEST_CHECK_THROWS(SafeIFStream(FSPath::cwd() / "safe_ifstream_TEST_dir" / "existing_perm"), SafeIFStreamError);
        }
    } test_existing_perm;

    struct ExistingNoentTest : TestCase
    {
        ExistingNoentTest() : TestCase("no such file") { }

        void run()
        {
            TEST_CHECK_THROWS(SafeIFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "noent"), SafeIFStreamError);
        }
    } test_existing_noent;
}

