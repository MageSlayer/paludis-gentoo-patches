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

#include <paludis/repositories/unpackaged/unpackaged_repository.hh>
#include <paludis/repositories/unpackaged/installed_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
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

    void dummy_used_this_for_config_protect(const std::string &)
    {
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

namespace test_cases
{
    struct MembersTest : TestCase
    {
        MembersTest() : TestCase("members") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                            )));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageIDSequence> ids(
                    env[selection::AllVersionsSorted(generator::All())]);
            TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "),
                    "cat/pkg-1.0:foo::unpackaged");
        }
    } test_members;

    struct MetadataTest : TestCase
    {
        MetadataTest() : TestCase("metadata") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                        )));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            TEST_CHECK_EQUAL(id->version(), VersionSpec("1.0", VersionSpecOptions()));
            TEST_CHECK_EQUAL(id->slot_key()->value(), SlotName("foo"));
            TEST_CHECK_EQUAL(id->name(), QualifiedPackageName("cat/pkg"));
            TEST_CHECK_EQUAL(id->repository()->name(), RepositoryName("unpackaged"));
            TEST_CHECK(bool(id->fs_location_key()));
            TEST_CHECK_EQUAL(id->fs_location_key()->value(), FSEntry("unpackaged_repository_TEST_dir/pkg"));
        }
    } test_metadata;

    struct MasksTest : TestCase
    {
        MasksTest() : TestCase("masks") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                        )));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            TEST_CHECK(! id->masked());
        }
    } test_masks;

    struct ActionsTest : TestCase
    {
        ActionsTest() : TestCase("actions") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                        )));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK(repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));

            const std::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            TEST_CHECK(id->supports_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<UninstallAction>()));
        }
    } test_actions;

    struct InstallTest : TestCase
    {
        InstallTest() : TestCase("install") { }

        void run()
        {
            TestEnvironment env;

            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                        )));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<Repository> installed_repo(std::make_shared<InstalledUnpackagedRepository>(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            n::environment() = &env,
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/installed"),
                            n::root() = FSEntry("unpackaged_repository_TEST_dir/root")
                        )));
            env.package_database()->add_repository(0, installed_repo);

            TEST_CHECK(! FSEntry("unpackaged_repository_TEST_dir/root/first").is_regular_file());

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

            TEST_CHECK(FSEntry("unpackaged_repository_TEST_dir/root/first").is_regular_file());
        }

        bool repeatable() const
        {
            return false;
        }
    } test_install;

    struct InstallUnderTest : TestCase
    {
        InstallUnderTest() : TestCase("install under") { }

        void run()
        {
            TestEnvironment env;

            std::shared_ptr<Repository> repo(std::make_shared<UnpackagedRepository>(
                        RepositoryName("unpackaged"),
                        make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                            n::build_dependencies() = "",
                            n::description() = "",
                            n::environment() = &env,
                            n::install_under() = FSEntry("/magic/pixie"),
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/under_pkg"),
                            n::name() = QualifiedPackageName("cat/pkg"),
                            n::rewrite_ids_over_to_root() = -1,
                            n::run_dependencies() = "",
                            n::slot() = SlotName("foo"),
                            n::version() = VersionSpec("1.0", VersionSpecOptions())
                            )));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<Repository> installed_repo(std::make_shared<InstalledUnpackagedRepository>(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            n::environment() = &env,
                            n::location() = FSEntry("unpackaged_repository_TEST_dir/under_installed"),
                            n::root() = FSEntry("unpackaged_repository_TEST_dir/under_root")
                        )));
            env.package_database()->add_repository(0, installed_repo);

            TEST_CHECK(! FSEntry("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").is_regular_file());

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

            TEST_CHECK(FSEntry("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").is_regular_file());
        }

        bool repeatable() const
        {
            return false;
        }
    } test_install_under;
}

