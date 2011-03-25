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
#include <paludis/util/make_null_shared_ptr.hh>
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

    struct ReplacingTest : TestCase
    {
        const std::string eapi;
        const std::string repo_path;
        const std::string test;
        const std::string replacing;
        const std::string replacing_pkg_name;

        ReplacingTest(const std::string & e, const std::string & p, const std::string & s, const std::string & r,
                const std::string & n) :
            TestCase(e + " " + s),
            eapi(e),
            repo_path(p),
            test(s),
            replacing(r),
            replacing_pkg_name(n)
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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / repo_path));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / repo_path
                        / "profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", eapi);
            keys->insert("eapi_when_unspecified", eapi);
            keys->insert("profile_eapi", eapi);
            keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / "distdir"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_replacing_dir" / "build"));
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
            installed_repo->add_version("cat", replacing_pkg_name, "1")->set_slot(SlotName("1"));
            installed_repo->add_version("cat", replacing_pkg_name, "2")->set_slot(SlotName("2"));
            installed_repo->add_version("cat", replacing_pkg_name, "3")->set_slot(SlotName("3"));
            env.package_database()->add_repository(2, installed_repo);

            const std::shared_ptr<const PackageIDSequence> rlist(env[selection::AllVersionsSorted(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec(replacing, &env, { })),
                            make_null_shared_ptr(), { }) |
                        filter::InstalledAtRoot(env.preferred_root_key()->value()))]);

            InstallAction action(make_named_values<InstallActionOptions>(
                        n::destination() = installed_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &do_uninstall,
                        n::replacing() = rlist,
                        n::want_phase() = &want_all_phases
                    ));

            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/" + test,
                                    &env, { })), make_null_shared_ptr(), { }) |
                        filter::SupportsAction<InstallAction>())]->last());
            TEST_CHECK(bool(id));
            id->perform_action(action);
        }
    };
}

namespace test_cases
{
    ReplacingTest test_exheres_0_replace_none("exheres-0", "repo1", "replace-none", "cat/none", "pkg");
    ReplacingTest test_exheres_0_replace_one("exheres-0", "repo1", "replace-one", "=cat/pkg-1", "pkg");
    ReplacingTest test_exheres_0_replace_many("exheres-0", "repo1", "replace-many", "cat/pkg", "pkg");

    ReplacingTest test_4_replace_none("0", "repo2", "replace-none", "cat/none", "replace-none");
    ReplacingTest test_4_replace_one("0", "repo2", "replace-one", "=cat/replace-one-1", "replace-one");
    ReplacingTest test_4_replace_many("0", "repo2", "replace-many", "cat/replace-many", "replace-many");
}

