/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    struct TestStripper :
        Stripper
    {
        void on_enter_dir(const FSPath &) override
        {
        }

        void on_leave_dir(const FSPath &) override
        {
        }


        void on_strip(const FSPath &) override
        {
        }

        void on_split(const FSPath &, const FSPath &) override
        {
        }

        void on_dwarf_compress(const FSPath &) override
        {
        }

        void on_unknown(const FSPath &) override
        {
        }

        TestStripper(const StripperOptions & o) :
            Stripper(o)
        {
        }
    };
}

TEST(Stripper, Works)
{
    TestStripper s(make_named_values<StripperOptions>(
                n::compress_splits() = false,
                n::controllable_strip() = false,
                n::controllable_strip_dir() = "",
                n::debug_dir() = FSPath("stripper_TEST_dir/image").realpath() / "usr" / "lib" / "debug",
                n::dwarf_compression() = false,
                n::image_dir() = FSPath("stripper_TEST_dir/image").realpath(),
                n::split() = true,
                n::strip_choice() = true,
                n::strip_restrict() = false
            ));
    s.strip();

    ASSERT_TRUE(FSPath("stripper_TEST_dir/image/usr/lib/debug/usr/bin/stripper_TEST_binary.debug").stat().is_regular_file());
}

