/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <u01drl3@abdn.ac.uk>
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

#include <paludis/repositories/gentoo/portage_repository.hh>
#include <paludis/repositories/gentoo/make_ebuild_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include "config.h"

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageRepositorySets.
 *
 */

namespace test_cases
{
    /**
     * \test Test PortageRepositorySets sets list.
     *
     */
    struct PortageRepositorySetsSetsListTest : TestCase
    {
        PortageRepositorySetsSetsListTest() : TestCase("sets list") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const SetNameCollection> sets_list(repo->sets_interface->sets_list());
            TEST_CHECK_EQUAL(sets_list->size(), 4U);
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("system")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("security")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("insecurity")));
            TEST_CHECK(sets_list->end() != sets_list->find(SetName("set1")));
        }
    } test_portage_repository_sets_sets_list;

    /**
     * \test Test PortageRepositorySets maintainer-defined sets.
     *
     */
    struct PortageRepositorySetsMaintainerDefinedSetsTest : TestCase
    {
        PortageRepositorySetsMaintainerDefinedSetsTest() : TestCase("maintainer-defined sets") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            tr1::shared_ptr<FakeInstalledRepository> installed(
                new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed->add_version("cat-two", "bar", "1.5");
            env.package_database()->add_repository(0, installed);

            tr1::shared_ptr<SetSpecTree::ConstItem> set1(repo->sets_interface->package_set(SetName("set1")));
            DepSpecPrettyPrinter pretty(0, false);
            set1->accept(pretty);
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "cat-one/foo >=cat-two/bar-2");
        }
    } test_portage_repository_sets_maintainer_defined_sets_list;

    /**
     * \test Test PortageRepositorySets insecurity set.
     *
     */
    struct PortageRepositorySetsInsecuritySetTest : TestCase
    {
        PortageRepositorySetsInsecuritySetTest() : TestCase("insecurity set") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<SetSpecTree::ConstItem> insecurity(repo->sets_interface->package_set(SetName("insecurity")));
            DepSpecPrettyPrinter pretty(0, false);
            insecurity->accept(pretty);
#if ENABLE_GLSA
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-one/foo-1::test-repo-1 =cat-two/bar-1.5::test-repo-1 "
                                       "=cat-two/bar-1.5.1::test-repo-1 =cat-three/baz-1.0::test-repo-1 "
                                       "=cat-three/baz-1.1-r2::test-repo-1 =cat-three/baz-1.2::test-repo-1");
#else
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "");
#endif
        }
    } test_portage_repository_sets_insecurity_set;

    /**
     * \test Test PortageRepositorySets security set.
     *
     */
    struct PortageRepositorySetsSecuritySetTest : TestCase
    {
        PortageRepositorySetsSecuritySetTest() : TestCase("security set") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_sets_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_sets_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> installed(
                new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed->add_version("cat-one", "foo", "2.1");
            installed->add_version("cat-two", "bar", "1.5");
            installed->add_version("cat-three", "baz", "1.0");
            env.package_database()->add_repository(0, installed);

            tr1::shared_ptr<const SetSpecTree::ConstItem> security(repo->sets_interface->package_set(SetName("security")));
            DepSpecPrettyPrinter pretty(0, false);
            security->accept(pretty);
#if ENABLE_GLSA
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "=cat-two/bar-2.0::test-repo-1 =cat-three/baz-1.3::test-repo-1");
#else
            TEST_CHECK_STRINGIFY_EQUAL(pretty, "");
#endif
        }
    } test_portage_repository_sets_security_set;
}

