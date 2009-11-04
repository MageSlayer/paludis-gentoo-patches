/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/functional>
#include <set>
#include <string>

#include "config.h"

using namespace test;
using namespace paludis;

namespace
{
    void cannot_uninstall(const std::tr1::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::tr1::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return make_shared_ptr(new StandardOutputManager);
    }

    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
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
    struct ERepositoryInstallEAPIKdebuild1Test : TestCase
    {
        ERepositoryInstallEAPIKdebuild1Test() : TestCase("install_eapi_kdebuild_1") { }

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
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "repo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("installed")),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                            )));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::perform_uninstall>(&cannot_uninstall),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                        value_for<n::want_phase>(&want_all_phases)
                        ));

            {
                TestMessageSuffix suffix("econf source kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("banned functions kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("banned vars kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-vars-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "UNKNOWN");
            }

            {
                TestMessageSuffix suffix("dosym success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dosym fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }

            {
                TestMessageSuffix suffix("doman kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/doman-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_prepare kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_prepare-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("no src_configure kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/src_configure-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_kdebuild_1;

    struct ERepositoryInfoEAPIKdebuild1Test : TestCase
    {
        ERepositoryInfoEAPIKdebuild1Test() : TestCase("info_eapi_kdebuild_1") { }

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
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "repo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_kdebuild_1_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(
                        make_named_values<FakeInstalledRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("installed")),
                            value_for<n::suitable_destination>(true),
                            value_for<n::supports_uninstall>(true)
                            )));
            env.package_database()->add_repository(2, installed_repo);

            InfoActionOptions options(make_named_values<InfoActionOptions>(
                        value_for<n::make_output_manager>(&make_standard_output_manager)
                        ));
            InfoAction action(options);

            {
                TestMessageSuffix suffix("info success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("info fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
            }
        }
    } test_e_repository_info_eapi_kdebuild_1;
}

