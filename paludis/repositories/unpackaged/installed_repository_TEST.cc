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
#include <paludis/user_dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/contents.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/fs_stat.hh>

#include <algorithm>
#include <sstream>

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

    struct ContentsDumper
    {
        std::stringstream s;

        void visit(const ContentsFileEntry & f)
        {
            s << "file<" << f.location_key()->value() << ">";
        }

        void visit(const ContentsDirEntry & f)
        {
            s << "dir<" << f.location_key()->value() << ">";
        }

        void visit(const ContentsSymEntry & f)
        {
            s << "sym<" << f.location_key()->value() << "=" << f.target_key()->value() << ">";

        }
        void visit(const ContentsOtherEntry & f)
        {
            s << "other<" << f.location_key()->value() << ">";
        }
    };

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }
}

TEST(InstalledRepository, Content)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo1"),
                    n::root() = FSPath("installed_repository_TEST_dir/root")
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageIDSequence> ids(
            env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("cat-one/foo-1:0::installed-unpackaged cat-one/foo-2:1::installed-unpackaged",
            join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
}

TEST(InstalledRepository, Metadata)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo1"),
                    n::root() = FSPath("installed_repository_TEST_dir/root")
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:0",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    EXPECT_EQ(id1->version(), VersionSpec("1", { }));
    EXPECT_EQ(SlotName("0"), id1->slot_key()->value());
    EXPECT_EQ(QualifiedPackageName("cat-one/foo"), id1->name());
    EXPECT_EQ(RepositoryName("installed-unpackaged"), id1->repository_name());
    EXPECT_TRUE(bool(id1->fs_location_key()));
    EXPECT_EQ(id1->fs_location_key()->value(), FSPath::cwd() / "installed_repository_TEST_dir/repo1/data/giant-space-weasel/1:0:foo/");

    EXPECT_TRUE(bool(id1->contents_key()));
    ContentsDumper d1;
    std::for_each(indirect_iterator(id1->contents_key()->value()->begin()),
            indirect_iterator(id1->contents_key()->value()->end()), accept_visitor(d1));
    EXPECT_EQ("dir</fnord>", d1.s.str());

    const std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    EXPECT_EQ(id2->version(), VersionSpec("2", { }));
    EXPECT_EQ(SlotName("1"), id2->slot_key()->value());
    EXPECT_EQ(QualifiedPackageName("cat-one/foo"), id2->name());
    EXPECT_EQ(RepositoryName("installed-unpackaged"), id2->repository_name());
    EXPECT_TRUE(bool(id2->fs_location_key()));
    EXPECT_EQ(id2->fs_location_key()->value(), FSPath::cwd() / "installed_repository_TEST_dir/repo1/data/giant-space-weasel/2:1:bar/");

    EXPECT_TRUE(bool(id2->contents_key()));
    ContentsDumper d2;
    std::for_each(indirect_iterator(id2->contents_key()->value()->begin()),
            indirect_iterator(id2->contents_key()->value()->end()), accept_visitor(d2));
    EXPECT_EQ("dir</stilton>file</stilton/cheese>file</stilton/is delicious>", d2.s.str());
}

TEST(InstalledRepository, Masks)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo1"),
                    n::root() = FSPath("installed_repository_TEST_dir/root")
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:0",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(! id1->masked());

    const std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(! id2->masked());
}

