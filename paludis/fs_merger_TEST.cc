/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/fs_merger.hh>
#include <paludis/hooker.hh>
#include <paludis/hook.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/return_literal_function.hh>

#include <functional>
#include <iterator>
#include <list>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::pair<uid_t, gid_t>
    get_new_ids_or_minus_one(const FSPath &)
    {
        return std::make_pair(-1, -1);
    }

    bool
    timestamps_nearly_equal(const Timestamp & i_set, const Timestamp & reference)
    {
        return i_set == reference ||
            (i_set.seconds() == reference.seconds() &&
             i_set.nanoseconds() % 1000 == 0 &&
             i_set.nanoseconds() / 1000 == reference.nanoseconds() / 1000);
    }

    class HookTestEnvironment :
        public TestEnvironment
    {
        private:
            mutable std::shared_ptr<Hooker> hooker;
            mutable std::list<std::pair<FSPath, bool> > hook_dirs;

        public:
            HookTestEnvironment(const FSPath & hooks);

            virtual ~HookTestEnvironment();

            virtual HookResult perform_hook(const Hook &, const std::shared_ptr<OutputManager> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    HookTestEnvironment::HookTestEnvironment(const FSPath & hooks)
    {
        if (hooks.stat().is_directory())
            hook_dirs.push_back(std::make_pair(hooks, false));
    }

    HookTestEnvironment::~HookTestEnvironment()
    {
    }

    HookResult
    HookTestEnvironment::perform_hook(const Hook & hook, const std::shared_ptr<OutputManager> & optional_output_manager) const
    {
        if (! hooker)
        {
            hooker = std::make_shared<Hooker>(this);
            for (std::list<std::pair<FSPath, bool> >::const_iterator h(hook_dirs.begin()),
                    h_end(hook_dirs.end()) ; h != h_end ; ++h)
                hooker->add_dir(h->first, h->second);
        }
        return hooker->perform_hook(hook, optional_output_manager);
    }

    struct TestMerger :
        FSMerger
    {
        TestMerger(const FSMergerParams & p) :
            FSMerger(p)
        {
        }

        void record_install_file(const FSPath &, const FSPath &, const std::string &, const FSMergerStatusFlags &)
        {
        }

        void record_install_dir(const FSPath &, const FSPath &, const FSMergerStatusFlags &)
        {
        }

        void record_install_sym(const FSPath &, const FSPath &, const FSMergerStatusFlags &)
        {
        }

        virtual void record_install_under_dir(const FSPath &, const FSMergerStatusFlags &)
        {
        }

        void on_error(bool is_check, const std::string & s)
        {
            if (is_check)
                make_check_fail();
            else
                throw FSMergerError(s);
        }

        void on_warn(bool, const std::string &)
        {
        }

        void display_override(const std::string &) const
        {
        }

        bool config_protected(const FSPath &, const FSPath &)
        {
            return false;
        }

        std::string make_config_protect_name(const FSPath & src, const FSPath &)
        {
            return src.basename() + ".cfgpro";
        }
    };

    struct MergerAndFriends
    {
        FSPath image_dir;
        FSPath root_dir;
        HookTestEnvironment env;
        TestMerger merger;

        MergerAndFriends(EntryType src_type, EntryType dst_type, int n = 0) :
            image_dir("fs_merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                    + (0 == n ? "" : "_" + stringify(n)) + "_dir/image"),
            root_dir("fs_merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                    + (0 == n ? "" : "_" + stringify(n)) + "_dir/root"),
            env(FSPath("fs_merger_TEST_dir/hooks")),
            merger(make_named_values<FSMergerParams>(
                        n::environment() = &env,
                        n::fix_mtimes_before() = Timestamp(0, 0),
                        n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                        n::image() = image_dir,
                        n::install_under() = FSPath("/"),
                        n::maybe_output_manager() = make_null_shared_ptr(),
                        n::merged_entries() = std::make_shared<FSPathSet>(),
                        n::no_chown() = true,
                        n::options() = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                        n::permit_destination() = std::bind(return_literal_function(true)),
                        n::root() = root_dir.realpath()
                        ))
        {
        }

        MergerAndFriends(const std::string & custom_test,
                const MergerOptions & o = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                const bool fix = false) :
            image_dir("fs_merger_TEST_dir/" + custom_test + "/image"),
            root_dir("fs_merger_TEST_dir/" + custom_test + "/root"),
            env(FSPath("fs_merger_TEST_dir/hooks")),
            merger(make_named_values<FSMergerParams>(
                    n::environment() = &env,
                    n::fix_mtimes_before() = fix ? FSPath("fs_merger_TEST_dir/reference").stat().mtim() : Timestamp(0, 0),
                    n::get_new_ids_or_minus_one() = &get_new_ids_or_minus_one,
                    n::image() = image_dir,
                    n::install_under() = FSPath("/"),
                    n::maybe_output_manager() = make_null_shared_ptr(),
                    n::merged_entries() = std::make_shared<FSPathSet>(),
                    n::no_chown() = true,
                    n::options() = o,
                    n::permit_destination() = std::bind(return_literal_function(true)),
                    n::root() = root_dir.realpath()
                    ))
        {
        }
    };

    std::shared_ptr<MergerAndFriends> make_merger(EntryType src_type, EntryType dst_type, int n = 0)
    {
        return std::make_shared<MergerAndFriends>(src_type, dst_type, n);
    }

    std::shared_ptr<MergerAndFriends> make_merger(const std::string & custom_test,
                const MergerOptions & o = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                const bool fix = false)
    {
        return std::make_shared<MergerAndFriends>(custom_test, o, fix);
    }
}

TEST(Merger, SymNothing)
{
    auto data(make_merger(et_sym, et_nothing, 0));
    ASSERT_TRUE(! (data->root_dir / "sym").stat().exists());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "sym").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "rewrite_me").stat().is_symlink());
    EXPECT_EQ("image_dst", (data->root_dir / "sym").readlink());
    EXPECT_EQ("/rewrite_target", (data->root_dir / "rewrite_me").readlink());
}

