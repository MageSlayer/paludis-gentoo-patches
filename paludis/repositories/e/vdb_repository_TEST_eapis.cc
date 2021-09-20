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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>

#include <paludis/metadata_key.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/unformatted_pretty_printer.hh>

#include <paludis/util/indirect_iterator-impl.hh>

#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void do_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions & u)
    {
        UninstallAction a(u);
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

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }

    bool ignore_nothing(const FSPath &)
    {
        return false;
    }

    struct PhasesTest :
        testing::TestWithParam<std::string>
    {
        std::string eapi;

        void SetUp() override
        {
            eapi = GetParam();
        }
    };
}

TEST_P(PhasesTest, Works)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "srcrepo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "srcrepo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", eapi);
    keys->insert("eapi_when_unspecified", eapi);
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_eapis_dir/root").realpath()));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "dstrepo"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_eapis_dir/root").realpath()));
    std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, vdb_repo);

    InstallAction install_action(make_named_values<InstallActionOptions>(
                n::destination() = vdb_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &do_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                n::config_protect() = "",
                n::if_for_install_id() = nullptr,
                n::ignore_for_unmerge() = &ignore_nothing,
                n::is_overwrite() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::override_contents() = nullptr,
                n::want_phase() = &want_all_phases
            ));

    InfoActionOptions info_action_options(make_named_values<InfoActionOptions>(
                n::make_output_manager() = &make_standard_output_manager
                ));

    ConfigActionOptions config_action_options(make_named_values<ConfigActionOptions>(
                n::make_output_manager() = &make_standard_output_manager
                ));

    InfoAction info_action(info_action_options);
    ConfigAction config_action(config_action_options);

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(install_action);
    }

    vdb_repo->invalidate();

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(install_action);
    }

    vdb_repo->invalidate();

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(info_action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(config_action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(uninstall_action);
    }
}

INSTANTIATE_TEST_SUITE_P(Works, PhasesTest, testing::Values(
            std::string("0"),
            std::string("1"),
            std::string("2"),
            std::string("3"),
            std::string("4"),
            std::string("5"),
            std::string("exheres-0")
            ));

namespace
{
    struct VarsTest :
        testing::TestWithParam<std::string>
    {
        std::string eapi;

        void SetUp() override
        {
            eapi = GetParam();
        }
    };
}

TEST_P(VarsTest, Works)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "srcrepo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "srcrepo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", eapi);
    keys->insert("eapi_when_unspecified", eapi);
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_eapis_dir/root").realpath()));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "dstrepo"));
    keys->insert("builddir", stringify(FSPath::cwd() / "vdb_repository_TEST_eapis_dir" / "build"));
    keys->insert("root", stringify(FSPath("vdb_repository_TEST_eapis_dir/root").realpath()));
    std::shared_ptr<Repository> vdb_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, vdb_repo);

    InstallAction install_action(make_named_values<InstallActionOptions>(
                n::destination() = vdb_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &do_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    UninstallAction uninstall_action(make_named_values<UninstallActionOptions>(
                n::config_protect() = "",
                n::if_for_install_id() = nullptr,
                n::ignore_for_unmerge() = &ignore_nothing,
                n::is_overwrite() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::override_contents() = nullptr,
                n::want_phase() = &want_all_phases
            ));

    InfoActionOptions info_action_options(make_named_values<InfoActionOptions>(
                n::make_output_manager() = &make_standard_output_manager
                ));

    ConfigActionOptions config_action_options(make_named_values<ConfigActionOptions>(
                n::make_output_manager() = &make_standard_output_manager
                ));

    InfoAction info_action(info_action_options);
    ConfigAction config_action(config_action_options);

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(install_action);
    }

    vdb_repo->invalidate();

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::srcrepo",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(install_action);
    }

    vdb_repo->invalidate();

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(info_action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(config_action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/target-" + eapi + "::installed",
                                &env, { })), nullptr, { }))]->begin());
        ASSERT_TRUE(bool(id));
        id->perform_action(uninstall_action);
    }
}

INSTANTIATE_TEST_SUITE_P(Works, VarsTest, testing::Values(
            std::string("0"),
            std::string("1"),
            std::string("2"),
            std::string("3"),
            std::string("4"),
            std::string("5"),
            std::string("exheres-0")
            ));

