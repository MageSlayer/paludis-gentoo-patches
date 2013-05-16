/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>

#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <paludis/standard_output_manager.hh>

#include <functional>
#include <set>
#include <string>

#include "config.h"

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void do_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions & uo)
    {
        UninstallAction a(uo);
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

    void dummy_used_this_for_config_protect(const std::string &)
    {
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    struct TestInfo
    {
        std::string eapi;
        std::string repo_path;
        std::string test;
        std::string replacing;
        std::string replacing_pkg_name;
    };

    struct ReplacingTest :
        testing::TestWithParam<TestInfo>
    {
        TestInfo info;

        void SetUp()
        {
            info = GetParam();
        }
    };
}

TEST_P(ReplacingTest, Works)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / info.repo_path));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / info.repo_path
                / "profiles/profile"));
    keys->insert("layout", "exheres");
    keys->insert("eapi_when_unknown", info.eapi);
    keys->insert("eapi_when_unspecified", info.eapi);
    keys->insert("profile_eapi", info.eapi);
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    installed_repo->add_version("cat", info.replacing_pkg_name, "1")->set_slot(SlotName("1"));
    installed_repo->add_version("cat", info.replacing_pkg_name, "2")->set_slot(SlotName("2"));
    installed_repo->add_version("cat", info.replacing_pkg_name, "3")->set_slot(SlotName("3"));
    env.add_repository(2, installed_repo);

    const std::shared_ptr<const PackageIDSequence> rlist(env[selection::AllVersionsSorted(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec(info.replacing, &env, { })),
                    nullptr, { }) |
                filter::InstalledAtRoot(env.preferred_root_key()->parse_value()))]);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &do_uninstall,
                n::replacing() = rlist,
                n::want_phase() = &want_all_phases
            ));

    const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("cat/" + info.test,
                            &env, { })), nullptr, { }) |
                filter::SupportsAction<InstallAction>())]->last());
    ASSERT_TRUE(bool(id));
    id->perform_action(action);
}

INSTANTIATE_TEST_CASE_P(Works, ReplacingTest, testing::Values(
            TestInfo{"exheres-0", "repo1", "replace-none", "cat/none", "pkg"},
            TestInfo{"exheres-0", "repo1", "replace-one", "=cat/pkg-1", "pkg"},
            TestInfo{"exheres-0", "repo1", "replace-many", "cat/pkg", "pkg"},
            TestInfo{"0", "repo2", "replace-none", "cat/none", "replace-none"},
            TestInfo{"0", "repo2", "replace-one", "=cat/replace-one-1", "replace-one"},
            TestInfo{"0", "repo2", "replace-many", "cat/replace-many", "replace-many"}
            ));

