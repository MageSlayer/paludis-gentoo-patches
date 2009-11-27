/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/user_dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/join.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>
#include <sstream>

using namespace test;
using namespace paludis;

namespace
{
    void cannot_uninstall(const std::tr1::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::tr1::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return make_shared_ptr(new StandardOutputManager);
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

    bool ignore_nothing(const FSEntry &)
    {
        return false;
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
            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo1")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root"))
                        )));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageIDSequence> ids(
                    env[selection::AllVersionsSorted(generator::All())]);
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
            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo1")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root"))
                        )));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:0",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            TEST_CHECK_EQUAL(id1->version(), VersionSpec("1", VersionSpecOptions()));
            TEST_CHECK_EQUAL(id1->slot_key()->value(), SlotName("0"));
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

            const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            TEST_CHECK_EQUAL(id2->version(), VersionSpec("2", VersionSpecOptions()));
            TEST_CHECK_EQUAL(id2->slot_key()->value(), SlotName("1"));
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
            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo1")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root"))
                        )));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:0",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            TEST_CHECK(! id1->masked());

            const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            TEST_CHECK(! id2->masked());
        }
    } test_masks;

    struct ActionsTest : TestCase
    {
        ActionsTest() : TestCase("actions") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo1")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root"))
                        )));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! repo->some_ids_might_support_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(repo->some_ids_might_support_action(SupportsActionTest<UninstallAction>()));

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:1",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            TEST_CHECK(! id1->supports_action(SupportsActionTest<InstallAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<ConfigAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<PretendAction>()));
            TEST_CHECK(! id1->supports_action(SupportsActionTest<InfoAction>()));
            TEST_CHECK(id1->supports_action(SupportsActionTest<UninstallAction>()));
        }
    } test_actions;

    struct UninstallLastTest : TestCase
    {
        UninstallLastTest() : TestCase("uninstall last") { }

        void run()
        {
            TestEnvironment env;

            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo2")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root2"))
                        )));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
            TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                    "cat-one/foo-1.2.3:fred::installed-unpackaged");

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/first").is_regular_file());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/second").is_regular_file());

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").is_symbolic_link());

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::All())]->begin());

            UninstallAction action(make_named_values<UninstallActionOptions>(
                        value_for<n::config_protect>(""),
                        value_for<n::if_for_install_id>(make_null_shared_ptr()),
                        value_for<n::ignore_for_unmerge>(&ignore_nothing),
                        value_for<n::is_overwrite>(false),
                        value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
            id->perform_action(action);

            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root2/first").exists());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/root2/second").is_regular_file());

            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/repo2/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(! FSEntry("installed_repository_TEST_dir/repo2/indices/packages/foo/cat-one").is_symbolic_link());

            repo->invalidate();

            const std::tr1::shared_ptr<const PackageIDSequence> post_ids(env[selection::AllVersionsSorted(generator::All())]);
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

            std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                        RepositoryName("installed-unpackaged"),
                        make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo3")),
                            value_for<n::root>(FSEntry("installed_repository_TEST_dir/root3"))
                        )));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
            TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                    "cat-one/foo-1.2.3:fred::installed-unpackaged cat-one/foo-3.2.1:barney::installed-unpackaged");

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").is_symbolic_link());

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(
                        generator::Matches(parse_user_package_dep_spec("cat-one/foo:fred",
                                &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());

            UninstallAction action(make_named_values<UninstallActionOptions>(
                        value_for<n::config_protect>(""),
                        value_for<n::if_for_install_id>(make_null_shared_ptr()),
                        value_for<n::ignore_for_unmerge>(&ignore_nothing),
                        value_for<n::is_overwrite>(false),
                        value_for<n::make_output_manager>(&make_standard_output_manager)
                    ));
            id->perform_action(action);

            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/categories/cat-one/foo").is_symbolic_link());
            TEST_CHECK(FSEntry("installed_repository_TEST_dir/repo3/indices/packages/foo/cat-one").is_symbolic_link());

            repo->invalidate();

            const std::tr1::shared_ptr<const PackageIDSequence> post_ids(env[selection::AllVersionsSorted(generator::All())]);
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
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(1, repo);

                const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(generator::All())]);
                TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "), "");

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").exists());
            }

            {
                TestMessageSuffix suffix("install 4a", true);

                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(0, repo);

                std::tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                                value_for<n::build_dependencies>(""),
                                value_for<n::description>(""),
                                value_for<n::environment>(&env),
                                value_for<n::install_under>(FSEntry("/")),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/src4a")),
                                value_for<n::name>(QualifiedPackageName("cat/pkg4a")),
                                value_for<n::rewrite_ids_over_to_root>(-1),
                                value_for<n::run_dependencies>(""),
                                value_for<n::slot>(SlotName("foo")),
                                value_for<n::version>(VersionSpec("1.0", VersionSpecOptions()))
                            )));
                env.package_database()->add_repository(1, source_repo);

                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "), "");
                }

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").exists());

                InstallAction action(make_named_values<InstallActionOptions>(
                            value_for<n::destination>(repo),
                            value_for<n::make_output_manager>(&make_standard_output_manager),
                            value_for<n::perform_uninstall>(&cannot_uninstall),
                            value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                            value_for<n::want_phase>(&want_all_phases)
                        ));
                (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());

                repo->invalidate();
                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("install 4b1", true);

                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(0, repo);

                std::tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                                value_for<n::build_dependencies>(""),
                                value_for<n::description>(""),
                                value_for<n::environment>(&env),
                                value_for<n::install_under>(FSEntry("/")),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/src4b1")),
                                value_for<n::name>(QualifiedPackageName("cat/pkg4b")),
                                value_for<n::rewrite_ids_over_to_root>(-1),
                                value_for<n::run_dependencies>(""),
                                value_for<n::slot>(SlotName("foo")),
                                value_for<n::version>(VersionSpec("1.0", VersionSpecOptions()))
                                )));
                env.package_database()->add_repository(1, source_repo);

                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged");
                }

                InstallAction action(make_named_values<InstallActionOptions>(
                            value_for<n::destination>(repo),
                            value_for<n::make_output_manager>(&make_standard_output_manager),
                            value_for<n::perform_uninstall>(&cannot_uninstall),
                            value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                            value_for<n::want_phase>(&want_all_phases)
                        ));
                (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("install 4b2", true);

                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(0, repo);

                std::tr1::shared_ptr<Repository> source_repo(new UnpackagedRepository(
                            RepositoryName("unpackaged"),
                            make_named_values<unpackaged_repositories::UnpackagedRepositoryParams>(
                                value_for<n::build_dependencies>(""),
                                value_for<n::description>(""),
                                value_for<n::environment>(&env),
                                value_for<n::install_under>(FSEntry("/")),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/src4b2")),
                                value_for<n::name>(QualifiedPackageName("cat/pkg4b")),
                                value_for<n::rewrite_ids_over_to_root>(-1),
                                value_for<n::run_dependencies>(""),
                                value_for<n::slot>(SlotName("foo")),
                                value_for<n::version>(VersionSpec("1.0", VersionSpecOptions()))
                            )));
                env.package_database()->add_repository(1, source_repo);

                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                InstallAction action(make_named_values<InstallActionOptions>(
                            value_for<n::destination>(repo),
                            value_for<n::make_output_manager>(&make_standard_output_manager),
                            value_for<n::perform_uninstall>(&cannot_uninstall),
                            value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                            value_for<n::want_phase>(&want_all_phases)
                        ));
                (*env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("unpackaged")))]->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("uninstall 4a", true);

                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(0, repo);

                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4a-1.0:foo::installed-unpackaged cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                UninstallAction action(make_named_values<UninstallActionOptions>(
                            value_for<n::config_protect>(""),
                            value_for<n::if_for_install_id>(make_null_shared_ptr()),
                            value_for<n::ignore_for_unmerge>(&ignore_nothing),
                            value_for<n::is_overwrite>(false),
                            value_for<n::make_output_manager>(&make_standard_output_manager)
                        ));
                (*env[selection::RequireExactlyOne(generator::Matches(
                        parse_user_package_dep_spec("cat/pkg4a",
                            &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin())->perform_action(action);

                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4a").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b").is_regular_file());
                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir/4b1").is_regular_file());
                TEST_CHECK(FSEntry("installed_repository_TEST_dir/root4/dir/4b2").is_regular_file());

                repo->invalidate();
                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(
                            env[selection::RequireExactlyOne(generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4b-1.0:foo::installed-unpackaged");
                }
            }

            {
                TestMessageSuffix suffix("uninstall 4b", true);

                TestEnvironment env;
                std::tr1::shared_ptr<Repository> repo(new InstalledUnpackagedRepository(
                            RepositoryName("installed-unpackaged"),
                            make_named_values<unpackaged_repositories::InstalledUnpackagedRepositoryParams>(
                                value_for<n::environment>(&env),
                                value_for<n::location>(FSEntry("installed_repository_TEST_dir/repo4")),
                                value_for<n::root>(FSEntry("installed_repository_TEST_dir/root4"))
                            )));
                env.package_database()->add_repository(0, repo);

                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "cat/pkg4b-1.0:foo::installed-unpackaged");
                }

                UninstallAction action(make_named_values<UninstallActionOptions>(
                            value_for<n::config_protect>(""),
                            value_for<n::if_for_install_id>(make_null_shared_ptr()),
                            value_for<n::ignore_for_unmerge>(&ignore_nothing),
                            value_for<n::is_overwrite>(false),
                            value_for<n::make_output_manager>(&make_standard_output_manager)
                        ));
                (*env[selection::RequireExactlyOne(generator::Matches(
                        parse_user_package_dep_spec("cat/pkg4b",
                            &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin())->perform_action(action);

                TEST_CHECK(! FSEntry("installed_repository_TEST_dir/root4/dir").is_directory());

                repo->invalidate();
                {
                    const std::tr1::shared_ptr<const PackageIDSequence> pre_ids(env[selection::AllVersionsSorted(
                                generator::InRepository(RepositoryName("installed-unpackaged")))]);
                    TEST_CHECK_EQUAL(join(indirect_iterator(pre_ids->begin()), indirect_iterator(pre_ids->end()), " "),
                            "");
                }
            }
        }

        bool repeatable() const
        {
            return false;
        }
    } test_multiple_ops;
}

