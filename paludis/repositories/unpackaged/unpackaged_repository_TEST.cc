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

#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/slot.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/fs_stat.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void cannot_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

TEST(UnpackagedRepository, Members)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                    )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageIDSequence> ids(
            env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("cat/pkg-1.0:foo::unpackaged", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
}

TEST(UnpackagedRepository, Metadata)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id(
            *env[selection::RequireExactlyOne(generator::All())]->begin());

    EXPECT_EQ(id->version(), VersionSpec("1.0", { }));
    EXPECT_EQ("foo", id->slot_key()->parse_value().raw_value());
    EXPECT_EQ(QualifiedPackageName("cat/pkg"), id->name());
    EXPECT_EQ(RepositoryName("unpackaged"), id->repository_name());
    ASSERT_TRUE(bool(id->fs_location_key()));
    EXPECT_EQ(FSPath("unpackaged_repository_TEST_dir/pkg"), id->fs_location_key()->parse_value());
}

TEST(UnpackagedRepository, Masks)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id(
            *env[selection::RequireExactlyOne(generator::All())]->begin());

    EXPECT_TRUE(! id->masked());
}

TEST(UnpackagedRepository, Actions)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                )));
    env.add_repository(1, repo);

    EXPECT_TRUE(repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));

    const std::shared_ptr<const PackageID> id(
            *env[selection::RequireExactlyOne(generator::All())]->begin());

    EXPECT_TRUE(id->supports_action(SupportsActionTest<InstallAction>()));
    EXPECT_TRUE(! id->supports_action(SupportsActionTest<ConfigAction>()));
    EXPECT_TRUE(! id->supports_action(SupportsActionTest<PretendAction>()));
    EXPECT_TRUE(! id->supports_action(SupportsActionTest<InfoAction>()));
    EXPECT_TRUE(! id->supports_action(SupportsActionTest<UninstallAction>()));
}

TEST(UnpackagedRepository, Install)
{
    TestEnvironment env;

    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                )));
    env.add_repository(1, repo);

    std::shared_ptr<Repository> installed_repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("unpackaged_repository_TEST_dir/installed"),
                    n::root() = FSPath("unpackaged_repository_TEST_dir/root"),
                    n::split_debug_location() = "/usr/lib/debug",
                    n::tool_prefix() = ""
                )));
    env.add_repository(0, installed_repo);

    EXPECT_TRUE(! FSPath("unpackaged_repository_TEST_dir/root/first").stat().is_regular_file());

    const std::shared_ptr<const PackageID> id(
            *env[selection::RequireExactlyOne(generator::All())]->begin());

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));
    id->perform_action(action);

    EXPECT_TRUE(FSPath("unpackaged_repository_TEST_dir/root/first").stat().is_regular_file());
}

TEST(UnpackagedRepository, InstallUnder)
{
    TestEnvironment env;

    std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                RepositoryName("unpackaged"),
                make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                    n::build_dependencies_target() = "",
                    n::build_dependencies_host() = "",
                    n::description() = "",
                    n::environment() = &env,
                    n::install_under() = FSPath("/magic/pixie"),
                    n::location() = FSPath("unpackaged_repository_TEST_dir/under_pkg"),
                    n::name() = QualifiedPackageName("cat/pkg"),
                    n::preserve_work() = indeterminate,
                    n::rewrite_ids_over_to_root() = -1,
                    n::run_dependencies() = "",
                    n::slot() = SlotName("foo"),
                    n::strip() = indeterminate,
                    n::version() = VersionSpec("1.0", { })
                    )));
    env.add_repository(1, repo);

    std::shared_ptr<Repository> installed_repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("unpackaged_repository_TEST_dir/under_installed"),
                    n::root() = FSPath("unpackaged_repository_TEST_dir/under_root"),
                    n::split_debug_location() = "/usr/lib/debug",
                    n::tool_prefix() = ""
                )));
    env.add_repository(0, installed_repo);

    EXPECT_TRUE(! FSPath("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").stat().is_regular_file());

    const std::shared_ptr<const PackageID> id(
            *env[selection::RequireExactlyOne(generator::All())]->begin());

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));
    id->perform_action(action);

    EXPECT_TRUE(FSPath("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").stat().is_regular_file());
}

