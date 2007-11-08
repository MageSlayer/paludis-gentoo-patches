/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/query.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/visitor-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>
#include <sstream>

using namespace test;
using namespace paludis;

namespace
{
    struct ContentsDumper :
        ConstVisitor<ContentsVisitorTypes>
    {
        std::stringstream s;

        void visit(const ContentsFileEntry & f)
        {
            s << "file<" << f.name() << ">";
        }

        void visit(const ContentsDirEntry & f)
        {
            s << "dir<" << f.name() << ">";
        }

        void visit(const ContentsSymEntry & f)
        {
            s << "sym<" << f.name() << "=" << f.target() << ">";
        }

        void visit(const ContentsMiscEntry & f)
        {
            s << "misc<" << f.name() << ">";
        }

        void visit(const ContentsDevEntry & f)
        {
            s << "dev<" << f.name() << ">";
        }

        void visit(const ContentsFifoEntry & f)
        {
            s << "fifo<" << f.name() << ">";
        }
    };
}

namespace test_cases
{
    struct MembersTest : TestCase
    {
        MembersTest() : TestCase("members") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo1"))
                        .root(FSEntry("installed_repository_TEST_dir/root"))
                        ));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageIDSequence> ids(
                    env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "),
                    "cat-one/foo-1:0::installed-unpackaged cat-one/foo-2:1::installed-unpackaged");
        }
    } test_members;

    struct MetadataTest : TestCase
    {
        MetadataTest() : TestCase("metadata") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo1"))
                        .root(FSEntry("installed_repository_TEST_dir/root"))
                        ));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageID> id1(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:0", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            TEST_CHECK_EQUAL(id1->version(), VersionSpec("1"));
            TEST_CHECK_EQUAL(id1->slot(), SlotName("0"));
            TEST_CHECK_EQUAL(id1->name(), QualifiedPackageName("cat-one/foo"));
            TEST_CHECK_EQUAL(id1->repository()->name(), RepositoryName("installed-unpackaged"));
            TEST_CHECK(id1->fs_location_key());
            TEST_CHECK_EQUAL(id1->fs_location_key()->value(),
                    FSEntry::cwd() / "installed_repository_TEST_dir/repo1/data/giant-space-weasel/1:0:foo/");

            TEST_CHECK(id1->contents_key());
            ContentsDumper d1;
            std::for_each(indirect_iterator(id1->contents_key()->value()->begin()),
                    indirect_iterator(id1->contents_key()->value()->end()), accept_visitor(d1));
            TEST_CHECK_EQUAL(d1.s.str(), "dir</fnord>");

            const tr1::shared_ptr<const PackageID> id2(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:1", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            TEST_CHECK_EQUAL(id2->version(), VersionSpec("2"));
            TEST_CHECK_EQUAL(id2->slot(), SlotName("1"));
            TEST_CHECK_EQUAL(id2->name(), QualifiedPackageName("cat-one/foo"));
            TEST_CHECK_EQUAL(id2->repository()->name(), RepositoryName("installed-unpackaged"));
            TEST_CHECK(id2->fs_location_key());
            TEST_CHECK_EQUAL(id2->fs_location_key()->value(),
                    FSEntry::cwd() / "installed_repository_TEST_dir/repo1/data/giant-space-weasel/2:1:bar/");

            TEST_CHECK(id2->contents_key());
            ContentsDumper d2;
            std::for_each(indirect_iterator(id2->contents_key()->value()->begin()),
                    indirect_iterator(id2->contents_key()->value()->end()), accept_visitor(d2));
            TEST_CHECK_EQUAL(d2.s.str(), "dir</stilton>file</stilton/cheese>file</stilton/is delicious>");
        }
    } test_metadata;

    struct MasksTest : TestCase
    {
        MasksTest() : TestCase("masks") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo1"))
                        .root(FSEntry("installed_repository_TEST_dir/root"))
                        ));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageID> id1(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:0", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            TEST_CHECK(! id1->masked());

            const tr1::shared_ptr<const PackageID> id2(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:1", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            TEST_CHECK(! id2->masked());
        }
    } test_masks;

    struct ActionsTest : TestCase
    {
        ActionsTest() : TestCase("actions") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo1"))
                        .root(FSEntry("installed_repository_TEST_dir/root"))
                        ));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));
            TEST_CHECK(repo->some_ids_might_support_action(SupportsActionTest<InstalledAction>()));

            const tr1::shared_ptr<const PackageID> id1(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:1", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            TEST_CHECK(! id1->supports_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(id1->supports_action(SupportsActionTest<UninstallAction>()));
            TEST_CHECK(id1->supports_action(SupportsActionTest<InstalledAction>()));
        }
    } test_actions;

    struct UninstallLastTest : TestCase
    {
        UninstallLastTest() : TestCase("uninstall last") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo2"))
                        .root(FSEntry("installed_repository_TEST_dir/root2"))
                        ));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                    env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                    "cat-one/foo-1.2.3:fred::installed-unpackaged");

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/first").is_regular_file());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/second").is_regular_file());

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").is_symbolic_link());

            const tr1::shared_ptr<const PackageID> id(
                    *env.package_database()->query(query::All(), qo_require_exactly_one)->begin());

            UninstallAction action(UninstallActionOptions::create()
                    .no_config_protect(false)
                    );
            id->perform_action(action);

            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root2/first").exists());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/second").is_regular_file());

            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").is_symbolic_link());

            repo->invalidate();

            const tr1::shared_ptr<const PackageIDSequence> post_ids(
                    env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(post_ids->begin()), indirect_iterator(post_ids->end()), " "), "");
        }

        bool repeatable() const
        {
            return false;
        }
    } test_uninstall_last;

    struct UninstallNotLastTest : TestCase
    {
        UninstallNotLastTest() : TestCase("uninstall not last") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                        .environment(&env)
                        .location(FSEntry("installed_repository_TEST_dir/repo3"))
                        .root(FSEntry("installed_repository_TEST_dir/root3"))
                        ));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                    env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                    "cat-one/foo-1.2.3:fred::installed-unpackaged cat-one/foo-3.2.1:barney::installed-unpackaged");

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").is_symbolic_link());

            const tr1::shared_ptr<const PackageID> id(
                    *env.package_database()->query(query::Matches(PackageDepSpec("cat-one/foo:fred", pds_pm_unspecific)),
                        qo_require_exactly_one)->begin());

            UninstallAction action(UninstallActionOptions::create()
                    .no_config_protect(false)
                    );
            id->perform_action(action);

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").is_symbolic_link());

            repo->invalidate();

            const tr1::shared_ptr<const PackageIDSequence> post_ids(
                    env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_EQUAL(join(indirect_iterator(post_ids->begin()), indirect_iterator(post_ids->end()), " "),
                    "cat-one/foo-3.2.1:barney::installed-unpackaged");
        }

        bool repeatable() const
        {
            return false;
        }
    } test_uninstall_not_last;

    struct MultipleOpsTest : TestCase
    {
        MultipleOpsTest() : TestCase("multiple ops") { }

        void run()
        {
            {
                TestMessageSuffix suffix("initial", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(1, repo);

                const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                        env.package_database()->query(query::All(), qo_order_by_version));
                TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "), "");

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").exists());
            }

            {
                TestMessageSuffix suffix("install 4a", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(0, repo);

                tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            unpackaged_repositories::UnpackagedRepositoryParams::create()
                            .environment(&env)
                            .name(QualifiedPackageName("cat/pkg4a"))
                            .version(VersionSpec("1.0"))
                            .slot(SlotName("foo"))
                            .location(FSEntry("installed_repository_TEST_dir/src4a"))
                            .build_dependencies("")
                            .run_dependencies("")
                            .description("")
                            ));
                env.package_database()->add_repository(1, source_repo);

                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "), "");
                }

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").exists());

                InstallAction action(InstallActionOptions::create()
                        .destination(repo)
                        .no_config_protect(false)
                        .checks(iaco_default)
                        .debug_build(iado_none));
                (*env.package_database()->query(query::Repository(RepositoryName("unpackaged")),
                                                qo_require_exactly_one)->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());

                repo->invalidate();
                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("install 4b1", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(0, repo);

                tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            unpackaged_repositories::UnpackagedRepositoryParams::create()
                            .environment(&env)
                            .name(QualifiedPackageName("cat/pkg4b"))
                            .version(VersionSpec("1.0"))
                            .slot(SlotName("foo"))
                            .location(FSEntry("installed_repository_TEST_dir/src4b1"))
                            .build_dependencies("")
                            .run_dependencies("")
                            .description("")
                            ));
                env.package_database()->add_repository(1, source_repo);

                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged");
                }

                InstallAction action(InstallActionOptions::create()
                        .destination(repo)
                        .no_config_protect(false)
                        .checks(iaco_default)
                        .debug_build(iado_none));
                (*env.package_database()->query(query::Repository(RepositoryName("unpackaged")),
                                                qo_require_exactly_one)->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("install 4b2", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(0, repo);

                tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            unpackaged_repositories::UnpackagedRepositoryParams::create()
                            .environment(&env)
                            .name(QualifiedPackageName("cat/pkg4b"))
                            .version(VersionSpec("1.0"))
                            .slot(SlotName("foo"))
                            .location(FSEntry("installed_repository_TEST_dir/src4b2"))
                            .build_dependencies("")
                            .run_dependencies("")
                            .description("")
                            ));
                env.package_database()->add_repository(1, source_repo);

                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                InstallAction action(InstallActionOptions::create()
                        .destination(repo)
                        .no_config_protect(false)
                        .checks(iaco_default)
                        .debug_build(iado_none));
                (*env.package_database()->query(query::Repository(RepositoryName("unpackaged")),
                                                qo_require_exactly_one)->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("uninstall 4a", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(0, repo);

                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                UninstallAction action(UninstallActionOptions::create()
                        .no_config_protect(false));
                (*env.package_database()->query(query::Matches(PackageDepSpec("cat/pkg4a", pds_pm_unspecific)),
                                                qo_require_exactly_one)->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("uninstall 4b", true);

                TestEnvironment env;
                tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            unpackaged_repositories::InstalledUnpackagedRepositoryParams::create()
                            .environment(&env)
                            .location(FSEntry("installed_repository_TEST_dir/repo4"))
                            .root(FSEntry("installed_repository_TEST_dir/root4"))
                            ));
                env.package_database()->add_repository(0, repo);

                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                UninstallAction action(UninstallActionOptions::create()
                        .no_config_protect(false));
                (*env.package_database()->query(query::Matches(PackageDepSpec("cat/pkg4b", pds_pm_unspecific)),
                                                qo_require_exactly_one)->begin())->perform_action(action);

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());

                repo->invalidate();
                {
                    const tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env.package_database()->query(query::Repository(RepositoryName("installed-unpackaged")), qo_order_by_version));
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "");
                }
            }
        }
    } test_multiple_ops;
}

