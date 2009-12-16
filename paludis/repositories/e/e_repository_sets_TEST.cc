/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/stringify_formatter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include "config.h"

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
    struct ERepositorySetsSetsListTest : TestCase
    {
        ERepositorySetsSetsListTest() : TestCase("sets list") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_sets_TEST_dir/repo1"));
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<const SetNameSet> sets_list(env.set_names());
            TEST_CHECK_EQUAL(join(sets_list->begin(), sets_list->end(), " "), "everything insecurity "
                    "insecurity::test-repo-1 "
                    "installed-packages installed-packages::default "
                    "installed-slots installed-slots::default "
                    "security security::test-repo-1 set1 set1* "
                    "set1::test-repo-1 set1::test-repo-1* "
                    "system system::test-repo-1 world world::default");
        }
    } test_e_repository_sets_sets_list;

    struct ERepositorySetsMaintainerDefinedSetsTest : TestCase
    {
        ERepositorySetsMaintainerDefinedSetsTest() : TestCase("maintainer-defined sets") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_sets_TEST_dir/repo1"));
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            std::tr1::shared_ptr<FakeInstalledRepository> installed(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("installed")),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                            )));
            installed->add_version("cat-two", "bar", "1.5");
            env.package_database()->add_repository(0, installed);
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<const SetSpecTree> set1(env.set(SetName("set1::test-repo-1")));
            TEST_CHECK(set1);
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            set1->root()->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "cat-one/foo >=cat-two/bar-2");
        }
    } test_e_repository_sets_maintainer_defined_sets_list;

#if ENABLE_XML
    struct ERepositorySetsInsecuritySetTest : TestCase
    {
        ERepositorySetsInsecuritySetTest() : TestCase("insecurity set") { }

        virtual unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_sets_TEST_dir/repo1"));
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<const SetSpecTree> insecurity(env.set(SetName("insecurity::test-repo-1")));
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            insecurity->root()->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-four/xyzzy-2.0.1::test-repo-1 =cat-four/xyzzy-2.0.2::test-repo-1 =cat-one/foo-1::test-repo-1 =cat-two/bar-1.5::test-repo-1 "
                                       "=cat-two/bar-1.5.1::test-repo-1 =cat-three/baz-1.0::test-repo-1 "
                                       "=cat-three/baz-1.1-r2::test-repo-1 =cat-three/baz-1.2::test-repo-1");
        }
    } test_e_repository_sets_insecurity_set;

    struct ERepositorySetsSecuritySetTest : TestCase
    {
        ERepositorySetsSecuritySetTest() : TestCase("security set") { }

        virtual unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_sets_TEST_dir/repo1"));
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<FakeInstalledRepository> installed(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("installed")),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                            )));
            installed->add_version("cat-one", "foo", "2.1");
            installed->add_version("cat-two", "bar", "1.5");
            installed->add_version("cat-three", "baz", "1.0");
            installed->add_version("cat-four", "xyzzy", "1.1.0")->set_slot(SlotName("1"));
            installed->add_version("cat-four", "xyzzy", "2.0.1")->set_slot(SlotName("2"));
            env.package_database()->add_repository(0, installed);

            std::tr1::shared_ptr<const SetSpecTree> security(env.set(SetName("security::test-repo-1")));
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            security->root()->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-four/xyzzy-2.0.3::test-repo-1 =cat-two/bar-2.0::test-repo-1 =cat-three/baz-1.3::test-repo-1");
        }
    } test_e_repository_sets_security_set;
#endif
}

