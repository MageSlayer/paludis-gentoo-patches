/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
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

    void dummy_used_this_for_config_protect(const std::string &)
    {
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

    struct ExlibsTest : TestCase
    {
        const std::string test;
        int expected_result;

        ExlibsTest(const std::string & s, const ExpectedResult e) :
            TestCase(s),
            test(s),
            expected_result(e)
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                        make_named_values<FakeInstalledRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("installed"),
                            n::suitable_destination() = true,
                            n::supports_uninstall() = true
                            )));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        n::destination() = installed_repo,
                        n::make_output_manager() = &make_standard_output_manager,
                        n::perform_uninstall() = &cannot_uninstall,
                        n::replacing() = std::make_shared<PackageIDSequence>(),
                        n::want_phase() = &want_all_phases
                    ));

            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/" + test,
                                    &env, { })), { }))]->last());
            TEST_CHECK(bool(id));
            switch (expected_result)
            {
                case success:
                    TEST_CHECK(id->supports_action(SupportsActionTest<InstallAction>()));
                    id->perform_action(action);
                    break;
                case throws_InstallActionError:
                    TEST_CHECK(id->supports_action(SupportsActionTest<InstallAction>()));
                    TEST_CHECK_THROWS(id->perform_action(action), ActionFailedError);
                    break;
                case unsupported:
                    TEST_CHECK(! id->supports_action(SupportsActionTest<InstallAction>()));
                    break;
            }
        }
    };
}

namespace test_cases
{
    ExlibsTest test_require_success("require-success", success);
    ExlibsTest test_require_fail("require-fail", unsupported);
    ExlibsTest test_require_param("require-param", success);
    ExlibsTest test_require_param_empty("require-param-empty", success);
    ExlibsTest test_require_param_missing("require-param-missing", unsupported);
    ExlibsTest test_require_param_undeclared("require-param-undeclared", unsupported);
    ExlibsTest test_require_params("require-params", success);
    ExlibsTest test_require_params_unaligned("require-params-unaligned", unsupported);
    ExlibsTest test_require_multiple_params("require-multiple-params", success);
    ExlibsTest test_require_multiple_params_spaces("require-multiple-params-spaces", success);
    ExlibsTest test_require_param_default("require-param-default", success);
    ExlibsTest test_require_param_default_spaces("require-multiple-params-default-spaces", success);
    ExlibsTest test_exparam_banned("exparam-banned", unsupported);
    ExlibsTest test_exparam_undeclared("exparam-undeclared", throws_InstallActionError);
    ExlibsTest test_exparam_subshell("exparam-subshell", throws_InstallActionError); // cookies to he who finds a way to make this test succeed :(
    ExlibsTest test_exarray("exarray", success);
    ExlibsTest test_exarray_spaces("exarray-spaces", success);
    ExlibsTest test_exarray_default("exarray-default", success);
    ExlibsTest test_exarray_default_spaces("exarray-default-spaces", success);
    ExlibsTest test_exarray_empty("exarray-empty", success);
    ExlibsTest test_noarray("noarray", success);
    ExlibsTest test_noarray_bad("noarray-bad", unsupported);
    ExlibsTest test_scalar_required("scalar-required", unsupported);
    ExlibsTest test_array_required("array-required", unsupported);
    ExlibsTest test_exlib_dot_with_exparam("exlib-dot-with-exparam", success);
    ExlibsTest test_exlib_plus_with_exparam("exlib-plus-with-exparam", success);
    ExlibsTest test_illegal_in_global_scope("illegal-in-global-scope", unsupported);
    ExlibsTest test_illegal_in_global_scope_in_func("illegal-in-global-scope-in-func", success);
    ExlibsTest test_called_cross_phase("called-cross-phase", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_default("called-cross-phase-default", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_user_overridden("called-cross-phase-user-overridden", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_exlib("called-cross-phase-exlib", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_exlib_exported("called-cross-phase-exlib-exported", throws_InstallActionError);
    ExlibsTest test_boolean("boolean", success);
    ExlibsTest test_boolean_badvalue("boolean-badvalue", unsupported);
    ExlibsTest test_boolean_blankvalue("boolean-blankvalue", unsupported);
    ExlibsTest test_boolean_badvaluewithdefault("boolean-badvaluewithdefault", unsupported);
    ExlibsTest test_boolean_baddefault("boolean-baddefault", unsupported);
    ExlibsTest test_boolean_blankdefault("boolean-blankdefault", unsupported);
    ExlibsTest test_boolean_nodefault("boolean-nodefault", unsupported);
    ExlibsTest test_boolean_notreally("boolean-notreally", unsupported);
}

