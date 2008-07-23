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
#include <paludis/util/dir_iterator.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/action.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/functional>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iterator>
#include <vector>

using namespace test;
using namespace paludis;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

namespace test_cases
{
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag1"), *e1) == use_enabled);
            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag2"), *e1) == use_enabled);
            TEST_CHECK((*repo)[k::use_interface()]->query_use(UseFlagName("flag3"), *e1) == use_disabled);
        }
    } test_vdb_repository_query_use;

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
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match-no-eol");
            std::tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());
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
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo2");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->build_dependencies_key());
            id->build_dependencies_key()->value()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "cat/pkg1 build: cat/pkg2 build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pr(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->run_dependencies_key());
            id->run_dependencies_key()->value()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "cat/pkg1 build: build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pp(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->post_dependencies_key());
            id->post_dependencies_key()->value()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "build: build,run: suggested: post: cat/pkg5");
        }
    } test_vdb_repository_dependencies_rewriter;

    struct PhasesTest : TestCase
    {
        const std::string eapi;

        PhasesTest(const std::string & e) :
            TestCase("phases eapi " + e),
            eapi(e)
        {
        }

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
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/srcrepo");
            keys->insert("profiles", "vdb_repository_TEST_dir/srcrepo/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", eapi);
            keys->insert("eapi_when_unspecified", eapi);
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo3");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action;

            InfoAction info_action;
            ConfigAction config_action;

            {
                TestMessageSuffix suffix("install", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("info", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(info_action);
            }

            {
                TestMessageSuffix suffix("config", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(config_action);
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(uninstall_action);
            }
        }
    } test_phases_eapi_0("0"), test_phases_eapi_1("1"), test_phases_eapi_exheres_0("exheres-0"),
                        test_phases_eapi_kdebuild_1("kdebuild-1");

    struct VarsTest : TestCase
    {
        const std::string eapi;

        VarsTest(const std::string & e) :
            TestCase("vars eapi " + e),
            eapi(e)
        {
        }

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
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/srcrepo");
            keys->insert("profiles", "vdb_repository_TEST_dir/srcrepo/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", eapi);
            keys->insert("eapi_when_unspecified", eapi);
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo3");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action;

            InfoAction info_action;
            ConfigAction config_action;

            {
                TestMessageSuffix suffix("vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(install_action);
            }

            vdb_repo->invalidate();

            {
                TestMessageSuffix suffix("info", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(info_action);
            }

            {
                TestMessageSuffix suffix("config", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(config_action);
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                TEST_CHECK(id);
                id->perform_action(uninstall_action);
            }
        }
    } test_vdb_vars_eapi_0("0"), test_vdb_vars_eapi_1("1"), test_vdb_vars_eapi_exheres_0("exheres-0"),
                      test_vdb_vars_eapi_kdebuild_1("kdebuild-1");

    struct NamesCacheIncrementalTest : TestCase
    {
        FSEntry names_cache;

        NamesCacheIncrementalTest() :
            TestCase("names cache incremental"),
            names_cache("vdb_repository_TEST_dir/namesincrtest/.cache/names/installed")
        {
        }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/namesincrtest_src");
            keys->insert("profiles", "vdb_repository_TEST_dir/namesincrtest_src/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", stringify(names_cache.dirname()));
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/namesincrtest");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action;

            {
                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }

            {
                TestMessageSuffix suffix("install", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1.1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1.1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 2U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(cache_contents.back().basename(), "pkg2");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg2"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new category", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat2/pkg1-1::namesincrtest_src",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\ncat2\n");
            }

            {
                TestMessageSuffix suffix("remove other category", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat2/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSEntry> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }
        }

        void read_cache(std::vector<FSEntry> & vec)
        {
            using namespace std::tr1::placeholders;
            std::remove_copy_if(DirIterator(names_cache, DirIteratorOptions() + dio_include_dotfiles),
                                DirIterator(), std::back_inserter(vec),
                                std::tr1::bind(&std::equal_to<std::string>::operator(),
                                          std::equal_to<std::string>(),
                                          "_VERSION_", std::tr1::bind(&FSEntry::basename, _1)));
        }

        std::string read_file(const FSEntry & f)
        {
            std::ifstream s(stringify(f).c_str());
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_names_cache_incremental;

    struct ProvidesCacheTest : TestCase
    {
        FSEntry provides_cache;

        ProvidesCacheTest() :
            TestCase("provides cache"),
            provides_cache("vdb_repository_TEST_dir/providestest/.cache/provides")
        {
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", "vdb_repository_TEST_dir/providestest");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            TEST_CHECK(! provides_cache.exists());

            {
                std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq((*vdb_repo)[k::provides_interface()]->provided_packages());
                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5U);

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg1-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg1-2::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-2::installed");
            }

            vdb_repo->regenerate_cache();
            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\ncat1/pkg1 2 virtual/foo\ncat1/pkg2 1 virtual/foo virtual/bar\ncat1/pkg2 2 virtual/bar\n");
            vdb_repo->invalidate();

            {
                std::tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq((*vdb_repo)[k::provides_interface()]->provided_packages());
                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5U);

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg1-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg1-2::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-1::installed");
                TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++)[k::provided_by()], "cat1/pkg2-2::installed");
            }
        }

        std::string read_file(const FSEntry & f)
        {
            std::ifstream s(stringify(f).c_str());
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache;

    struct ProvidesCacheIncrementalTest : TestCase
    {
        FSEntry provides_cache;

        ProvidesCacheIncrementalTest() :
            TestCase("provides cache incremental"),
            provides_cache("vdb_repository_TEST_dir/providesincrtest/.cache/provides")
        {
        }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/providesincrtest_src1");
            keys->insert("profiles", "vdb_repository_TEST_dir/providesincrtest_src1/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo1(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/providesincrtest_src2");
            keys->insert("profiles", "vdb_repository_TEST_dir/providesincrtest_src1/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo2(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(2, repo2);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", "vdb_repository_TEST_dir/providesincrtest");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), vdb_repo)
                    );

            UninstallAction uninstall_action;

            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");

            {
                TestMessageSuffix suffix("install", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1.1::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall equivalent", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1.1::providesincrtest_src2",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1-r0 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1.1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall different PROVIDE", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::providesincrtest_src2",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg1 2 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::providesincrtest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg2 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::tr1::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");
            }
        }

        std::string read_file(const FSEntry & f)
        {
            std::ifstream s(stringify(f).c_str());
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache_incremental;

    struct ReinstallTest : TestCase
    {
        ReinstallTest() : TestCase("reinstall") { }

        bool repeatable() const
        {
            return false;
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/reinstalltest_src1");
            keys->insert("profiles", "vdb_repository_TEST_dir/reinstalltest_src1/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo1(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo1);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/reinstalltest_src2");
            keys->insert("profiles", "vdb_repository_TEST_dir/reinstalltest_src1/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo2(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(2, repo2);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/reinstalltest");
            keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("vdb_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> vdb_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(0, vdb_repo);

            InstallAction install_action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), vdb_repo)
                    );

            TEST_CHECK(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"))->empty());

            {
                TestMessageSuffix suffix("install", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1::installed");
            }

            {
                TestMessageSuffix suffix("reinstall equivalent", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src2",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                id->perform_action(install_action);
                vdb_repo->invalidate();

                std::tr1::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg")));
                TEST_CHECK_EQUAL(join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "), "cat/pkg-1-r0::installed");
            }
        }
    } reinstall_test;
}