TEST(Merger, SymSym)
{
    auto data(make_merger(et_sym, et_sym, 0));

    ASSERT_TRUE((data->root_dir / "sym").stat().is_symlink());
    EXPECT_EQ("root_dst", (data->root_dir / "sym").readlink());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "sym").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "rewrite_me").stat().is_symlink());
    EXPECT_EQ("image_dst", (data->root_dir / "sym").readlink());
    EXPECT_EQ("/rewrite_target", (data->root_dir / "rewrite_me").readlink());
}

TEST(Merger, SymFile)
{
    auto data(make_merger(et_sym, et_file, 0));
    ASSERT_TRUE((data->root_dir / "sym").stat().is_regular_file());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "sym").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "rewrite_me").stat().is_symlink());
    EXPECT_EQ("image_dst", (data->root_dir / "sym").readlink());
    EXPECT_EQ("/rewrite_target", (data->root_dir / "rewrite_me").readlink());
}

TEST(Merger, SymDir)
{
    auto data(make_merger(et_sym, et_dir, 0));
    ASSERT_TRUE((data->root_dir / "sym").stat().is_directory());

    ASSERT_TRUE(! data->merger.check());
    EXPECT_THROW(data->merger.merge(), FSMergerError);

    ASSERT_TRUE((data->root_dir / "sym").stat().is_directory());
}

TEST(Merger, DirNothing)
{
    auto data(make_merger(et_dir, et_nothing, 0));
    ASSERT_TRUE(! (data->root_dir / "dir").stat().exists());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "dir").stat().is_directory());
}

TEST(Merger, DirDir)
{
    auto data(make_merger(et_dir, et_dir, 0));
    ASSERT_TRUE((data->root_dir / "dir").stat().is_directory());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "dir").stat().is_directory());
}

TEST(Merger, DirFile)
{
    auto data(make_merger(et_dir, et_file, 0));
    ASSERT_TRUE((data->root_dir / "dir").stat().is_regular_file());

    ASSERT_TRUE(! data->merger.check());
    EXPECT_THROW(data->merger.merge(), FSMergerError);

    ASSERT_TRUE((data->root_dir / "dir").stat().is_regular_file());
}

