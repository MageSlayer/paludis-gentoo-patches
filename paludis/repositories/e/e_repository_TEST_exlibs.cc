/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2009 Bo Ørsted Andresen
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
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
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
    void cannot_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
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

    enum ExpectedResult
    {
        success,
        unsupported,
        throws_InstallActionError
    };

    struct TestInfo
    {
        std::string test;
        ExpectedResult expected_result;
    };

    struct ExlibsTest :
        testing::TestWithParam<TestInfo>
    {
        TestInfo info;

        void SetUp() override
        {
            info = GetParam();
        }
    };
}

TEST_P(ExlibsTest, Works)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_exlibs_dir" / "repo1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_exlibs_dir" / "repo1/profiles/profile"));
    keys->insert("layout", "exheres");
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("profile_eapi", "exheres-0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_exlibs_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_exlibs_dir" / "build"));
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
    env.add_repository(2, installed_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("cat/" + info.test,
                            &env, { })), nullptr, { }))]->last());
    ASSERT_TRUE(bool(id));
    switch (info.expected_result)
    {
        case success:
            ASSERT_TRUE(id->supports_action(SupportsActionTest<InstallAction>()));
            id->perform_action(action);
            break;
        case throws_InstallActionError:
            ASSERT_TRUE(id->supports_action(SupportsActionTest<InstallAction>()));
            EXPECT_THROW(id->perform_action(action), ActionFailedError);
            break;
        case unsupported:
            ASSERT_TRUE(! id->supports_action(SupportsActionTest<InstallAction>()));
            break;
    }
}

INSTANTIATE_TEST_SUITE_P(Works, ExlibsTest, testing::Values(
            TestInfo{"require-success", success},
            TestInfo{"require-fail", unsupported},
            TestInfo{"require-param", success},
            TestInfo{"require-param-empty", success},
            TestInfo{"require-param-missing", unsupported},
            TestInfo{"require-param-undeclared", unsupported},
            TestInfo{"require-params", success},
            TestInfo{"require-params-unaligned", unsupported},
            TestInfo{"require-multiple-params", success},
            TestInfo{"require-multiple-params-spaces", success},
            TestInfo{"require-param-default", success},
            TestInfo{"require-multiple-params-default-spaces", success},
            TestInfo{"exparam-banned", unsupported},
            TestInfo{"exparam-undeclared", throws_InstallActionError},
            TestInfo{"exparam-subshell", throws_InstallActionError}, // cookies to he who finds a way to make this test succeed :(
            TestInfo{"exarray", success},
            TestInfo{"exarray-spaces", success},
            TestInfo{"exarray-default", success},
            TestInfo{"exarray-default-spaces", success},
            TestInfo{"exarray-default-emptied", success},
            TestInfo{"exarray-empty", success},
            TestInfo{"noarray", success},
            TestInfo{"noarray-bad", unsupported},
            TestInfo{"scalar-required", unsupported},
            TestInfo{"array-required", unsupported},
            TestInfo{"exlib-dot-with-exparam", success},
            TestInfo{"exlib-plus-with-exparam", success},
            TestInfo{"illegal-in-global-scope", unsupported},
            TestInfo{"illegal-in-global-scope-in-func", success},
            TestInfo{"called-cross-phase", throws_InstallActionError},
            TestInfo{"called-cross-phase-default", throws_InstallActionError},
            TestInfo{"called-cross-phase-user-overridden", throws_InstallActionError},
            TestInfo{"called-cross-phase-exlib", throws_InstallActionError},
            TestInfo{"called-cross-phase-exlib-exported", throws_InstallActionError},
            TestInfo{"boolean", success},
            TestInfo{"boolean-badvalue", unsupported},
            TestInfo{"boolean-blankvalue", unsupported},
            TestInfo{"boolean-badvaluewithdefault", unsupported},
            TestInfo{"boolean-baddefault", unsupported},
            TestInfo{"boolean-blankdefault", unsupported},
            TestInfo{"boolean-nodefault", unsupported},
            TestInfo{"boolean-notreally", unsupported}
            ));

