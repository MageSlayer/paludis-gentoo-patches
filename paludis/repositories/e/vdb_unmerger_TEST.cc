/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <paludis/repositories/e/vdb_unmerger.hh>
#include <paludis/repositories/e/vdb_repository.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/metadata_key.hh>

#include <algorithm>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }

    class VDBUnmergerNoDisplay :
        public VDBUnmerger
    {
        protected:

            void display(const std::string &) const
            {
            }

        public:

            VDBUnmergerNoDisplay(const VDBUnmergerOptions & o) :
                VDBUnmerger(o)
            {
            }
    };

    std::string fix(const std::string & s)
    {
        std::string result(s);
        std::replace(result.begin(), result.end(), ' ', '_');
        std::replace(result.begin(), result.end(), '\t', '_');
        return result;
    }

    struct VDBUnmergerTest :
        testing::TestWithParam<std::string>
    {
        std::string what;
        FSPath root_dir;
        std::string target;
        TestEnvironment env;
        std::shared_ptr<Repository> repo;
        std::shared_ptr<VDBUnmergerNoDisplay> unmerger;

        VDBUnmergerTest() :
            root_dir("vdb_unmerger_TEST_dir/root")
        {
        }

        void SetUp()
        {
            what = GetParam();
            target = what;

            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "vdb_unmerger_TEST_dir" / "repo"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_unmerger_TEST_dir" / "build"));
            repo = VDBRepository::repository_factory_create(&env, std::bind(from_keys, keys, std::placeholders::_1));
            env.add_repository(0, repo);

            auto id(*env[selection::RequireExactlyOne(generator::Matches(
                            parse_user_package_dep_spec("cat/" + fix(what), &env, { }), make_null_shared_ptr(), { }))]->begin());

            unmerger = std::make_shared<VDBUnmergerNoDisplay>(make_named_values<VDBUnmergerOptions>(
                            n::config_protect() = "/protected_file /protected_dir",
                            n::config_protect_mask() = "/protected_dir/unprotected_file /protected_dir/unprotected_dir",
                            n::contents() = id->contents_key()->parse_value(),
                            n::environment() = &env,
                            n::ignore() = &ignore_nothing,
                            n::output_manager() = std::make_shared<StandardOutputManager>(),
                            n::package_id() = id,
                            n::root() = root_dir
                            ));
        }
    };
}

struct VDBUnmergerTestRemovesAll : VDBUnmergerTest
{
};

TEST_P(VDBUnmergerTestRemovesAll, RemovesAll)
{
    ASSERT_TRUE((root_dir / target).stat().exists());
    unmerger->unmerge();
    ASSERT_TRUE(! (root_dir / target).stat().exists());
}

INSTANTIATE_TEST_CASE_P(RemovesAll, VDBUnmergerTestRemovesAll, testing::Values(
            std::string("file_ok"),
            std::string("file_ with spaces"),
            std::string("file_ with lots  of   spaces"),
            std::string("file_ with trailing  space\t "),
            std::string("dir_ok"),
            std::string("dir_ with spaces"),
            std::string("dir_ with lots  of   spaces"),
            std::string("sym_ok"),
            std::string("sym_ with spaces"),
            std::string("sym_ with lots  of   spaces"),
            std::string("sym with many arrows")
            ));

struct VDBUnmergerTestRemaining : VDBUnmergerTest
{
};

TEST_P(VDBUnmergerTestRemaining, Remaining)
{
    ASSERT_TRUE((root_dir / target).stat().exists());
    unmerger->unmerge();
    ASSERT_TRUE((root_dir / target).stat().exists());
}

INSTANTIATE_TEST_CASE_P(Remaining, VDBUnmergerTestRemaining, testing::Values(
            std::string("file_bad_type"),
            std::string("file_bad_md5sum"),
            std::string("file_bad_mtime"),
            std::string("dir_bad_type"),
            std::string("dir_not_empty"),
            std::string("sym_bad_type"),
            std::string("sym_bad_dst"),
            std::string("sym_bad_mtime"),
            std::string("sym_bad_entry_1"),
            std::string("sym_bad_entry_2")
            ));

struct VDBUnmergerTestSucceeds : VDBUnmergerTest
{
};

TEST_P(VDBUnmergerTestSucceeds, Succeeds)
{
    ASSERT_TRUE((root_dir / target).stat().exists());
    unmerger->unmerge();
}

INSTANTIATE_TEST_CASE_P(Succeeds, VDBUnmergerTestSucceeds, testing::Values(
            std::string("file_replaces_dir")
            ));

struct VDBUnmergerTestConfigProtect : VDBUnmergerTest
{
};

TEST_P(VDBUnmergerTestConfigProtect, Works)
{
    EXPECT_TRUE((root_dir / "protected_file").stat().is_regular_file());
    EXPECT_TRUE((root_dir / "unprotected_file").stat().is_regular_file());
    EXPECT_TRUE((root_dir / "protected_file_not_really").stat().is_regular_file());

    EXPECT_TRUE((root_dir / "protected_dir/protected_file").stat().is_regular_file());
    EXPECT_TRUE((root_dir / "protected_dir/unprotected_file").stat().is_regular_file());
    EXPECT_TRUE((root_dir / "protected_dir/unprotected_file_not_really").stat().is_regular_file());

    EXPECT_TRUE((root_dir / "protected_dir/unprotected_dir/unprotected_file").stat().is_regular_file());

    EXPECT_TRUE((root_dir / "protected_dir/unprotected_dir_not_really/protected_file").stat().is_regular_file());

    EXPECT_TRUE((root_dir / "protected_dir_not_really/unprotected_file").stat().is_regular_file());

    unmerger->unmerge();

    EXPECT_TRUE((root_dir / "protected_file").stat().exists());
    EXPECT_TRUE(! (root_dir / "unprotected_file").stat().exists());
    EXPECT_TRUE(! (root_dir / "protected_file_not_really").stat().exists());

    EXPECT_TRUE((root_dir / "protected_dir/protected_file").stat().exists());
    EXPECT_TRUE(! (root_dir / "protected_dir/unprotected_file").stat().exists());
    EXPECT_TRUE((root_dir / "protected_dir/unprotected_file_not_really").stat().exists());

    EXPECT_TRUE(! (root_dir / "protected_dir/unprotected_dir/unprotected_file").stat().exists());

    EXPECT_TRUE((root_dir / "protected_dir/unprotected_dir_not_really/protected_file").stat().exists());

    EXPECT_TRUE(! (root_dir / "protected_dir_not_really/unprotected_file").stat().exists());
}

INSTANTIATE_TEST_CASE_P(Succeeds, VDBUnmergerTestConfigProtect, testing::Values(
            std::string("config_protect")
            ));

