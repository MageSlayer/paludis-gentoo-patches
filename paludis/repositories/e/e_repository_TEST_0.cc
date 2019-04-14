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
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>

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

TEST(ERepository, InstallEAPI0)
{
    TestEnvironment env;

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_0_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_0_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_0_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_0_dir" / "build"));
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
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
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
                        PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/econf-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/emake-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/einstall-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-fail-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-fail-2",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-die-1",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/keepdir-die-2",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/dobin-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fperms-fail",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fperms-die",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        ASSERT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doman-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
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
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/vars-0",
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
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-disable-dependency-tracking-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/use-with-enable-empty-third-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/output-0",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        id->perform_action(action);
    }
}
