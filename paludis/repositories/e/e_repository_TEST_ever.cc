/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/output_manager.hh>
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

    struct EverTest : TestCase
    {
        const std::string test;

        EverTest(const std::string & s) :
            TestCase("ever " + s),
            test(s)
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
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_ever_dir" / "repo1"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_ever_dir" / "repo1/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_ever_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_ever_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                        make_named_values<FakeInstalledRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("installed"),
                            n::suitable_destination() = true,
                            n::supports_uninstall() = true
                            )));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::shared_ptr<Map<std::string, std::string> > iv_keys(std::make_shared<Map<std::string, std::string>>());
            iv_keys->insert("root", "/");
            iv_keys->insert("format", "installed_virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::bind(from_keys, iv_keys, std::placeholders::_1)));
            std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
            v_keys->insert("format", "virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::bind(from_keys, v_keys, std::placeholders::_1)));
#endif

            InstallAction action(make_named_values<InstallActionOptions>(
                        n::destination() = installed_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &cannot_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/" + test,
                                    &env, { })), make_null_shared_ptr(), { }))]->last());
            TEST_CHECK(bool(id));
            id->perform_action(action);
        }
    };
}

namespace test_cases
{
    EverTest test_split("ever-split");
    EverTest test_split_all("ever-split-all");
    EverTest test_at_least("ever-at-least");
    EverTest test_is_scm("ever-is_scm");
    EverTest test_major("ever-major");
    EverTest test_range("ever-range");
    EverTest test_remainder("ever-remainder");
    EverTest test_replace("ever-replace");
    EverTest test_replace_all("ever-replace_all");
    EverTest test_delete("ever-delete");
    EverTest test_delete_all("ever-delete_all");
}

