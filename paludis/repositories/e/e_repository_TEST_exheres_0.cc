/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/exndbam_repository.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/safe_ifstream.hh>

#include <paludis/standard_output_manager.hh>
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
}

TEST(ERepository, InstallExheres0)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "repo/profiles/profile"));
    keys->insert("layout", "exheres");
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("profile_eapi_when_unspecified", "exheres-0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "build"));
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
    installed_repo->add_version("cat", "pretend-installed", "0");
    installed_repo->add_version("cat", "pretend-installed", "1");
    env.add_repository(2, installed_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/expatch-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/expatch-success-dir",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/expatch-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/expatch-unrecognised",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/econf-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-fail-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-fail-2",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/dobin-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/herebin-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/herebin-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereconfd-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereconfd-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereenvd-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereenvd-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereinitd-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereinitd-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereins-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/hereins-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/heresbin-success",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/heresbin-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-phase-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-vars-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/expand-vars-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doman-success-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doman-nonfatal-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doman-failure-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/change-globals-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/install-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/install-s-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/global-optionq-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/expecting-tests-enabled-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/expecting-tests-disabled-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/expecting-tests-none-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/banned-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }
}

TEST(ERepository, ReallyInstallExheres0)
{
    TestEnvironment env;
    auto keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "repo/profiles/profile"));
    keys->insert("layout", "exheres");
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("profile_eapi_when_unspecified", "exheres-0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    auto i_keys(std::make_shared<Map<std::string, std::string>>());
    i_keys->insert("format", "exndbam");
    i_keys->insert("names_cache", "/var/empty");
    i_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "instrepo"));
    i_keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "build"));
    i_keys->insert("root", stringify(FSPath::cwd() / "e_repository_TEST_exheres_0_dir" / "root"));
    std::shared_ptr<Repository> i_repo(ExndbamRepository::repository_factory_create(&env,
                std::bind(from_keys, i_keys, std::placeholders::_1)));
    env.add_repository(1, i_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = i_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                n::config_protect() = "",
                n::if_for_install_id() = nullptr,
                n::ignore_for_unmerge() = [] (const FSPath &) { return false; },
                n::is_overwrite() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::override_contents() = nullptr,
                n::want_phase() = &want_all_phases
            ));

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/exdirectory-phase-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/exdirectory-forbid-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/exdirectory-allow-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/permitted-directories-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/permitted-directories-2",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/permitted-directories-3",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/permitted-directories-4",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/exvolatile-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);

        i_repo->invalidate();
        const std::shared_ptr<const PackageID> uninstall_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/exvolatile-1::installed",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(uninstall_id));
        uninstall_id->perform_action(uninstall_action);
    }
}

