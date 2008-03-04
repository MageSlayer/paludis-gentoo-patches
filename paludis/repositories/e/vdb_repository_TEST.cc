/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/options.hh>
#include <paludis/query.hh>
#include <paludis/dep_spec.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/action.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <fstream>
#include <iterator>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VDBRepository.
 *
 */

namespace test_cases
{
    /**
     * \test Test VDBRepository repo names
     *
     */
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    /**
     * \test Test VDBRepository has_category_named
     *
     */
    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    /**
     * \test Test VDBRepository query_use
     *
     */
    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<const PackageID> e1(*env.package_database()->query(query::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1", UserPackageDepSpecOptions()))),
                        qo_require_exactly_one)->begin());

            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag1"), *e1) == use_enabled);
            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag2"), *e1) == use_enabled);
            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag3"), *e1) == use_disabled);
        }
    } test_vdb_repository_query_use;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNewFileTest : TestCase
    {
        VDBRepositoryAddToWorldNewFileTest() : TestCase("add to world (new file)") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-new-file");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            (*repo)[k::world_interface()]->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-new-file");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_new_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldEmptyFileTest : TestCase
    {
        VDBRepositoryAddToWorldEmptyFileTest() : TestCase("add to world (empty file)") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-empty");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            (*repo)[k::world_interface()]->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-empty");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_empty_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchTest() : TestCase("add to world (no match)") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            (*repo)[k::world_interface()]->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldMatchTest : TestCase
    {
        VDBRepositoryAddToWorldMatchTest() : TestCase("add to world (match)") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-match");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            (*repo)[k::world_interface()]->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/foofoo\ncat-one/bar\n");
        }
    } test_vdb_repository_add_to_world_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchNoEOLTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchNoEOLTest() : TestCase("add to world (no match, no trailing eol)") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match-no-eol");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            (*repo)[k::world_interface()]->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match-no-eol");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match_no_eol;

    /**
     * \test Test VDBRepository CONTENTS.
     */
    struct VDBRepositoryContentsTest : TestCase
    {
        VDBRepositoryContentsTest() : TestCase("CONTENTS") { }

        struct ContentsGatherer :
            ConstVisitor<ContentsVisitorTypes>
        {
            std::string _str;

            void visit(const ContentsFileEntry & e)
            {
                _str += "file\n";
                _str += stringify(e.name());
                _str += '\n';
            }

            void visit(const ContentsDirEntry & e)
            {
                _str += "directory\n";
                _str += stringify(e.name());
                _str += '\n';
            }

            void visit(const ContentsSymEntry & e)
            {
                _str += "symlink\n";
                _str += stringify(e.name());
                _str += '\n';
                _str += stringify(e.target());
                _str += '\n';
            }

            void visit(const ContentsMiscEntry & e)
            {
                _str += "miscellaneous\n";
                _str += stringify(e.name());
                _str += '\n';
            }

            void visit(const ContentsFifoEntry & e)
            {
                _str += "fifo\n";
                _str += stringify(e.name());
                _str += '\n';
            }

            void visit(const ContentsDevEntry & e)
            {
                _str += "device\n";
                _str += stringify(e.name());
                _str += '\n';
            }
        };

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match-no-eol");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<const PackageID> e1(*env.package_database()->query(query::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1", UserPackageDepSpecOptions()))),
                        qo_require_exactly_one)->begin());
            ContentsGatherer gatherer;
            std::for_each(indirect_iterator(e1->contents_key()->value()->begin()),
                          indirect_iterator(e1->contents_key()->value()->end()),
                          accept_visitor(gatherer));
            TEST_CHECK_EQUAL(gatherer._str,
                             "directory\n/directory\n"
                             "file\n/directory/file\n"
                             "symlink\n/directory/symlink\ntarget\n"
                             "directory\n/directory with spaces\n"
                             "directory\n/directory with trailing space \n"
                             "directory\n/directory  with  consecutive  spaces\n"
                             "file\n/file with spaces\n"
                             "file\n/file  with  consecutive  spaces\n"
                             "file\n/file with  trailing   space\t \n"
                             "symlink\n/symlink\ntarget  with  consecutive  spaces\n"
                             "symlink\n/symlink with spaces\ntarget with spaces\n"
                             "symlink\n/symlink  with  consecutive  spaces\ntarget  with  consecutive  spaces\n"
                             "symlink\n/symlink\ntarget -> with -> multiple -> arrows\n"
                             "symlink\n/symlink\ntarget with trailing space \n"
                             "symlink\n/symlink\n target with leading space\n"
                             "symlink\n/symlink with trailing space \ntarget\n"
                             "fifo\n/fifo\n"
                             "fifo\n/fifo with spaces\n"
                             "fifo\n/fifo  with  consecutive  spaces\n"
                             "device\n/device\n"
                             "device\n/device with spaces\n"
                             "device\n/device  with  consecutive  spaces\n"
                             "miscellaneous\n/miscellaneous\n"
                             "miscellaneous\n/miscellaneous with spaces\n"
                             "miscellaneous\n/miscellaneous  with  consecutive  spaces\n");
        }
    } vdb_repository_contents_test;

    struct VDBRepositoryDependenciesRewriterTest : TestCase
    {
        VDBRepositoryDependenciesRewriterTest() : TestCase("dependencies_rewriter") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo2");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);

            const tr1::shared_ptr<const PackageID> id(*env.package_database()->
                    query(query::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->build_dependencies_key());
            id->build_dependencies_key()->value()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "cat/pkg1 build: cat/pkg2 build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pr(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->run_dependencies_key());
            id->run_dependencies_key()->value()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "cat/pkg1 build: build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pp(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->post_dependencies_key());
            id->post_dependencies_key()->value()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "build: build,run: suggested: post: cat/pkg5");
        }
    } test_vdb_repository_dependencies_rewriter;

    struct InstallReinstallUninstallTest : TestCase
    {
        InstallReinstallUninstallTest() : TestCase("install / reinstall / uninstall") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/srcrepo");
            keys->insert("profiles", "vdb_repository_TEST_dir/srcrepo/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", "vdb_repository_TEST_dir/root");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo3");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", "vdb_repository_TEST_dir/root");
            tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env, keys));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::no_config_protect(), false)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action(UninstallActionOptions::named_create()
                    (k::no_config_protect(), false)
                    );

            {
                TestMessageSuffix suffix("install", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-1::srcrepo",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-1::srcrepo",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("uninstall", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-1::installed",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(uninstall_action);
            }
        }
    } test_vdb_install_reinstall_uninstall;

    struct VarsTest : TestCase
    {
        VarsTest() : TestCase("vars") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/srcrepo");
            keys->insert("profiles", "vdb_repository_TEST_dir/srcrepo/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", "vdb_repository_TEST_dir/root");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo3");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", "vdb_repository_TEST_dir/root");
            tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env, keys));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::no_config_protect(), false)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action(UninstallActionOptions::named_create()
                    (k::no_config_protect(), false)
                    );

            {
                TestMessageSuffix suffix("vars", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/vars-1::srcrepo",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/vars-1::srcrepo",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("uninstall", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/vars-1::installed",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(uninstall_action);
            }
        }
    } test_vdb_vars;
}