TEST(InstalledRepository, Actions)
{
    TestEnvironment env;
    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo1"),
                    n::root() = FSPath("installed_repository_TEST_dir/root")
                )));
    env.add_repository(1, repo);

    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
    EXPECT_TRUE(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
    EXPECT_TRUE(repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(! id1->supports_action(SupportsActionTest<InstallAction>()));
    EXPECT_TRUE(! id1->supports_action(SupportsActionTest<ConfigAction>()));
    EXPECT_TRUE(! id1->supports_action(SupportsActionTest<PretendAction>()));
    EXPECT_TRUE(! id1->supports_action(SupportsActionTest<InfoAction>()));
    EXPECT_TRUE(id1->supports_action(SupportsActionTest<UninstallAction>()));
}

TEST(InstalledRepository, UninstallLast)
{
    TestEnvironment env;

    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo2"),
                    n::root() = FSPath("installed_repository_TEST_dir/root2")
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("cat-one/foo-1.2.3:fred::installed-unpackaged",
            join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));

    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root2/first").stat().is_regular_file());
    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root2/second").stat().is_regular_file());

    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").stat().is_symlink());
    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").stat().is_symlink());

    const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::All())]->begin());

    UninstallAction action(make_named_values<UninstallActionOptions>(
                n::config_protect() = "",
                n::if_for_install_id() = make_null_shared_ptr(),
                n::ignore_for_unmerge() = &ignore_nothing,
                n::is_overwrite() = false,
                n::make_output_manager() = &make_standard_output_manager
            ));
    id->perform_action(action);

    EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root2/first").stat().exists());
    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root2/second").stat().is_regular_file());

    EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").stat().is_symlink());
    EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").stat().is_symlink());

    repo->invalidate();

    const std::shared_ptr<const PackageIDSequence> post_ids(env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("", join(indirect_iterator(post_ids->begin()),
                indirect_iterator(post_ids->end()), " "));
}

TEST(InstalledRepository, UninstallNotLast)
{
    TestEnvironment env;

    std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                RepositoryName("installed-unpackaged"),
                make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                    n::environment() = &env,
                    n::location() = FSPath("installed_repository_TEST_dir/repo3"),
                    n::root() = FSPath("installed_repository_TEST_dir/root3")
                )));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("cat-one/foo-1.2.3:fred::installed-unpackaged cat-one/foo-3.2.1:barney::installed-unpackaged",
            join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));

    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").stat().is_symlink());
    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").stat().is_symlink());

    const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(
                generator::Matches(parse_user_package_dep_spec("cat-one/foo:fred",
                        &env, { }), make_null_shared_ptr(), { }))]->begin());

    UninstallAction action(make_named_values<UninstallActionOptions>(
                n::config_protect() = "",
                n::if_for_install_id() = make_null_shared_ptr(),
                n::ignore_for_unmerge() = &ignore_nothing,
                n::is_overwrite() = false,
                n::make_output_manager() = &make_standard_output_manager
            ));
    id->perform_action(action);

    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").stat().is_symlink());
    EXPECT_TRUE(FSPath("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").stat().is_symlink());

    repo->invalidate();

    const std::shared_ptr<const PackageIDSequence> post_ids(env[selection::AllVersionsSorted(generator::All())]);
    EXPECT_EQ("cat-one/foo-3.2.1:barney::installed-unpackaged",
            join(indirect_iterator(post_ids->begin()), indirect_iterator(post_ids->end()), " "));
}

