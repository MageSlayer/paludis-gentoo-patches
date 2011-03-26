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
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/join.hh>
#include <paludis/unformatted_pretty_printer.hh>

#include "config.h"

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

TEST(ERepository, ListSets)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_sets_TEST_dir/repo1"));
    keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const SetNameSet> sets_list(env.set_names());
    ASSERT_EQ("everything insecurity "
            "insecurity::test-repo-1 "
            "installed-packages installed-packages::default "
            "installed-slots installed-slots::default "
            "nothing "
            "security security::test-repo-1 set1 set1* "
            "set1::test-repo-1 set1::test-repo-1* "
            "system system::test-repo-1 world world::default",
            join(sets_list->begin(), sets_list->end(), " "));
}

TEST(ERepository, MaintainerDefinedSets)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_sets_TEST_dir/repo1"));
    keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    std::shared_ptr<FakeInstalledRepository> installed(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    installed->add_version("cat-two", "bar", "1.5");
    env.add_repository(0, installed);
    env.add_repository(1, repo);

    std::shared_ptr<const SetSpecTree> set1(env.set(SetName("set1::test-repo-1")));
    ASSERT_TRUE(bool(set1));
    UnformattedPrettyPrinter ff;
    erepository::SpecTreePrettyPrinter pretty(ff, { });
    set1->top()->accept(pretty);
    EXPECT_EQ("cat-one/foo >=cat-two/bar-2", stringify(pretty));
}

#if ENABLE_XML

TEST(ERepository, Insecurity)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_sets_TEST_dir/repo1"));
    keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const SetSpecTree> insecurity(env.set(SetName("insecurity::test-repo-1")));
    UnformattedPrettyPrinter ff;
    erepository::SpecTreePrettyPrinter pretty(ff, { });
    insecurity->top()->accept(pretty);
    ASSERT_EQ("=cat-four/xyzzy-2.0.1::test-repo-1 =cat-four/xyzzy-2.0.2::test-repo-1 =cat-one/foo-1::test-repo-1 =cat-two/bar-1.5::test-repo-1 "
            "=cat-two/bar-1.5.1::test-repo-1 =cat-three/baz-1.0::test-repo-1 "
            "=cat-three/baz-1.1-r2::test-repo-1 =cat-three/baz-1.2::test-repo-1",
            stringify(pretty));
}

TEST(ERepository, Security)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string> >());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_sets_TEST_dir/repo1"));
    keys->insert("profiles", "e_repository_sets_TEST_dir/repo1/profiles/profile");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);
    std::shared_ptr<FakeInstalledRepository> installed(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    installed->add_version("cat-one", "foo", "2.1");
    installed->add_version("cat-two", "bar", "1.5");
    installed->add_version("cat-three", "baz", "1.0");
    installed->add_version("cat-four", "xyzzy", "1.1.0")->set_slot(SlotName("1"));
    installed->add_version("cat-four", "xyzzy", "2.0.1")->set_slot(SlotName("2"));
    env.add_repository(0, installed);

    std::shared_ptr<const SetSpecTree> security(env.set(SetName("security::test-repo-1")));
    UnformattedPrettyPrinter ff;
    erepository::SpecTreePrettyPrinter pretty(ff, { });
    security->top()->accept(pretty);
    EXPECT_EQ("=cat-four/xyzzy-2.0.3::test-repo-1 =cat-two/bar-2.0::test-repo-1 =cat-three/baz-1.3::test-repo-1", stringify(pretty));
}

#endif


