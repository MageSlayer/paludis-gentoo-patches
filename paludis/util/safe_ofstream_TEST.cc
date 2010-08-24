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

#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/fs_path.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <unistd.h>
#include <sys/types.h>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct NewTest : TestCase
    {
        NewTest() : TestCase("new file") { }

        void run()
        {
            SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "new");
            TEST_CHECK(s);
            s << "foo";
            TEST_CHECK(s);
        }

        bool repeatable() const
        {
            return false;
        }
    } test_new;

    struct ExistingTest : TestCase
    {
        ExistingTest() : TestCase("existing file") { }

        void run()
        {
            SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing");
            TEST_CHECK(s);
            s << "foo";
            TEST_CHECK(s);
        }

        bool repeatable() const
        {
            return false;
        }
    } test_existing;

    struct ExistingSymTest : TestCase
    {
        ExistingSymTest() : TestCase("existing sym") { }

        void run()
        {
            SafeOFStream s(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_sym");
            TEST_CHECK(s);
            s << "foo";
            TEST_CHECK(s);
        }

        bool repeatable() const
        {
            return false;
        }
    } test_existing_sym;

    struct ExistingDirTest : TestCase
    {
        ExistingDirTest() : TestCase("existing dir") { }

        void run()
        {
            TEST_CHECK_THROWS(SafeOFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_dir"), SafeOFStreamError);
        }

        bool repeatable() const
        {
            return false;
        }
    } test_existing_dir;

    struct ExistingPermTest : TestCase
    {
        ExistingPermTest() : TestCase("existing unwriteable file") { }

        void run()
        {
            TEST_CHECK_THROWS(SafeOFStream(FSPath::cwd() / "safe_ofstream_TEST_dir" / "existing_perm"), SafeOFStreamError);
        }

        bool skip() const
        {
            return 0 == getuid();
        }

        bool repeatable() const
        {
            return false;
        }
    } test_existing_perm;

    struct WriteFailTest : TestCase
    {
        WriteFailTest() : TestCase("write fail") { }

        void run()
        {
            bool threw(false);
            try
            {
                SafeOFStream s(FSPath("/dev/full"));
                TEST_CHECK(s);
                s << "foo";
                TEST_CHECK(! s);
            }
            catch (const SafeOFStreamError &)
            {
                threw = true;
            }

            TEST_CHECK(threw);
        }

        bool repeatable() const
        {
            return false;
        }
    } test_existing_write_fail;
}

