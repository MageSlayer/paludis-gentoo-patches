/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011, 2012 Ciaran McCreesh
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
#include <paludis/util/make_named_values.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/stringify.hh>

#include <paludis/repository_factory.hh>
#include <paludis/repository.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/metadata_key.hh>
#include <paludis/unformatted_pretty_printer.hh>

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

    struct TestParams
    {
        std::string eapi;
        bool special;
    };

    struct DependRdependTest :
        testing::TestWithParam<TestParams>
    {
        std::string eapi;
        bool special;

        void SetUp() override
        {
            eapi = GetParam().eapi;
            special = GetParam().special;
        }
    };
}

TEST_P(DependRdependTest, Works)
{
    TestEnvironment env;

    FSPath root(FSPath::cwd() / "depend_rdepend_TEST_dir" / "root");

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "depend_rdepend_TEST_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "depend_rdepend_TEST_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "depend_rdepend_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "depend_rdepend_TEST_dir" / "build"));
    keys->insert("root", stringify(root));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
    v_keys->insert("format", "vdb");
    v_keys->insert("names_cache", "/var/empty");
    v_keys->insert("location", stringify(FSPath::cwd() / "depend_rdepend_TEST_dir" / "vdb"));
    v_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, v_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = v_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    {
        std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "donly")))]->begin());

        EXPECT_EQ("the/depend", id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        if (special)
            EXPECT_EQ("the/depend", id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        else
            EXPECT_FALSE(id->run_dependencies_key());

        id->perform_action(action);

        v_repo->invalidate();

        std::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "donly")) |
                    filter::InstalledAtRoot(root))]->begin());

        EXPECT_EQ("the/depend", v_id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        if (special)
            EXPECT_EQ("the/depend", v_id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        else
            EXPECT_FALSE(v_id->run_dependencies_key());
    }

    {
        std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "ronly")))]->begin());

        EXPECT_FALSE(id->build_dependencies_key());
        EXPECT_EQ("the/rdepend", id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));

        id->perform_action(action);

        v_repo->invalidate();

        std::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "ronly")) |
                    filter::InstalledAtRoot(root))]->begin());

        EXPECT_FALSE(v_id->build_dependencies_key());
        EXPECT_EQ("the/rdepend", v_id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
    }

    {
        std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "both")))]->begin());

        EXPECT_EQ("the/depend", id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        EXPECT_EQ("the/rdepend", id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));

        id->perform_action(action);

        v_repo->invalidate();

        std::shared_ptr<const PackageID> v_id(*env[selection::RequireExactlyOne(generator::Package(
                        QualifiedPackageName("cat/eapi" + eapi + "both")) |
                    filter::InstalledAtRoot(root))]->begin());

        EXPECT_EQ("the/depend", v_id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        EXPECT_EQ("the/rdepend", v_id->run_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
    }
}

INSTANTIATE_TEST_SUITE_P(DependRdepend, DependRdependTest, testing::Values(
            TestParams{"0", true},
            TestParams{"1", true},
            TestParams{"2", true},
            TestParams{"3", true},
            TestParams{"4", false}
            ));

