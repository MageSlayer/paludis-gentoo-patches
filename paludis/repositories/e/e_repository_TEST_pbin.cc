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
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>

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

#include <functional>
#include <set>
#include <string>

#include "config.h"

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

    struct ERepositoryInstallEAPIPBinTest :
        testing::TestWithParam<std::string>
    {
        std::string base_eapi;

        void SetUp()
        {
            base_eapi = GetParam();
        }
    };
}

TEST_P(ERepositoryInstallEAPIPBinTest, Works)
{
    TestEnvironment env;
    FSPath root(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "root");

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("repo" + base_eapi)));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("repo" + base_eapi + "/profiles/profile")));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "build"));
    keys->insert("root", stringify(root));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<Map<std::string, std::string> > b_keys(std::make_shared<Map<std::string, std::string>>());
    b_keys->insert("format", "e");
    b_keys->insert("names_cache", "/var/empty");
    b_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("binrepo" + base_eapi)));
    b_keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("binrepo" + base_eapi + "/profiles/profile")));
    b_keys->insert("layout", "traditional");
    b_keys->insert("eapi_when_unknown", "0");
    b_keys->insert("eapi_when_unspecified", "0");
    b_keys->insert("profile_eapi", "0");
    b_keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    b_keys->insert("binary_distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    b_keys->insert("binary_keywords_filter", "test");
    b_keys->insert("binary_destination", "true");
    b_keys->insert("master_repository", "repo" + base_eapi);
    b_keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "build"));
    b_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> b_repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, b_keys, std::placeholders::_1)));
    env.add_repository(2, b_repo);

    std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
    v_keys->insert("format", "vdb");
    v_keys->insert("names_cache", "/var/empty");
    v_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "vdb"));
    v_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, v_repo);

    {
        InstallAction bin_action(make_named_values<InstallActionOptions>(
                    n::destination() = b_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/simple-1",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ(base_eapi, visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(bin_action);
    }

    EXPECT_TRUE(! (root / ("installed-" + base_eapi)).stat().exists());
    b_repo->invalidate();

    {
        InstallAction install_action(make_named_values<InstallActionOptions>(
                    n::destination() = v_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/simple-1::binrepo" + base_eapi,
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("pbin-1+" + base_eapi, visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(install_action);
    }

    EXPECT_TRUE((root / ("installed-" + base_eapi)).stat().exists());
}

INSTANTIATE_TEST_CASE_P(Works, ERepositoryInstallEAPIPBinTest, testing::Values(
            std::string("0"),
            std::string("1"),
            std::string("2"),
            std::string("3"),
            std::string("4"),
            std::string("exheres-0")
            ));

TEST(Symlinks, Works)
{
    TestEnvironment env;
    FSPath root(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "root");

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("repoexheres-0")));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("repoexheres-0/profiles/profile")));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "build"));
    keys->insert("root", stringify(root));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<Map<std::string, std::string> > b_keys(std::make_shared<Map<std::string, std::string>>());
    b_keys->insert("format", "e");
    b_keys->insert("names_cache", "/var/empty");
    b_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("binrepoexheres-0")));
    b_keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / ("binrepoexheres-0/profiles/profile")));
    b_keys->insert("layout", "traditional");
    b_keys->insert("eapi_when_unknown", "0");
    b_keys->insert("eapi_when_unspecified", "0");
    b_keys->insert("profile_eapi", "0");
    b_keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    b_keys->insert("binary_distdir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "distdir"));
    b_keys->insert("binary_keywords_filter", "test");
    b_keys->insert("binary_destination", "true");
    b_keys->insert("master_repository", "repoexheres-0");
    b_keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "build"));
    b_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> b_repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, b_keys, std::placeholders::_1)));
    env.add_repository(2, b_repo);

    std::shared_ptr<Map<std::string, std::string> > v_keys(std::make_shared<Map<std::string, std::string>>());
    v_keys->insert("format", "vdb");
    v_keys->insert("names_cache", "/var/empty");
    v_keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_pbin_dir" / "vdb"));
    v_keys->insert("root", stringify(root));
    std::shared_ptr<Repository> v_repo(VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, v_repo);

    {
        InstallAction bin_action(make_named_values<InstallActionOptions>(
                    n::destination() = b_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/symlinks-1",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("exheres-0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(bin_action);
    }

    EXPECT_TRUE(! (root / ("symlinks-a")).stat().exists());
    EXPECT_TRUE(! (root / ("symlinks-b")).stat().exists());
    EXPECT_TRUE(! (root / ("symlinks-c")).stat().exists());
    b_repo->invalidate();

    {
        InstallAction install_action(make_named_values<InstallActionOptions>(
                    n::destination() = v_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/symlinks-1::binrepoexheres-0",
                                &env, { })), make_null_shared_ptr(), { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("pbin-1+exheres-0", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(install_action);
    }

    EXPECT_TRUE((root / ("symlinks-a")).stat().exists());
    EXPECT_TRUE((root / ("symlinks-b")).stat().exists());
    EXPECT_TRUE((root / ("symlinks-c")).stat().exists());

    EXPECT_TRUE((root / ("symlinks-a")).stat().is_symlink());
    EXPECT_TRUE((root / ("symlinks-b")).stat().is_regular_file());
    EXPECT_TRUE((root / ("symlinks-c")).stat().is_symlink());
}

