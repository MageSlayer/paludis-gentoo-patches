/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include <paludis/stripper.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_named_values.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace
{
    struct TestStripper :
        Stripper
    {
        virtual void on_enter_dir(const FSPath &)
        {
        }

        virtual void on_leave_dir(const FSPath &)
        {
        }


        virtual void on_strip(const FSPath &)
        {
        }

        virtual void on_split(const FSPath &, const FSPath &)
        {
        }

        virtual void on_unknown(const FSPath &)
        {
        }

        TestStripper(const StripperOptions & o) :
            Stripper(o)
        {
        }
    };
}

namespace test_cases
{
    struct StripperTest : TestCase
    {
        StripperTest() : TestCase("stripper") { }

        void run()
        {
            TestStripper s(make_named_values<StripperOptions>(
                        n::debug_dir() = FSPath("stripper_TEST_dir/image").realpath() / "usr" / "lib" / "debug",
                        n::image_dir() = FSPath("stripper_TEST_dir/image").realpath(),
                        n::split() = true,
                        n::strip() = true
                    ));
            s.strip();

            TEST_CHECK(FSPath("stripper_TEST_dir/image/usr/lib/debug/usr/bin/stripper_TEST_binary.debug").stat().is_regular_file());
        }

        bool repeatable() const
        {
            return false;
        }
    } test_stripper;
}

