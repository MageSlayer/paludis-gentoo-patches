/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
 * Copyright (c) 2007 David Leverton
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_merger.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/fs_stat.hh>

#include <paludis/standard_output_manager.hh>

#include <gtest/gtest.h>

using namespace paludis;

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

            void on_enter_dir(bool, const FSPath)
            {
            }
    };

    static std::string file_contents(const FSPath & f)
    {
        if (! f.stat().is_regular_file())
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

    struct VDBMergerTest :
        testing::TestWithParam<std::string>
    {
        TestEnvironment env;
        std::string target;
        FSPath root_dir;
        std::shared_ptr<VDBMergerNoDisplay> merger;

        VDBMergerTest() :
            root_dir("/")
        {
        }

        void SetUp()
        {
            target = GetParam();
            root_dir = FSPath::cwd() / "vdb_merger_TEST_dir" / target / "root";
            merger = std::make_shared<VDBMergerNoDisplay>(make_named_values<VDBMergerParams>(
                        n::config_protect() = "/protected_file /protected_dir",
                        n::config_protect_mask() = "/protected_dir/unprotected_file /protected_dir/unprotected_dir",
                        n::contents_file() = FSPath::cwd() / "vdb_merger_TEST_dir/CONTENTS" / target,
                        n::environment() = &env,
                        n::fix_mtimes_before() = Timestamp(0, 0),
                        n::image() = FSPath::cwd() / "vdb_merger_TEST_dir" / target / "image",
                        n::merged_entries() = std::make_shared<FSPathSet>(),
                        n::options() = MergerOptions() + mo_rewrite_symlinks + mo_allow_empty_dirs,
                        n::output_manager() = std::make_shared<StandardOutputManager>(),
                        n::package_id() = std::shared_ptr<PackageID>(),
                        n::root() = root_dir
                        ));
        }

        void TearDown()
        {
            merger.reset();
        }
    };
}

struct VDBMergerTestConfigProtect : VDBMergerTest { };

TEST_P(VDBMergerTestConfigProtect, ConfigProtect)
{
    EXPECT_EQ("bar", file_contents(root_dir / "protected_file"));
    EXPECT_TRUE(! (root_dir / "._cfg0000_protected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "unprotected_file"));
    EXPECT_TRUE(! (root_dir / "._cfg0000_unprotected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_file_not_really"));
    EXPECT_TRUE(! (root_dir / "._cfg0000_protected_file_not_really").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_protected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_unprotected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_file_not_really"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_unprotected_file_not_really").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file_already_needs_update"));
    EXPECT_EQ("baz", file_contents(root_dir / "protected_dir/._cfg0000_protected_file_already_needs_update"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0001_protected_file_already_needs_update").stat().exists());
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/unchanged_protected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_unchanged_protected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file_same_as_existing_update"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/._cfg0000_protected_file_same_as_existing_update"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0001_protected_file_same_as_existing_update").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_dir/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/unprotected_dir/._cfg0000_unprotected_file").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_dir_not_really/protected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/unprotected_dir_not_really/._cfg0000_protected_file").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir_not_really/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir_not_really/._cfg0000_unprotected_file").stat().exists());

    merger->merge();

    EXPECT_EQ("bar", file_contents(root_dir / "protected_file"));
    EXPECT_EQ("foo", file_contents(root_dir / "._cfg0000_protected_file"));
    EXPECT_EQ("foo", file_contents(root_dir / "unprotected_file"));
    EXPECT_TRUE(! (root_dir / "._cfg0000_unprotected_file").stat().exists());
    EXPECT_EQ("foo", file_contents(root_dir / "protected_file_not_really"));
    EXPECT_TRUE(! (root_dir / "._cfg0000_protected_file_not_really").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/._cfg0000_protected_file"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_unprotected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_file_not_really"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/._cfg0000_unprotected_file_not_really"));

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file_already_needs_update"));
    EXPECT_EQ("baz", file_contents(root_dir / "protected_dir/._cfg0000_protected_file_already_needs_update"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/._cfg0001_protected_file_already_needs_update"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/unchanged_protected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0000_unchanged_protected_file").stat().exists());
    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/protected_file_same_as_existing_update"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/._cfg0000_protected_file_same_as_existing_update"));
    EXPECT_TRUE(! (root_dir / "protected_dir/._cfg0001_protected_file_same_as_existing_update").stat().exists());

    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/unprotected_dir/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir/unprotected_dir/._cfg0000_unprotected_file").stat().exists());

    EXPECT_EQ("bar", file_contents(root_dir / "protected_dir/unprotected_dir_not_really/protected_file"));
    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir/unprotected_dir_not_really/._cfg0000_protected_file"));

    EXPECT_EQ("foo", file_contents(root_dir / "protected_dir_not_really/unprotected_file"));
    EXPECT_TRUE(! (root_dir / "protected_dir_not_really/._cfg0000_unprotected_file").stat().exists());
}

INSTANTIATE_TEST_CASE_P(ConfigProtect, VDBMergerTestConfigProtect, testing::Values(std::string("config_protect")));

struct VDBMergerErrorTest : VDBMergerTest { };

TEST_P(VDBMergerErrorTest, Error)
{
    EXPECT_THROW(merger->check(), FSMergerError);
}

INSTANTIATE_TEST_CASE_P(Errors, VDBMergerErrorTest, testing::Values(
            std::string("dir_newline"),
            std::string("sym_newline"),
            std::string("sym_target_newline"),
            std::string("sym_arrow"),
            std::string("sym_arrow2")
            ));

