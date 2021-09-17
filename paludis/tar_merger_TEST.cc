/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/tar_merger.hh>
#include <paludis/hooker.hh>
#include <paludis/hook.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/set.hh>
#include <paludis/util/process.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/return_literal_function.hh>

#include <functional>

#include <gtest/gtest.h>

#include "config.h"

using namespace paludis;

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const FSPath &)
    {
        return std::make_pair(-1, -1);
    }

    struct TestTarMerger :
        TarMerger
    {
        TestTarMerger(const TarMergerParams & p) :
            TarMerger(p)
        {
        }

        void on_error(bool is_check, const std::string & s) override
        {
            if (is_check)
                make_check_fail();
            else
                throw MergerError(s);
        }

        void on_warn(bool, const std::string &) override
        {
        }

        void display_override(const std::string &) const override
        {
        }

        void track_install_file(const FSPath &, const FSPath &) override
        {
        }

        void track_install_sym(const FSPath &, const FSPath &) override
        {
        }
    };
}

#if ENABLE_PBINS

TEST(TarMerger, Works)
{
    auto output(FSPath("tar_merger_TEST_dir") / "simple.tar");

    TestEnvironment env;
    TestTarMerger merger(make_named_values<TarMergerParams>(
                n::compression() = tmc_none,
                n::environment() = &env,
                n::fix_mtimes_before() = Timestamp(0, 0),
                n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                n::image() = FSPath("tar_merger_TEST_dir") / "simple",
                n::install_under() = FSPath("/"),
                n::maybe_output_manager() = nullptr,
                n::merged_entries() = std::make_shared<FSPathSet>(),
                n::no_chown() = true,
                n::options() = MergerOptions() + mo_rewrite_symlinks,
                n::permit_destination() = std::bind(return_literal_function(true)),
                n::root() = FSPath("/"),
                n::tar_file() = output
                ));

    ASSERT_TRUE(! output.stat().is_regular_file());

    ASSERT_TRUE(merger.check());
    merger.merge();
    output = FSPath(stringify(output));

    ASSERT_TRUE(output.stat().is_regular_file());
    EXPECT_TRUE(output.stat().file_size() > 100);

    Process untar_process(ProcessCommand({"sh", "-c", "tar xpf ../simple.tar 2>&1"}));
    untar_process.chdir(FSPath("tar_merger_TEST_dir/simple_extract"));
    ASSERT_EQ(0, untar_process.run().wait());

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "file").stat().is_regular_file());
    EXPECT_EQ((FSPath("tar_merger_TEST_dir") / "simple_extract" / "file").stat().file_size(),
            (FSPath("tar_merger_TEST_dir") / "simple" / "file").stat().file_size());

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "another").stat().is_regular_file());

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").stat().is_regular_file());
    EXPECT_TRUE(0 != ((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").stat().permissions() & S_IXUSR));

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir").stat().is_directory());
    EXPECT_EQ((FSPath("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir").stat().permissions() & 0xFFF, S_IRWXU | S_IRWXG | S_IRWXO);

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "goodsym").stat().is_symlink());
    EXPECT_EQ("file", (FSPath("tar_merger_TEST_dir") / "simple_extract" / "goodsym").readlink());

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "badsym").stat().is_symlink());
    EXPECT_EQ("nothing", (FSPath("tar_merger_TEST_dir") / "simple_extract" / "badsym").readlink());

    EXPECT_TRUE((FSPath("tar_merger_TEST_dir") / "simple_extract" / "rewritesym").stat().is_symlink());
    EXPECT_EQ("/bin/cat", (FSPath("tar_merger_TEST_dir") / "simple_extract" / "rewritesym").readlink());
}

#else

TEST(TarMerger, NotAvailable)
{
    auto output(FSPath("tar_merger_TEST_dir") / "simple.tar");

    TestEnvironment env;
    TestTarMerger merger(make_named_values<TarMergerParams>(
                n::compression() = tmc_none,
                n::environment() = &env,
                n::fix_mtimes_before() = Timestamp(0, 0),
                n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                n::image() = FSPath("tar_merger_TEST_dir") / "simple",
                n::install_under() = FSPath("/"),
                n::maybe_output_manager() = nullptr,
                n::merged_entries() = std::make_shared<FSPathSet>(),
                n::no_chown() = true,
                n::options() = MergerOptions(),
                n::permit_destination() = std::bind(return_literal_function(true)),
                n::root() = FSPath("/"),
                n::tar_file() = output
                ));

    ASSERT_TRUE(! output.stat().is_regular_file());

    EXPECT_THROW(merger.merge(), NotAvailableError);
}

#endif

