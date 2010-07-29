/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hooker.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/stringify.hh>
#include <paludis/hook.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const FSEntry &)
    {
        return std::make_pair(-1, -1);
    }
}

namespace
{
    struct TestTarMerger :
        TarMerger
    {
        TestTarMerger(const TarMergerParams & p) :
            TarMerger(p)
        {
        }

        virtual void on_error(bool is_check, const std::string & s)
        {
            if (is_check)
                make_check_fail();
            else
                throw MergerError(s);
        }

        void on_warn(bool, const std::string &)
        {
        }

        void display_override(const std::string &) const
        {
        }

        void track_install_file(const FSEntry &, const FSEntry &)
        {
        }

        void track_install_sym(const FSEntry &, const FSEntry &)
        {
        }
    };
}

namespace test_cases
{
    struct SimpleTarMergerTest : TestCase
    {
        SimpleTarMergerTest() : TestCase("simple tar merge") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            auto output(FSEntry("tar_merger_TEST_dir") / "simple.tar");

            TestEnvironment env;
            TestTarMerger merger(make_named_values<TarMergerParams>(
                        n::compression() = tmc_none,
                        n::environment() = &env,
                        n::fix_mtimes_before() = Timestamp(0, 0),
                        n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                        n::image() = FSEntry("tar_merger_TEST_dir") / "simple",
                        n::install_under() = FSEntry("/"),
                        n::merged_entries() = std::make_shared<FSEntrySet>(),
                        n::no_chown() = true,
                        n::options() = MergerOptions(),
                        n::root() = FSEntry("/"),
                        n::tar_file() = output
                        ));

            TEST_CHECK(! output.is_regular_file());

            merger.merge();
            output = FSEntry(stringify(output));

            TEST_CHECK(output.is_regular_file());
            TEST_CHECK(output.file_size() > 100);

            Command cmd("tar xf ../simple.tar 2>&1");
            cmd.with_chdir(FSEntry("tar_merger_TEST_dir/simple_extract"));
            TEST_CHECK_EQUAL(0, run_command(cmd));

            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "file").is_regular_file());
            TEST_CHECK_EQUAL((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "file").file_size(),
                    (FSEntry("tar_merger_TEST_dir") / "simple" / "file").file_size());

            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "another").is_regular_file());

            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").is_regular_file());
            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "subdir" / "subsubdir" / "script").has_permission(fs_ug_owner, fs_perm_execute));

            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "goodsym").is_symbolic_link());
            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "goodsym").readlink() == "file");

            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "badsym").is_symbolic_link());
            TEST_CHECK((FSEntry("tar_merger_TEST_dir") / "simple_extract" / "badsym").readlink() == "nothing");
        }
    } test_simple_tar_merger;
}