TEST(Merger, DirSym1)
{
    auto data(make_merger(et_dir, et_sym, 1));
    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "dir").realpath().stat().is_directory());
    ASSERT_TRUE(! (data->root_dir / "dir" / "file").stat().exists());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "dir").realpath().stat().is_directory());
    ASSERT_TRUE((data->root_dir / "dir" / "file").stat().is_regular_file());
}

TEST(Merger, DirSym2)
{
    auto data(make_merger(et_dir, et_sym, 2));
    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "dir").realpath().stat().is_regular_file());

    ASSERT_TRUE(! data->merger.check());
    EXPECT_THROW(data->merger.merge(), FSMergerError);

    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "dir").realpath().stat().is_regular_file());
}

TEST(Merger, DirSym3)
{
    auto data(make_merger(et_dir, et_sym, 3));
    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    EXPECT_THROW((data->root_dir / "dir").realpath(), FSError);

    ASSERT_TRUE(! data->merger.check());
    EXPECT_THROW(data->merger.merge(), FSMergerError);

    ASSERT_TRUE((data->root_dir / "dir").stat().is_symlink());
    EXPECT_THROW((data->root_dir / "dir").realpath(), FSError);
}

TEST(Merger, FileNothing)
{
    auto data(make_merger(et_file, et_nothing, 0));
    ASSERT_TRUE(! (data->root_dir / "file").stat().exists());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "file").stat().is_regular_file());
    SafeIFStream f(data->root_dir / "file");
    ASSERT_TRUE(f);
    std::string fs(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()));
    EXPECT_EQ("image contents\n", fs);
}

TEST(Merger, FileFile)
{
    auto data(make_merger(et_file, et_file, 0));
    ASSERT_TRUE((data->root_dir / "file").stat().is_regular_file());
    SafeIFStream b(data->root_dir / "file");
    ASSERT_TRUE(b);
    std::string bs((std::istreambuf_iterator<char>(b)), std::istreambuf_iterator<char>());
    EXPECT_EQ("root contents\n", bs);

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "file").stat().is_regular_file());
    SafeIFStream f(data->root_dir / "file");
    ASSERT_TRUE(f);
    std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_EQ("image contents\n", fs);
}

TEST(Merger, FileSym)
{
    auto data(make_merger(et_file, et_sym, 0));
    ASSERT_TRUE((data->root_dir / "file1").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "file2").stat().is_symlink());
    ASSERT_TRUE((data->root_dir / "file3").stat().is_symlink());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE((data->root_dir / "file1").stat().is_regular_file());
    SafeIFStream f(data->root_dir / "file1");
    ASSERT_TRUE(f);
    std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_EQ("image 1 contents\n", fs);

    ASSERT_TRUE((data->root_dir / "file2").stat().is_regular_file());
    SafeIFStream f2(data->root_dir / "file2");
    ASSERT_TRUE(f2);
    std::string fs2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
    EXPECT_EQ("image 2 contents\n", fs2);

    ASSERT_TRUE((data->root_dir / "file3").stat().is_regular_file());
    SafeIFStream f3(data->root_dir / "file3");
    ASSERT_TRUE(f3);
    std::string fs3((std::istreambuf_iterator<char>(f3)), std::istreambuf_iterator<char>());
    EXPECT_EQ("image 3 contents\n", fs3);
}

TEST(Merger, FileDir)
{
    auto data(make_merger(et_file, et_dir, 0));
    ASSERT_TRUE((data->root_dir / "file").stat().is_directory());

    ASSERT_TRUE(! data->merger.check());
    EXPECT_THROW(data->merger.merge(), FSMergerError);

    ASSERT_TRUE((data->root_dir / "file").stat().is_directory());
}

