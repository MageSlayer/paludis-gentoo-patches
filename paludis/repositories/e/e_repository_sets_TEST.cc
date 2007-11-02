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
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/stringify_formatter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include "config.h"

using namespace test;
using namespace paludis;

/** \file
 * Test cases for ERepositorySets.
 *
 */

namespace test_cases
{
    /**
     * \test Test ERepositorySets sets list.
     *
     */
    struct ERepositorySetsSetsListTest : TestCase
    {
        ERepositorySetsSetsListTest() : TestCase("sets list") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const SetNameSet> sets_list(repo->sets_interface->sets_list());
            TEST_CHECK_EQUAL(sets_list->size(), 4U);
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("system")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("security")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("insecurity")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("set1")));
        }
    } test_e_repository_sets_sets_list;

    /**
     * \test Test ERepositorySets maintainer-defined sets.
     *
     */
    struct ERepositorySetsMaintainerDefinedSetsTest : TestCase
    {
        ERepositorySetsMaintainerDefinedSetsTest() : TestCase("maintainer-defined sets") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            tr1::shared_ptr<FakeInstalledRepository> installed(
                new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed->add_version("cat-two", "bar", "1.5");
            env.package_database()->add_repository(0, installed);

            tr1::shared_ptr<SetSpecTree::ConstItem> set1(repo->sets_interface->package_set(SetName("set1")));
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            set1->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "cat-one/foo >=cat-two/bar-2");
        }
    } test_e_repository_sets_maintainer_defined_sets_list;

#if ENABLE_GLSA
    /**
     * \test Test ERepositorySets insecurity set.
     *
     */
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
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<SetSpecTree::ConstItem> insecurity(repo->sets_interface->package_set(SetName("insecurity")));
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            insecurity->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-one/foo-1::test-repo-1 =cat-two/bar-1.5::test-repo-1 "
                                       "=cat-two/bar-1.5.1::test-repo-1 =cat-three/baz-1.0::test-repo-1 "
                                       "=cat-three/baz-1.1-r2::test-repo-1 =cat-three/baz-1.2::test-repo-1");
        }
    } test_e_repository_sets_insecurity_set;

    /**
     * \test Test ERepositorySets security set.
     *
     */
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
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> installed(
                new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed->add_version("cat-one", "foo", "2.1");
            installed->add_version("cat-two", "bar", "1.5");
            installed->add_version("cat-three", "baz", "1.0");
            env.package_database()->add_repository(0, installed);

            tr1::shared_ptr<const SetSpecTree::ConstItem> security(repo->sets_interface->package_set(SetName("security")));
            StringifyFormatter ff;
            erepository::DepSpecPrettyPrinter pretty(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
            security->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-two/bar-2.0::test-repo-1 =cat-three/baz-1.3::test-repo-1");
        }
    } test_e_repository_sets_security_set;
#endif
}