TEST(InstalledRepository, MultipleOps)
{
    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(1, repo);

        const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
        EXPECT_EQ("", join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));

        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir").stat().exists());
    }

    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(0, repo);

        std::shared_ptr<Repository> source_repo(std::make_shared<UnpackagedRepository>(
                    RepositoryName("unpackaged"),
                    make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                        n::build_dependencies() = "",
                        n::description() = "",
                        n::environment() = &env,
                        n::install_under() = FSPath("/"),
                        n::location() = FSPath("installed_repository_TEST_dir/src4a"),
                        n::name() = QualifiedPackageName("cat/pkg4a"),
                        n::preserve_work() = indeterminate,
                        n::rewrite_ids_over_to_root() = -1,
                        n::run_dependencies() = "",
                        n::slot() = SlotName("foo"),
                        n::strip() = indeterminate,
                        n::version() = VersionSpec("1.0", { })
                    )));
        env.add_repository(1, source_repo);

        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("", join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }

        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir").stat().exists());

        InstallAction action(make_named_values<InstallActionOptions>(
                    n::destination() = repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                ));
        (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir").stat().is_directory());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4a").stat().is_regular_file());

        repo->invalidate();
        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }
    }

    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(0, repo);

        std::shared_ptr<Repository> source_repo(std::make_shared<UnpackagedRepository>(
                    RepositoryName("unpackaged"),
                    make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                        n::build_dependencies() = "",
                        n::description() = "",
                        n::environment() = &env,
                        n::install_under() = FSPath("/"),
                        n::location() = FSPath("installed_repository_TEST_dir/src4b1"),
                        n::name() = QualifiedPackageName("cat/pkg4b"),
                        n::preserve_work() = indeterminate,
                        n::rewrite_ids_over_to_root() = -1,
                        n::run_dependencies() = "",
                        n::slot() = SlotName("foo"),
                        n::strip() = indeterminate,
                        n::version() = VersionSpec("1.0", { })
                        )));
        env.add_repository(1, source_repo);

        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }

        InstallAction action(make_named_values<InstallActionOptions>(
                    n::destination() = repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                ));
        (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir").stat().is_directory());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4a").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b1").stat().is_regular_file());
        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir/4b2").stat().is_regular_file());

        repo->invalidate();
        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }
    }

    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(0, repo);

        std::shared_ptr<Repository> source_repo(std::make_shared<UnpackagedRepository>(
                    RepositoryName("unpackaged"),
                    make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                        n::build_dependencies() = "",
                        n::description() = "",
                        n::environment() = &env,
                        n::install_under() = FSPath("/"),
                        n::location() = FSPath("installed_repository_TEST_dir/src4b2"),
                        n::name() = QualifiedPackageName("cat/pkg4b"),
                        n::preserve_work() = indeterminate,
                        n::rewrite_ids_over_to_root() = -1,
                        n::run_dependencies() = "",
                        n::slot() = SlotName("foo"),
                        n::strip() = indeterminate,
                        n::version() = VersionSpec("1.0", { })
                    )));
        env.add_repository(1, source_repo);

        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }

        InstallAction action(make_named_values<InstallActionOptions>(
                    n::destination() = repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                ));
        (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir").stat().is_directory());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4a").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b").stat().is_regular_file());
        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir/4b1").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b2").stat().is_regular_file());

        repo->invalidate();
        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }
    }

    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(0, repo);

        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }

        UninstallAction action(make_named_values<UninstallActionOptions>(
                    n::config_protect() = "",
                    n::if_for_install_id() = make_null_shared_ptr(),
                    n::ignore_for_unmerge() = &ignore_nothing,
                    n::is_overwrite() = false,
                    n::make_output_manager() = &make_standard_output_manager
                ));
        (*env[selection::RequireExactlyOne(generator::Matches(
                parse_user_package_dep_spec("cat/pkg4a",
                    &env, { }), make_null_shared_ptr(), { }))]->begin())->perform_action(action);

        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir").stat().is_directory());
        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir/4a").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b").stat().is_regular_file());
        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir/4b1").stat().is_regular_file());
        EXPECT_TRUE(FSPath("installed_repository_TEST_dir/root4/dir/4b2").stat().is_regular_file());

        repo->invalidate();
        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(
                    env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }
    }

    {
        TestEnvironment env;
        std::shared_ptr<Repository> repo(std::make_shared<InstalledUnpackagedRepository>(
                    RepositoryName("installed-unpackaged"),
                    make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                        n::environment() = &env,
                        n::location() = FSPath("installed_repository_TEST_dir/repo4"),
                        n::root() = FSPath("installed_repository_TEST_dir/root4")
                    )));
        env.add_repository(0, repo);

        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("cat/pkg4b-1.0:foo::installed-unpackaged",
                    join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }

        UninstallAction action(make_named_values<UninstallActionOptions>(
                    n::config_protect() = "",
                    n::if_for_install_id() = make_null_shared_ptr(),
                    n::ignore_for_unmerge() = &ignore_nothing,
                    n::is_overwrite() = false,
                    n::make_output_manager() = &make_standard_output_manager
                ));
        (*env[selection::RequireExactlyOne(generator::Matches(
                parse_user_package_dep_spec("cat/pkg4b",
                    &env, { }), make_null_shared_ptr(), { }))]->begin())->perform_action(action);

        EXPECT_TRUE(! FSPath("installed_repository_TEST_dir/root4/dir").stat().is_directory());

        repo->invalidate();
        {
            const std::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                        generator::InRepository(RepositoryName("installed-unpackaged")))]);
            EXPECT_EQ("", join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "));
        }
    }
}

