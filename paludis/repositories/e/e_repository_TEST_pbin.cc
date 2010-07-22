/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
    struct ERepositoryInstallEAPIPBinTest : TestCase
    {
        const std::string base_eapi;

        ERepositoryInstallEAPIPBinTest(const std::string & b) :
            TestCase("install_eapi_pbin_1+" + b),
            base_eapi(b)
        {
        }

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
            FSEntry root(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "root");

            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / ("repo" + base_eapi)));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / ("repo" + base_eapi + "/profiles/profile")));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "build"));
            keys->insert("root", stringify(root));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<Map<std::string, std::string> > b_keys(new Map<std::string, std::string>);
            b_keys->insert("format", "e");
            b_keys->insert("names_cache", "/var/empty");
            b_keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / ("binrepo" + base_eapi)));
            b_keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / ("binrepo" + base_eapi + "/profiles/profile")));
            b_keys->insert("layout", "traditional");
            b_keys->insert("eapi_when_unknown", "0");
            b_keys->insert("eapi_when_unspecified", "0");
            b_keys->insert("profile_eapi", "0");
            b_keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
            b_keys->insert("binary_distdir", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
            b_keys->insert("binary_keywords", "test");
            b_keys->insert("binary_destination", "true");
            b_keys->insert("master_repository", "repo" + base_eapi);
            b_keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "build"));
            b_keys->insert("root", stringify(root));
            std::shared_ptr<Repository> b_repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, b_keys, std::placeholders::_1)));
            env.package_database()->add_repository(2, b_repo);

            std::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "vdb");
            v_keys->insert("names_cache", "/var/empty");
            v_keys->insert("provides_cache", "/var/empty");
            v_keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_pbin_dir" / "vdb"));
            v_keys->insert("root", stringify(root));
            std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, v_repo);

            {
                InstallAction bin_action(make_named_values<InstallActionOptions>(
                            n::destination() = b_repo,
                            n::make_output_manager() = &make_standard_output_manager,
                            n::perform_uninstall() = &cannot_uninstall,
                            n::replacing() = std::make_shared<PackageIDSequence>(),
                            n::want_phase() = &want_all_phases
                            ));

                TestMessageSuffix suffix("prefix", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/simple-1",
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), base_eapi);
                id->perform_action(bin_action);
            }

            TEST_CHECK(! (root / ("installed-" + base_eapi)).exists());
            b_repo->invalidate();

            {
                InstallAction install_action(make_named_values<InstallActionOptions>(
                            n::destination() = v_repo,
                            n::make_output_manager() = &make_standard_output_manager,
                            n::perform_uninstall() = &cannot_uninstall,
                            n::replacing() = std::make_shared<PackageIDSequence>(),
                            n::want_phase() = &want_all_phases
                            ));

                TestMessageSuffix suffix("prefix", true);
                const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/simple-1::binrepo" + base_eapi,
                                        &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
                TEST_CHECK(bool(id));
                TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(),
                        "pbin-1+" + base_eapi);
                id->perform_action(install_action);
            }

            TEST_CHECK((root / ("installed-" + base_eapi)).exists());
        }
    } test_e_repository_install_eapi_pbin_0("0"), test_e_repository_install_eapi_pbin_1("1"),
      test_e_repository_install_eapi_pbin_2("2"), test_e_repository_install_eapi_pbin_3("3"),
      test_e_repository_install_eapi_pbin_4("4"), test_e_repository_install_eapi_pbin_exheres_0("exheres-0");
}