TEST(Merger, Override)
{
    auto data(make_merger("override"));

    ASSERT_TRUE((data->image_dir / "dir_skip_me").stat().is_directory());
    ASSERT_TRUE((data->image_dir / "dir_install_me").stat().is_directory());
    ASSERT_TRUE((data->image_dir / "file_skip_me").stat().is_regular_file());
    ASSERT_TRUE((data->image_dir / "file_install_me").stat().is_regular_file());
    ASSERT_TRUE((data->image_dir / "sym_skip_me").stat().is_symlink());
    ASSERT_TRUE((data->image_dir / "sym_install_me").stat().is_symlink());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE(! (data->root_dir / "dir_skip_me").stat().exists());
    ASSERT_TRUE((data->root_dir / "dir_install_me").stat().is_directory());
    ASSERT_TRUE(! (data->root_dir / "file_skip_me").stat().exists());
    ASSERT_TRUE((data->root_dir / "file_install_me").stat().is_regular_file());
    ASSERT_TRUE(! (data->root_dir / "sym_skip_me").stat().exists());
    ASSERT_TRUE((data->root_dir / "sym_install_me").stat().is_symlink());
}

TEST(Merger, EmptyDirAllowed)
{
    auto data(make_merger("empty_dir_allowed", { mo_allow_empty_dirs }));
    ASSERT_TRUE((data->image_dir / "empty").stat().is_directory());
    ASSERT_TRUE(FSIterator(data->image_dir / "empty", { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

    ASSERT_TRUE(data->merger.check());
}

TEST(Merger, EmptyDirDisallowed)
{
    auto data(make_merger("empty_dir_disallowed", { }));
    ASSERT_TRUE((data->image_dir / "empty").stat().is_directory());
    ASSERT_TRUE(FSIterator(data->image_dir / "empty", { fsio_include_dotfiles, fsio_first_only }) == FSIterator());

    ASSERT_TRUE(! data->merger.check());
}

TEST(Merger, EmptyRootAllowed)
{
    auto data(make_merger("empty_root_allowed", { mo_allow_empty_dirs }));
    ASSERT_TRUE(FSIterator(data->image_dir, { fsio_include_dotfiles, fsio_first_only }) == FSIterator());
    ASSERT_TRUE(data->merger.check());
}

TEST(Merger, EmptyRootDisallowed)
{
    auto data(make_merger("empty_root_disallowed", { }));
    ASSERT_TRUE(FSIterator(data->image_dir, { fsio_include_dotfiles, fsio_first_only }) == FSIterator());
    ASSERT_TRUE(data->merger.check());
}

TEST(Merger, Mtimes)
{
    auto data(make_merger("mtimes", { mo_preserve_mtimes }));
    Timestamp m_new((data->image_dir / "new_file").stat().mtim());
    Timestamp m_existing((data->image_dir / "existing_file").stat().mtim());
    Timestamp m_dir_new((data->image_dir / "dir" / "new_file").stat().mtim());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "new_file").stat().mtim(), m_new));
    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "existing_file").stat().mtim(), m_existing));
    ASSERT_TRUE(Timestamp::now().seconds() - (data->root_dir / "dodgy_file").stat().mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);

    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "dir" / "new_file").stat().mtim(), m_dir_new));
    ASSERT_TRUE(Timestamp::now().seconds() - (data->root_dir / "dir" / "dodgy_file").stat().mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);
}

TEST(Merger, MtimesFixes)
{
    auto data(make_merger("mtimes_fix", { mo_preserve_mtimes }, true));
    Timestamp m_new((data->image_dir / "new_file").stat().mtim());
    Timestamp m_existing((data->image_dir / "existing_file").stat().mtim());
    Timestamp m_dir_new((data->image_dir / "dir" / "new_file").stat().mtim());

    ASSERT_TRUE(data->merger.check());
    data->merger.merge();

    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "new_file").stat().mtim(), m_new));
    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "existing_file").stat().mtim(), m_existing));
    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "dodgy_file").stat().mtim(), FSPath("fs_merger_TEST_dir/reference").stat().mtim()));

    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "dir" / "new_file").stat().mtim(), m_dir_new));
    ASSERT_TRUE(timestamps_nearly_equal((data->root_dir / "dir" / "dodgy_file").stat().mtim(), FSPath("fs_merger_TEST_dir/reference").stat().mtim()));
}

