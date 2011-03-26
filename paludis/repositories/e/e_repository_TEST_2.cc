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
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
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
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>
#include <set>
#include <string>

#include "config.h"

using namespace test;
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

namespace test_cases
{
    struct ERepositoryInstallEAPI2Test : TestCase
    {
        ERepositoryInstallEAPI2Test() : TestCase("install_eapi_2") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_2_dir" / "repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_2_dir" / "repo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_2_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_2_dir" / "build"));
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

            {
                TestMessageSuffix suffix("econf source 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("doman 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("src_prepare 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("src_configure 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("default src_configure 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default-src_configure-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("default src_compile 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default-src_compile-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("default_src_compile 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/default_src_compile-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("src_compile via default function 2", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_compile-via-default-func-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "2");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_2;
}

