/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
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
        throws_UnsupportedActionError,
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
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_exlibs_dir" / "repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_exlibs_dir" / "repo1/profiles/profile"));
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_exlibs_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_exlibs_dir" / "build"));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(installed_repo),
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::perform_uninstall>(&cannot_uninstall),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/" + test,
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());
            TEST_CHECK(id);
            switch (expected_result)
            {
                case success:
                    id->perform_action(action);
                    break;
                case throws_InstallActionError:
                    TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
                    break;
                case throws_UnsupportedActionError:
                    TEST_CHECK_THROWS(id->perform_action(action), UnsupportedActionError);
                    break;
            }
        }
    };
}

namespace test_cases
{
    ExlibsTest test_require_success("require-success", success);
    ExlibsTest test_require_fail("require-fail", throws_UnsupportedActionError);
    ExlibsTest test_require_param("require-param", success);
    ExlibsTest test_require_param_empty("require-param-empty", success);
    ExlibsTest test_require_param_missing("require-param-missing", throws_UnsupportedActionError);
    ExlibsTest test_require_param_undeclared("require-param-undeclared", throws_UnsupportedActionError);
    ExlibsTest test_require_params("require-params", success);
    ExlibsTest test_require_params_unaligned("require-params-unaligned", throws_UnsupportedActionError);
    ExlibsTest test_require_multiple_params("require-multiple-params", success);
    ExlibsTest test_require_multiple_params_spaces("require-multiple-params-spaces", success);
    ExlibsTest test_require_param_default("require-param-default", success);
    ExlibsTest test_require_param_default_spaces("require-multiple-params-default-spaces", success);
    ExlibsTest test_exparam_banned("exparam-banned", throws_UnsupportedActionError);
    ExlibsTest test_exparam_undeclared("exparam-undeclared", throws_InstallActionError);
    ExlibsTest test_exparam_subshell("exparam-subshell", throws_InstallActionError); // cookies to he who finds a way to make this test succeed :(
    ExlibsTest test_exarray("exarray", success);
    ExlibsTest test_exarray_spaces("exarray-spaces", success);
    ExlibsTest test_exarray_default("exarray-default", success);
    ExlibsTest test_exarray_default_spaces("exarray-default-spaces", success);
    ExlibsTest test_exarray_empty("exarray-empty", success);
    ExlibsTest test_noarray("noarray", success);
    ExlibsTest test_noarray_bad("noarray-bad", throws_UnsupportedActionError);
    ExlibsTest test_scalar_required("scalar-required", throws_UnsupportedActionError);
    ExlibsTest test_array_required("array-required", throws_UnsupportedActionError);
    ExlibsTest test_illegal_in_global_scope("illegal-in-global-scope", throws_UnsupportedActionError);
    ExlibsTest test_illegal_in_global_scope_in_func("illegal-in-global-scope-in-func", success);
    ExlibsTest test_called_cross_phase("called-cross-phase", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_default("called-cross-phase-default", throws_InstallActionError);
    ExlibsTest test_called_cross_phase_user_overridden("called-cross-phase-user-overridden", throws_InstallActionError);
    ExlibsTest test_boolean("boolean", success);
    ExlibsTest test_boolean_badvalue("boolean-badvalue", throws_UnsupportedActionError);
    ExlibsTest test_boolean_blankvalue("boolean-blankvalue", throws_UnsupportedActionError);
    ExlibsTest test_boolean_badvaluewithdefault("boolean-badvaluewithdefault", throws_UnsupportedActionError);
    ExlibsTest test_boolean_baddefault("boolean-baddefault", throws_UnsupportedActionError);
    ExlibsTest test_boolean_blankdefault("boolean-blankdefault", throws_UnsupportedActionError);
    ExlibsTest test_boolean_nodefault("boolean-nodefault", throws_UnsupportedActionError);
    ExlibsTest test_boolean_notreally("boolean-notreally", throws_UnsupportedActionError);
}

