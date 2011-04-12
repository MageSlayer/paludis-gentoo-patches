/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/join.hh>

#include <paludis/metadata_key.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/unformatted_pretty_printer.hh>
#include <paludis/contents.hh>

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void do_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions & u)
    {
        UninstallAction a(u);
        id->perform_action(a);
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }

    struct ContentsGatherer
    {
        std::string _str;

        void visit(const ContentsFileEntry & e)
        {
            _str += "file\n";
            _str += stringify(e.location_key()->parse_value());
            _str += '\n';
        }

        void visit(const ContentsDirEntry & e)
        {
            _str += "directory\n";
            _str += stringify(e.location_key()->parse_value());
            _str += '\n';
        }

        void visit(const ContentsSymEntry & e)
        {
            _str += "symlink\n";
            _str += stringify(e.location_key()->parse_value());
            _str += '\n';
            _str += stringify(e.target_key()->parse_value());
            _str += '\n';
        }

        void visit(const ContentsOtherEntry & e)
        {
            _str += "other\n";
            _str += stringify(e.location_key()->parse_value());
            _str += '\n';
        }
    };

    void install(const Environment & env,
            const std::shared_ptr<Repository> & vdb_repo,
            const std::string & chosen_one,
            const std::string & victim)
    {
        std::shared_ptr<PackageIDSequence> replacing(std::make_shared<PackageIDSequence>());
        if (! victim.empty())
            replacing->push_back(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(victim,
                            &env, { })), make_null_shared_ptr(), { }))]->begin());
        InstallAction install_action(make_named_values<InstallActionOptions>(
                    n::destination() = vdb_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &do_uninstall,
                    n::replacing() = replacing,
                    n::want_phase() = &want_all_phases
                ));
        (*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(chosen_one,
                            &env, { })), make_null_shared_ptr(), { }))]->begin())->perform_action(install_action);
    }
}

TEST(VDBRepository, RepoName)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "repo1"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    EXPECT_EQ("installed", stringify(repo->name()));
}

TEST(VDBRepository, HasCategoryNamed)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "repo1"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    EXPECT_TRUE(repo->has_category_named(CategoryNamePart("cat-one"), { }));
    EXPECT_TRUE(repo->has_category_named(CategoryNamePart("cat-two"), { }));
    EXPECT_TRUE(! repo->has_category_named(CategoryNamePart("cat-three"), { }));
}

TEST(VDBRepository, QueryUse)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "repo1"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(e1->choices_key()));
    ASSERT_TRUE(bool(e1->choices_key()->parse_value()));
    ASSERT_TRUE(bool(e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag1"))));
    EXPECT_TRUE(e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag1"))->enabled());
    EXPECT_TRUE(e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag2"))->enabled());
    EXPECT_TRUE(! e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("flag3"))->enabled());
    EXPECT_TRUE(e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("test"))->enabled());
    EXPECT_TRUE(e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("kernel_linux"))->enabled());
    EXPECT_TRUE(! e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("test2")));
    EXPECT_TRUE(! e1->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix("kernel_freebsd")));
}

TEST(VDBRepository, Contents)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "repo1"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("world", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "world-no-match-no-eol"));
    std::shared_ptr<Repository> repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> e1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());
    ContentsGatherer gatherer;
    auto contents(e1->contents_key()->parse_value());
    std::for_each(indirect_iterator(contents->begin()),
                  indirect_iterator(contents->end()),
                  accept_visitor(gatherer));
    EXPECT_EQ(
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
            "other\n/fifo\n"
            "other\n/fifo with spaces\n"
            "other\n/fifo  with  consecutive  spaces\n"
            "other\n/device\n"
            "other\n/device with spaces\n"
            "other\n/device  with  consecutive  spaces\n"
            "other\n/miscellaneous\n"
            "other\n/miscellaneous with spaces\n"
            "other\n/miscellaneous  with  consecutive  spaces\n",
        gatherer._str);
}

