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
#include <paludis/metadata_key.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
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

#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace test;
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
}

namespace test_cases
{
    struct NamesCacheIncrementalTest : TestCase
    {
        FSPath names_cache;

        NamesCacheIncrementalTest() :
            TestCase("names cache incremental"),
            names_cache(stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "namesincrtest/.cache/names/installed"))
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

        void install(const Environment & env,
                const std::shared_ptr<Repository> & vdb_repo,
                const std::string & chosen_one,
                const std::string & victim) const
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

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "namesincrtest_src"));
            keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "namesincrtest_src/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(1, repo);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", stringify(names_cache.dirname()));
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "namesincrtest"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(0, vdb_repo);

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            {
                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }

            {
                TestMessageSuffix suffix("install", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::namesincrtest_src", "=cat1/pkg1-1::installed");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1::namesincrtest_src", "=cat1/pkg1-1.1::installed");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                install(env, vdb_repo, "=cat1/pkg1-2::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                install(env, vdb_repo, "=cat1/pkg2-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 2U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(cache_contents.back().basename(), "pkg2");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg2"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("new category", true);
                install(env, vdb_repo, "=cat2/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\ncat2\n");
            }

            {
                TestMessageSuffix suffix("remove other category", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat2/pkg1-1::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat1\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 0U);
            }

            {
                TestMessageSuffix suffix("install paludis-1", true);
                install(env, vdb_repo, "=cat3/pkg1-1::namesincrtest_src", "");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat3\n");
            }

            {
                TestMessageSuffix suffix("upgrade paludis-1", true);
                install(env, vdb_repo, "=cat3/pkg1-2::namesincrtest_src", "=cat3/pkg1-1::installed");
                vdb_repo->invalidate();

                std::vector<FSPath> cache_contents;
                read_cache(cache_contents);
                TEST_CHECK_EQUAL(cache_contents.size(), 1U);
                TEST_CHECK_EQUAL(cache_contents.front().basename(), "pkg1");
                TEST_CHECK_EQUAL(read_file(names_cache / "pkg1"), "cat3\n");
            }
        }

        void read_cache(std::vector<FSPath> & vec)
        {
            using namespace std::placeholders;
            std::remove_copy_if(FSIterator(names_cache, { fsio_include_dotfiles }),
                                FSIterator(), std::back_inserter(vec),
                                std::bind(&std::equal_to<std::string>::operator(),
                                          std::equal_to<std::string>(),
                                          "_VERSION_", std::bind(&FSPath::basename, _1)));
        }

        std::string read_file(const FSPath & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_names_cache_incremental;

    struct ProvidesCacheTest : TestCase
    {
        FSPath provides_cache;

        ProvidesCacheTest() :
            TestCase("provides cache"),
            provides_cache(stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providestest/.cache/provides"))
        {
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providestest"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(0, vdb_repo);

            TEST_CHECK(! provides_cache.stat().exists());

            {
                std::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq(vdb_repo->provides_interface()->provided_packages());

                for (RepositoryProvidesInterface::ProvidesSequence::ConstIterator s(seq->begin()), s_end(seq->end()) ;
                        s != s_end ; ++s)
                    TestMessageSuffix x(stringify(s->virtual_name()) + " by " + stringify(*s->provided_by()), true);

                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5);

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-2:2::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-2:2::installed");
            }

            vdb_repo->regenerate_cache();
            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\ncat1/pkg1 2 virtual/foo\ncat1/pkg2 1 virtual/foo virtual/bar\ncat1/pkg2 2 virtual/bar\n");
            vdb_repo->invalidate();

            {
                std::shared_ptr<const RepositoryProvidesInterface::ProvidesSequence> seq(vdb_repo->provides_interface()->provided_packages());
                TEST_CHECK_EQUAL(std::distance(seq->begin(), seq->end()), 5);

                for (auto i(seq->begin()), i_end(seq->end()) ;
                        i != i_end ; ++i)
                    TEST_CHECK(! stringify(i->provided_by()->slot_key()->value()).empty());

                RepositoryProvidesInterface::ProvidesSequence::ConstIterator it(seq->begin());
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg1-2:2::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/foo");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-1:1::installed");
                TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/bar");
                TEST_CHECK_STRINGIFY_EQUAL(*(*it++).provided_by(), "cat1/pkg2-2:2::installed");
            }
        }

        std::string read_file(const FSPath & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache;

    struct ProvidesCacheIncrementalTest : TestCase
    {
        FSPath provides_cache;

        ProvidesCacheIncrementalTest() :
            TestCase("provides cache incremental"),
            provides_cache(stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest/.cache/provides"))
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

        void install(const Environment & env,
                const std::shared_ptr<Repository> & vdb_repo,
                const std::string & chosen_one,
                const std::string & victim) const
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

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest_src1"));
            keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> repo1(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(1, repo1);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest_src2"));
            keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest_src1/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> repo2(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(2, repo2);

            keys = std::make_shared<Map<std::string, std::string>>();
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", stringify(provides_cache));
            keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "providesincrtest"));
            keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_cache_dir" / "build"));
            keys->insert("root", stringify(FSPath("vdb_repository_TEST_cache_dir/root").realpath()));
            std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.add_repository(0, vdb_repo);

            UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                        n::config_protect() = "",
                        n::if_for_install_id() = make_null_shared_ptr(),
                        n::ignore_for_unmerge() = &ignore_nothing,
                        n::is_overwrite() = false,
                        n::make_output_manager() = &make_standard_output_manager
                    ));

            TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");

            {
                TestMessageSuffix suffix("install", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("upgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::providesincrtest_src1", "=cat1/pkg1-1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall equivalent", true);
                install(env, vdb_repo, "=cat1/pkg1-1.1::providesincrtest_src2", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1.1-r0 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("downgrade", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src1", "=cat1/pkg1-1.1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("reinstall different PROVIDE", true);
                install(env, vdb_repo, "=cat1/pkg1-1::providesincrtest_src2", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new slot", true);
                install(env, vdb_repo, "=cat1/pkg1-2::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg1 2 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other slot", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-2::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("new package", true);
                install(env, vdb_repo, "=cat1/pkg2-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\ncat1/pkg2 1 virtual/foo\n");
            }

            {
                TestMessageSuffix suffix("remove other package", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg2-1::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat1/pkg1 1 virtual/bar\n");
            }

            {
                TestMessageSuffix suffix("uninstall", true);
                const std::shared_ptr<const PackageID> inst_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat1/pkg1-1::installed",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                inst_id->perform_action(uninstall_action);
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\n");
            }

            {
                TestMessageSuffix suffix("install paludis-1", true);
                install(env, vdb_repo, "=cat2/pkg1-1::providesincrtest_src1", "");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat2/pkg1 1 virtual/moo\n");
            }

            {
                TestMessageSuffix suffix("upgrade paludis-1", true);
                install(env, vdb_repo, "=cat2/pkg1-2::providesincrtest_src1", "=cat2/pkg1-1::installed");
                vdb_repo->invalidate();

                TEST_CHECK_EQUAL(read_file(provides_cache), "paludis-3\ninstalled\ncat2/pkg1 2 virtual/moo\n");
            }
        }

        std::string read_file(const FSPath & f)
        {
            SafeIFStream s(f);
            std::stringstream ss;
            std::copy(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(ss));
            return ss.str();
        }
    } test_provides_cache_incremental;
}

