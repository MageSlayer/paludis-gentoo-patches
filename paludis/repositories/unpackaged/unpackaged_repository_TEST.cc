/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/visitor-impl.hh>
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
#include <paludis/util/kc.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct MembersTest : TestCase
    {
        MembersTest() : TestCase("members") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/pkg"))
                        (k::install_under(), FSEntry("/"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::description(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        ));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageIDSequence> ids(
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
            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/pkg"))
                        (k::install_under(), FSEntry("/"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        (k::description(), "")
                        ));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            TEST_CHECK_EQUAL(id->version(), VersionSpec("1.0"));
            TEST_CHECK_EQUAL(id->slot(), SlotName("foo"));
            TEST_CHECK_EQUAL(id->name(), QualifiedPackageName("cat/pkg"));
            TEST_CHECK_EQUAL(id->repository()->name(), RepositoryName("unpackaged"));
            TEST_CHECK(id->fs_location_key());
            TEST_CHECK_EQUAL(id->fs_location_key()->value(), FSEntry("unpackaged_repository_TEST_dir/pkg"));
        }
    } test_metadata;

    struct MasksTest : TestCase
    {
        MasksTest() : TestCase("masks") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/pkg"))
                        (k::install_under(), FSEntry("/"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        (k::description(), "")
                        ));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id(
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
            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/pkg"))
                        (k::install_under(), FSEntry("/"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        (k::description(), "")
                        ));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK(repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InstalledAction>()));

            const std::tr1::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            TEST_CHECK(id->supports_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<UninstallAction>()));
            TEST_CHECK(! id->supports_action(SupportsActionTest<InstalledAction>()));
        }
    } test_actions;

    struct InstallTest : TestCase
    {
        InstallTest() : TestCase("install") { }

        void run()
        {
            TestEnvironment env;

            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/pkg"))
                        (k::install_under(), FSEntry("/"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        (k::description(), "")
                        ));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<Repository> installed_repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("unpackaged_repository_TEST_dir/installed"))
                        .root(FSEntry("unpackaged_repository_TEST_dir/root"))
                        ));
            env.package_database()->add_repository(0, installed_repo);

            TEST_CHECK(! FSEntry("unpackaged_repository_TEST_dir/root/first").is_regular_file());

            const std::tr1::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            InstallAction action(InstallActionOptions::named_create()
                    (k::no_config_protect(), false)
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );
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

            std::tr1::shared_ptr<Repository> repo(new UnpackagedRepository(
                        RepositoryName("unpackaged"),
                        unpackaged_repositories::UnpackagedRepositoryParams::named_create()
                        (k::environment(), &env)
                        (k::name(), QualifiedPackageName("cat/pkg"))
                        (k::version(), VersionSpec("1.0"))
                        (k::slot(), SlotName("foo"))
                        (k::location(), FSEntry("unpackaged_repository_TEST_dir/under_pkg"))
                        (k::install_under(), FSEntry("/magic/pixie"))
                        (k::build_dependencies(), "")
                        (k::run_dependencies(), "")
                        (k::rewrite_ids_over_to_root(), -1)
                        (k::description(), "")
                        ));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<Repository> installed_repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("unpackaged_repository_TEST_dir/under_installed"))
                        .root(FSEntry("unpackaged_repository_TEST_dir/under_root"))
                        ));
            env.package_database()->add_repository(0, installed_repo);

            TEST_CHECK(! FSEntry("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").is_regular_file());

            const std::tr1::shared_ptr<const PackageID> id(
                    *env[selection::RequireExactlyOne(generator::All())]->begin());

            InstallAction action(InstallActionOptions::named_create()
                    (k::no_config_protect(), false)
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );
            id->perform_action(action);

            TEST_CHECK(FSEntry("unpackaged_repository_TEST_dir/under_root/magic/pixie/first").is_regular_file());
        }

        bool repeatable() const
        {
            return false;
        }
    } test_install_under;
}