TEST(VDBRepository, Reinstall)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo1);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src2"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "reinstalltest_src1/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> repo2(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(2, repo2);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "reinstalltest"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, vdb_repo);

    InstallAction install_action(make_named_values<InstallActionOptions>(
                n::destination() = vdb_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &do_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    EXPECT_TRUE(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { })->empty());

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                &env, { })), make_null_shared_ptr(), { }))]->begin());
        id->perform_action(install_action);
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src1",
                                &env, { })), make_null_shared_ptr(), { }))]->begin());
        id->perform_action(install_action);
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/pkg-1::reinstalltest_src2",
                                &env, { })), make_null_shared_ptr(), { }))]->begin());
        id->perform_action(install_action);
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1-r0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }
}

TEST(VDBRepository, PhaseOrdering)
{
    TestEnvironment env(FSPath(stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "root")).realpath());
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "postinsttest_src1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "postinsttest_src1/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo1);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "postinsttest"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, vdb_repo);

    EXPECT_TRUE(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { })->empty());

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-install-eapi-1", 1);
        install(env, vdb_repo, "=cat/pkg-0::postinsttest", "");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "0", 1);
        install(env, vdb_repo, "=cat/pkg-0::postinsttest", "");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-upgrade-eapi-1-1", 1);
        install(env, vdb_repo, "=cat/pkg-0.1::postinsttest", "=cat/pkg-0::installed");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids2(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0.1::installed", join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-upgrade-eapi-1-paludis-1", 1);
        install(env, vdb_repo, "=cat/pkg-1::postinsttest", "=cat/pkg-0.1::installed");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-reinstall-eapi-paludis-1", 1);
        install(env, vdb_repo, "=cat/pkg-1::postinsttest", "");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "1.1", 1);
        install(env, vdb_repo, "=cat/pkg-1.1::postinsttest", "=cat/pkg-1::installed");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-1.1::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-new-slot", 1);
        install(env, vdb_repo, "=cat/pkg-2::postinsttest", "");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-1.1::installed cat/pkg-2::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    {
        ::setenv("VDB_REPOSITORY_TEST_RMDIR", "x-downgrade-eapi-paludis-1-1", 1);
        install(env, vdb_repo, "=cat/pkg-0::postinsttest", "=cat/pkg-1.1::installed");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids2(env[selection::AllVersionsSorted(generator::Package(
                        QualifiedPackageName("cat/pkg")) & generator::InRepository(RepositoryName("installed")))]);
        EXPECT_EQ("cat/pkg-0::installed cat/pkg-2::installed", join(indirect_iterator(ids2->begin()), indirect_iterator(ids2->end()), " "));
    }
}

TEST(VDBRepository, RemoveStaleFiles)
{
    TestEnvironment env(FSPath(stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "root")).realpath());
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "removestalefiles"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "removestalefiles/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo1);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("provides_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "removestalefilesvdb"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_dir/root").realpath()));
    std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, vdb_repo);

    EXPECT_TRUE(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { })->empty());

    EXPECT_FALSE((FSPath("vdb_repository_TEST_dir/root") / "stale-first").stat().exists());
    EXPECT_FALSE((FSPath("vdb_repository_TEST_dir/root") / "stale-both").stat().exists());

    {
        ::setenv("VDB_REPOSITORY_TEST_STALE", "true", 1);
        install(env, vdb_repo, "=cat/pkg-0::removestalefiles", "");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    EXPECT_TRUE((FSPath("vdb_repository_TEST_dir/root") / "stale-first").stat().exists());
    EXPECT_TRUE((FSPath("vdb_repository_TEST_dir/root") / "stale-both").stat().exists());

    {
        ::setenv("VDB_REPOSITORY_TEST_STALE", "false", 1);
        install(env, vdb_repo, "=cat/pkg-0::removestalefiles", "=cat/pkg-0::installed");
        vdb_repo->invalidate();

        std::shared_ptr<const PackageIDSequence> ids(vdb_repo->package_ids(QualifiedPackageName("cat/pkg"), { }));
        EXPECT_EQ("cat/pkg-0::installed", join(indirect_iterator(ids->begin()), indirect_iterator(ids->end()), " "));
    }

    EXPECT_FALSE((FSPath("vdb_repository_TEST_dir/root") / "stale-first").stat().exists());
    EXPECT_TRUE((FSPath("vdb_repository_TEST_dir/root") / "stale-both").stat().exists());
}

