/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/repository_factory.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/metadata_key.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

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

namespace
{
    struct DependRdependTest : TestCase
    {
        const std::string eapi;
        const bool special;

        DependRdependTest(const std::string & e, bool s) :
            TestCase("RDEPEND=DEPEND EAPI " + e),
            eapi(e),
            special(s)
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
            env.set_paludis_command("/bin/false");

            FSEntry root(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "root");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "repo/profiles/profile"));
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "build"));
            keys->insert("root", stringify(root));
            std::tr1::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "vdb");
            v_keys->insert("names_cache", "/var/empty");
            v_keys->insert("provides_cache", "/var/empty");
            v_keys->insert("location", stringify(FSEntry::cwd() / "depend_rdepend_TEST_dir" / "vdb"));
            v_keys->insert("root", stringify(root));
            std::tr1::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, v_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            iv_keys->insert("format", "installed_virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            std::tr1::shared_ptr<Map<std::string, std::string> > v_keys(new Map<std::string, std::string>);
            v_keys->insert("format", "virtuals");
            env.package_database()->add_repository(-2, RepositoryFactory::get_instance()->create(&env,
                        std::tr1::bind(from_keys, v_keys, std::tr1::placeholders::_1)));
#endif

            InstallAction action(make_named_values<InstallActionOptions>(
                        value_for<n::destination>(v_repo),
                        value_for<n::make_output_manager>(&make_standard_output_manager),
                        value_for<n::perform_uninstall>(&cannot_uninstall),
                        value_for<n::replacing>(make_shared_ptr(new PackageIDSequence)),
                        value_for<n::want_phase>(&want_all_phases)
                    ));

            StringifyFormatter f;

            {
                std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "donly")))]->begin());

                TEST_CHECK_EQUAL(id->build_dependencies_key()->pretty_print_flat(f), "the/depend");
                if (special)
                    TEST_CHECK_EQUAL(id->run_dependencies_key()->pretty_print_flat(f), "the/depend");
                else
                    TEST_CHECK_EQUAL(id->run_dependencies_key()->pretty_print_flat(f), "");

                id->perform_action(action);

                v_repo->invalidate();

                std::tr1::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "donly")) |
                            filter::InstalledAtRoot(root))]->begin());

                TEST_CHECK_EQUAL(v_id->build_dependencies_key()->pretty_print_flat(f), "the/depend");
                if (special)
                    TEST_CHECK_EQUAL(v_id->run_dependencies_key()->pretty_print_flat(f), "the/depend");
                else
                    TEST_CHECK_EQUAL(v_id->run_dependencies_key()->pretty_print_flat(f), "");
            }

            {
                std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "ronly")))]->begin());

                TEST_CHECK_EQUAL(id->build_dependencies_key()->pretty_print_flat(f), "");
                TEST_CHECK_EQUAL(id->run_dependencies_key()->pretty_print_flat(f), "the/rdepend");

                id->perform_action(action);

                v_repo->invalidate();

                std::tr1::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "ronly")) |
                            filter::InstalledAtRoot(root))]->begin());

                TEST_CHECK_EQUAL(v_id->build_dependencies_key()->pretty_print_flat(f), "");
                TEST_CHECK_EQUAL(v_id->run_dependencies_key()->pretty_print_flat(f), "the/rdepend");
            }

            {
                std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "both")))]->begin());

                TEST_CHECK_EQUAL(id->build_dependencies_key()->pretty_print_flat(f), "the/depend");
                TEST_CHECK_EQUAL(id->run_dependencies_key()->pretty_print_flat(f), "the/rdepend");

                id->perform_action(action);

                v_repo->invalidate();

                std::tr1::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                                QualifiedPackageName("cat/eapi" + eapi + "both")) |
                            filter::InstalledAtRoot(root))]->begin());

                TEST_CHECK_EQUAL(v_id->build_dependencies_key()->pretty_print_flat(f), "the/depend");
                TEST_CHECK_EQUAL(v_id->run_dependencies_key()->pretty_print_flat(f), "the/rdepend");
            }
        }
    };
}

namespace test_cases
{
    DependRdependTest eapi_0("0", true);
    DependRdependTest eapi_1("1", true);
    DependRdependTest eapi_2("2", true);
    DependRdependTest eapi_3("3", true);
    DependRdependTest eapi_4("4", false);
}

