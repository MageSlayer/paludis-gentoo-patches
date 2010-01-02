/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "merger.hh"
#include <paludis/environments/test/test_environment.hh>
#include <paludis/hooker.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/hook.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <iterator>
#include <list>

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

namespace paludis
{
    class HookTestEnvironment :
        public TestEnvironment
    {
        private:
            mutable std::tr1::shared_ptr<Hooker> hooker;
            mutable std::list<std::pair<FSEntry, bool> > hook_dirs;

        public:
            HookTestEnvironment(const FSEntry & hooks);

            virtual ~HookTestEnvironment();

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

    };

    HookTestEnvironment::HookTestEnvironment(const FSEntry & hooks)
    {
        if (hooks.is_directory())
            hook_dirs.push_back(std::make_pair(hooks, false));
    }

    HookTestEnvironment::~HookTestEnvironment()
    {
    }

    HookResult
    HookTestEnvironment::perform_hook(const Hook & hook) const
    {
        if (! hooker)
        {
            hooker.reset(new Hooker(this));
            for (std::list<std::pair<FSEntry, bool> >::const_iterator h(hook_dirs.begin()),
                    h_end(hook_dirs.end()) ; h != h_end ; ++h)
                hooker->add_dir(h->first, h->second);
        }
        return hooker->perform_hook(hook);
    }
}


namespace
{
    struct TestMerger :
        Merger
    {
        TestMerger(const MergerParams & p) :
            Merger(p)
        {
        }

        void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const MergeStatusFlags &)
        {
        }

        void record_install_dir(const FSEntry &, const FSEntry &, const MergeStatusFlags &)
        {
        }

        void record_install_sym(const FSEntry &, const FSEntry &, const MergeStatusFlags &)
        {
        }

        virtual void record_install_under_dir(const FSEntry &, const MergeStatusFlags &)
        {
        }

        void on_error(bool is_check, const std::string & s)
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

        bool config_protected(const FSEntry &, const FSEntry &)
        {
            return false;
        }

        std::string make_config_protect_name(const FSEntry & src, const FSEntry &)
        {
            return src.basename() + ".cfgpro";
        }
    };

    class MergerTest :
        public TestCase
    {
        public:
            FSEntry image_dir;
            FSEntry root_dir;
            HookTestEnvironment env;
            TestMerger merger;

            bool repeatable() const
            {
                return false;
            }

        protected:
            MergerTest(EntryType src_type, EntryType dst_type, int n = 0) :
                TestCase("merge " + stringify(src_type) + " over " + stringify(dst_type) + (0 == n ? "" : " "
                            + stringify(n))),
                image_dir("merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n)) + "_dir/image"),
                root_dir("merger_TEST_dir/" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n)) + "_dir/root"),
                env(FSEntry("merger_TEST_dir/hooks")),
                merger(make_named_values<MergerParams>(
                            value_for<n::environment>(&env),
                            value_for<n::fix_mtimes_before>(Timestamp(0, 0)),
                            value_for<n::get_new_ids_or_minus_one>(&get_new_ids_or_minus_one),
                            value_for<n::image>(image_dir),
                            value_for<n::install_under>(FSEntry("/")),
                            value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                            value_for<n::no_chown>(true),
                            value_for<n::options>(MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs),
                            value_for<n::root>(root_dir)
                        ))
            {
            }

            MergerTest(const std::string & custom_test,
                    const MergerOptions & o = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                    const bool fix = false) :
                TestCase("merge " + custom_test + " test"),
                image_dir("merger_TEST_dir/" + custom_test + "/image"),
                root_dir("merger_TEST_dir/" + custom_test + "/root"),
                env(FSEntry("merger_TEST_dir/hooks")),
                merger(make_named_values<MergerParams>(
                        value_for<n::environment>(&env),
                        value_for<n::fix_mtimes_before>(fix ? FSEntry("merger_TEST_dir/reference").mtim() : Timestamp(0, 0)),
                        value_for<n::get_new_ids_or_minus_one>(&get_new_ids_or_minus_one),
                        value_for<n::image>(image_dir),
                        value_for<n::install_under>(FSEntry("/")),
                        value_for<n::merged_entries>(make_shared_ptr(new FSEntrySet)),
                        value_for<n::no_chown>(true),
                        value_for<n::options>(o),
                        value_for<n::root>(root_dir)
                        ))
            {
            }
    };
}

