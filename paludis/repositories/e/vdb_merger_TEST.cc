/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include "vdb_merger.hh"
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/standard_output_manager.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    class VDBMergerNoDisplay :
        public VDBMerger
    {
        private:
            void display_override(const std::string &) const
            {
            }

        public:
            VDBMergerNoDisplay(const VDBMergerParams & p) :
                VDBMerger(p)
            {
            }

            bool check()
            {
                return FSMerger::check();
            }

            void on_enter_dir(bool, const FSEntry)
            {
            }
    };

    class VDBMergerTest :
        public TestCase
    {
        public:

            FSEntry root_dir;
            std::string target;
            TestEnvironment env;
            VDBMergerNoDisplay merger;

            bool repeatable() const
            {
                return false;
            }

        protected:

            VDBMergerTest(const std::string & what) :
                TestCase("merge '" + what + "' test"),
                root_dir(FSEntry::cwd() / "vdb_merger_TEST_dir" / what / "root"),
                target(what),
                merger(make_named_values<VDBMergerParams>(
                            n::config_protect() = "/protected_file /protected_dir",
                            n::config_protect_mask() = "/protected_dir/unprotected_file /protected_dir/unprotected_dir",
                            n::contents_file() = FSEntry::cwd() / "vdb_merger_TEST_dir/CONTENTS" / what,
                            n::environment() = &env,
                            n::fix_mtimes_before() = Timestamp(0, 0),
                            n::image() = FSEntry::cwd() / "vdb_merger_TEST_dir" / what / "image",
                            n::merged_entries() = std::make_shared<FSEntrySet>(),
                            n::options() = FSMergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                            n::output_manager() = std::make_shared<StandardOutputManager>(),
                            n::package_id() = std::shared_ptr<PackageID>(),
                            n::root() = root_dir
                        ))
            {
            }
    };
}

namespace test_cases
{
    struct VDBMergerTestConfigProtect : VDBMergerTest
    {
        VDBMergerTestConfigProtect() : VDBMergerTest("config_protect") { }

        static std::string file_contents(const FSEntry & f)
        {
            if (! f.is_regular_file())
                return "";

            try
            {
                SafeIFStream stream(f);

                std::string contents;
                stream >> contents;
                return contents;
            }
            catch (const SafeIFStreamError &)
            {
                return "";
            }
        }

        void run()
        {
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_file"), "bar");
            TEST_CHECK(! (root_dir / "._cfg0000_protected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "unprotected_file"), "bar");
            TEST_CHECK(! (root_dir / "._cfg0000_unprotected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_file_not_really"), "bar");
            TEST_CHECK(! (root_dir / "._cfg0000_protected_file_not_really").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_protected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_file"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_unprotected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_file_not_really"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_unprotected_file_not_really").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file_already_needs_update"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_protected_file_already_needs_update"), "baz");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0001_protected_file_already_needs_update").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unchanged_protected_file"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_unchanged_protected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file_same_as_existing_update"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_protected_file_same_as_existing_update"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0001_protected_file_same_as_existing_update").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_dir/unprotected_file"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir/unprotected_dir/._cfg0000_unprotected_file").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_dir_not_really/protected_file"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir/unprotected_dir_not_really/._cfg0000_protected_file").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir_not_really/unprotected_file"), "bar");
            TEST_CHECK(! (root_dir / "protected_dir_not_really/._cfg0000_unprotected_file").exists());

            merger.merge();

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_file"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "._cfg0000_protected_file"), "foo");
            TEST_CHECK_EQUAL(file_contents(root_dir / "unprotected_file"), "foo");
            TEST_CHECK(! (root_dir / "._cfg0000_unprotected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_file_not_really"), "foo");
            TEST_CHECK(! (root_dir / "._cfg0000_protected_file_not_really").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_protected_file"), "foo");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_file"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_unprotected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_file_not_really"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_unprotected_file_not_really"), "foo");

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file_already_needs_update"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_protected_file_already_needs_update"), "baz");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0001_protected_file_already_needs_update"), "foo");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unchanged_protected_file"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0000_unchanged_protected_file").exists());
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/protected_file_same_as_existing_update"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/._cfg0000_protected_file_same_as_existing_update"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/._cfg0001_protected_file_same_as_existing_update").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_dir/unprotected_file"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir/unprotected_dir/._cfg0000_unprotected_file").exists());

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_dir_not_really/protected_file"), "bar");
            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir/unprotected_dir_not_really/._cfg0000_protected_file"), "foo");

            TEST_CHECK_EQUAL(file_contents(root_dir / "protected_dir_not_really/unprotected_file"), "foo");
            TEST_CHECK(! (root_dir / "protected_dir_not_really/._cfg0000_unprotected_file").exists());
        }
    } test_vdb_merger_config_protect;

    struct VDBMergerTestFileNewline : VDBMergerTest
    {
        VDBMergerTestFileNewline() : VDBMergerTest("file_newline") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_file_newline;

    struct VDBMergerTestDirNewline : VDBMergerTest
    {
        VDBMergerTestDirNewline() : VDBMergerTest("dir_newline") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_dir_newline;

    struct VDBMergerTestSymNewline : VDBMergerTest
    {
        VDBMergerTestSymNewline() : VDBMergerTest("sym_newline") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_sym_newline;

    struct VDBMergerTestSymTargetNewline : VDBMergerTest
    {
        VDBMergerTestSymTargetNewline() : VDBMergerTest("sym_target_newline") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_sym_target_newline;

    struct VDBMergerTestSymArrow : VDBMergerTest
    {
        VDBMergerTestSymArrow() : VDBMergerTest("sym_arrow") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_sym_arrow;

    struct VDBMergerTestSymArrow2 : VDBMergerTest
    {
        VDBMergerTestSymArrow2() : VDBMergerTest("sym_arrow2") { }

        void run()
        {
            TEST_CHECK_THROWS(merger.check(), FSMergerError);
        }
    } test_vdb_merger_sym_arrow2;
}