namespace test_cases
{
    struct MergerTestSymNothing : MergerTest
    {
        MergerTestSymNothing() : MergerTest(et_sym, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "sym").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK((root_dir / "rewrite_me").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_nothing;

    struct MergerTestSymSym : MergerTest
    {
        MergerTestSymSym() : MergerTest(et_sym, et_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "root_dst");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK((root_dir / "rewrite_me").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_sym;

    struct MergerTestSymFile : MergerTest
    {
        MergerTestSymFile() : MergerTest(et_sym, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_regular_file());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK((root_dir / "rewrite_me").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
            TEST_CHECK_EQUAL((root_dir / "rewrite_me").readlink(), "/rewrite_target");
        }
    } test_merger_sym_file;

    struct MergerTestSymDir : MergerTest
    {
        MergerTestSymDir() : MergerTest(et_sym, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "sym").is_directory());
        }
    } test_merger_sym_dir;

    struct MergerTestDirNothing : MergerTest
    {
        MergerTestDirNothing() : MergerTest(et_dir, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "dir").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_directory());
        }
    } test_merger_dir_nothing;

    struct MergerTestDirDir : MergerTest
    {
        MergerTestDirDir() : MergerTest(et_dir, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_directory());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_directory());
        }
    } test_merger_dir_dir;

    struct MergerTestDirFile : MergerTest
    {
        MergerTestDirFile() : MergerTest(et_dir, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_regular_file());
        }
    } test_merger_dir_file;

    struct MergerTestDirSym1 : MergerTest
    {
        MergerTestDirSym1() : MergerTest(et_dir, et_sym, 1) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_directory());
            TEST_CHECK(! (root_dir / "dir" / "file").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_directory());
            TEST_CHECK((root_dir / "dir" / "file").is_regular_file());
        }
    } test_merger_dir_sym_1;

    struct MergerTestDirSym2 : MergerTest
    {
        MergerTestDirSym2() : MergerTest(et_dir, et_sym, 2) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_regular_file());
        }
    } test_merger_dir_sym_2;

    struct MergerTestDirSym3 : MergerTest
    {
        MergerTestDirSym3() : MergerTest(et_dir, et_sym, 3) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);
        }
    } test_merger_dir_sym_3;

    struct MergerTestFileNothing : MergerTest
    {
        MergerTestFileNothing() : MergerTest(et_file, et_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "file").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").is_regular_file());
            SafeIFStream f(root_dir / "file");
            TEST_CHECK(f);
            std::string fs(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()));
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_nothing;

    struct MergerTestFileFile : MergerTest
    {
        MergerTestFileFile() : MergerTest(et_file, et_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").is_regular_file());
            SafeIFStream b(root_dir / "file");
            TEST_CHECK(b);
            std::string bs((std::istreambuf_iterator<char>(b)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(bs, "root contents\n");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").is_regular_file());
            SafeIFStream f(root_dir / "file");
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_file;

    struct MergerTestFileSym : MergerTest
    {
        MergerTestFileSym() : MergerTest(et_file, et_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "file1").is_symbolic_link());
            TEST_CHECK((root_dir / "file2").is_symbolic_link());
            TEST_CHECK((root_dir / "file3").is_symbolic_link());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file1").is_regular_file());
            SafeIFStream f(root_dir / "file1");
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image 1 contents\n");

            TEST_CHECK((root_dir / "file2").is_regular_file());
            SafeIFStream f2(root_dir / "file2");
            TEST_CHECK(f2);
            std::string fs2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs2, "image 2 contents\n");

            TEST_CHECK((root_dir / "file3").is_regular_file());
            SafeIFStream f3(root_dir / "file3");
            TEST_CHECK(f3);
            std::string fs3((std::istreambuf_iterator<char>(f3)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs3, "image 3 contents\n");
        }
    } test_merger_file_sym;

    struct MergerTestFileDir : MergerTest
    {
        MergerTestFileDir() : MergerTest(et_file, et_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "file").is_directory());
        }
    } test_merger_file_dir;

    struct MergerOverrideTest : MergerTest
    {
        MergerOverrideTest() : MergerTest("override") { }

        void run()
        {
            TEST_CHECK((image_dir / "dir_skip_me").is_directory());
            TEST_CHECK((image_dir / "dir_install_me").is_directory());
            TEST_CHECK((image_dir / "file_skip_me").is_regular_file());
            TEST_CHECK((image_dir / "file_install_me").is_regular_file());
            TEST_CHECK((image_dir / "sym_skip_me").is_symbolic_link());
            TEST_CHECK((image_dir / "sym_install_me").is_symbolic_link());

            TEST_CHECK(merger.check());
            merger.merge();


            TEST_CHECK(! (root_dir / "dir_skip_me").exists());
            TEST_CHECK((root_dir / "dir_install_me").is_directory());
            TEST_CHECK(! (root_dir / "file_skip_me").exists());
            TEST_CHECK((root_dir / "file_install_me").is_regular_file());
            TEST_CHECK(! (root_dir / "sym_skip_me").exists());
            TEST_CHECK((root_dir / "sym_install_me").is_symbolic_link());
        }
    } test_merger_override;

    struct MergerEmptyDirAllowedTest : MergerTest
    {
        MergerEmptyDirAllowedTest() : MergerTest("empty_dir_allowed", MergerOptions() + mo_allow_empty_dirs) { }

        void run()
        {
            TEST_CHECK((image_dir / "empty").is_directory());
            TEST_CHECK(DirIterator(image_dir / "empty", DirIteratorOptions() + dio_include_dotfiles + dio_first_only) == DirIterator());

            TEST_CHECK(merger.check());
        }
    } test_merger_empty_dir_allowed;

    struct MergerEmptyDirDisallowedTest : MergerTest
    {
        MergerEmptyDirDisallowedTest() : MergerTest("empty_dir_disallowed", MergerOptions()) { }

        void run()
        {
            TEST_CHECK((image_dir / "empty").is_directory());
            TEST_CHECK(DirIterator(image_dir / "empty", DirIteratorOptions() + dio_include_dotfiles + dio_first_only) == DirIterator());

            TEST_CHECK(! merger.check());
        }
    } test_merger_empty_dir_disallowed;

    struct MergerEmptyRootAllowedTest : MergerTest
    {
        MergerEmptyRootAllowedTest() : MergerTest("empty_root_allowed", MergerOptions() + mo_allow_empty_dirs) { }

        void run()
        {
            TEST_CHECK(DirIterator(image_dir, DirIteratorOptions() + dio_include_dotfiles + dio_first_only) == DirIterator());

            TEST_CHECK(merger.check());
        }
    } test_merger_empty_root_allowed;

    struct MergerEmptyRootDisallowedTest : MergerTest
    {
        MergerEmptyRootDisallowedTest() : MergerTest("empty_root_disallowed", MergerOptions()) { }

        void run()
        {
            TEST_CHECK(DirIterator(image_dir, DirIteratorOptions() + dio_include_dotfiles + dio_first_only) == DirIterator());

            TEST_CHECK(! merger.check());
        }
    } test_merger_empty_root_disallowed;

    struct MergerMtimesTest : MergerTest
    {
        MergerMtimesTest() : MergerTest("mtimes", MergerOptions() + mo_preserve_mtimes) { }

        void run()
        {
            Timestamp m_new((image_dir / "new_file").mtim());
            Timestamp m_existing((image_dir / "existing_file").mtim());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "new_file").mtim() == m_new);
            TEST_CHECK((root_dir / "existing_file").mtim() == m_existing);
            TEST_CHECK(Timestamp::now().seconds() - (root_dir / "dodgy_file").mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);

            TEST_CHECK((root_dir / "dir" / "new_file").mtim() == m_new);
            TEST_CHECK(Timestamp::now().seconds() - (root_dir / "dir" / "dodgy_file").mtim().seconds() >= (60 * 60 * 24 * 365 * 3) - 1);
        }
    } test_merger_mtimes;

    struct MergerMtimesFixTest : MergerTest
    {
        MergerMtimesFixTest() : MergerTest("mtimes_fix", MergerOptions() + mo_preserve_mtimes, true) { }

        void run()
        {
            Timestamp m_new((image_dir / "new_file").mtim());
            Timestamp m_existing((image_dir / "existing_file").mtim());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "new_file").mtim() == m_new);
            TEST_CHECK((root_dir / "existing_file").mtim() == m_existing);
            TEST_CHECK((root_dir / "dodgy_file").mtim() == FSEntry("merger_TEST_dir/reference").mtim());

            TEST_CHECK((root_dir / "dir" / "new_file").mtim() == m_new);
            TEST_CHECK((root_dir / "dir" / "dodgy_file").mtim() == FSEntry("merger_TEST_dir/reference").mtim());
        }
    } test_merger_mtimes_fix;
}

