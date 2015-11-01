/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Ciaran McCreesh
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
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
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
#include <paludis/slot.hh>
#include <paludis/unformatted_pretty_printer.hh>

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
}

TEST(ERepository, InstallEAPI5)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);

    InstallAction action(make_named_values<InstallActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::perform_uninstall() = &cannot_uninstall,
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = &want_all_phases
            ));

    PretendAction pretend_action(make_named_values<PretendActionOptions>(
                n::destination() = installed_repo,
                n::make_output_manager() = &make_standard_output_manager,
                n::replacing() = std::make_shared<PackageIDSequence>()
                ));

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-disable-silent-rules-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/econf-no-docdir-htmldir-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/usex-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/new-stdin-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doheader-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/doheader-dies-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-fail-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/strict-use-injection-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/ebuild-phase-func-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/global-no-failglob-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-bare-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-dotslash-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-absolute-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-relative-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/unpack-case-sensitive-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-die-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/nonfatal-assert-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        EXPECT_THROW(id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/no-get_libdir-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/has-einstall-5",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(action);
    }
}

TEST(ERepository, RequiredUse)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-none-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-one-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(! pretend_action.failed());
    }

    {
        PretendAction pretend_action(make_named_values<PretendActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::replacing() = std::make_shared<PackageIDSequence>()
                    ));

        const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat/required-use-at-most-one-two-5::test-repo",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(id));
        EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
        id->perform_action(pretend_action);
        ASSERT_TRUE(pretend_action.failed());
    }
}

TEST(ERepository, SubSlots)
{
    FSPath root(FSPath::cwd() / "e_repository_TEST_5_dir" / "root");

    TestEnvironment env;
    env.set_system_root(FSPath("e_repository_TEST_5_dir/root").realpath());

    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "repo/profiles/profile"));
    keys->insert("layout", "traditional");
    keys->insert("eapi_when_unknown", "0");
    keys->insert("eapi_when_unspecified", "0");
    keys->insert("profile_eapi", "0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    keys = std::make_shared<Map<std::string, std::string>>();
    keys->insert("format", "vdb");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "vdb"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_5_dir" / "build"));
    keys->insert("root", stringify(FSPath("e_repository_TEST_5_dir/root").realpath()));
    std::shared_ptr<Repository> installed_repo(VDBRepository::VDBRepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(0, installed_repo);

    {
        InstallAction install_action(make_named_values<InstallActionOptions>(
                    n::destination() = installed_repo,
                    n::make_output_manager() = &make_standard_output_manager,
                    n::perform_uninstall() = &cannot_uninstall,
                    n::replacing() = std::make_shared<PackageIDSequence>(),
                    n::want_phase() = &want_all_phases
                    ));

        {
            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/subslots-5",
                                    &env, { })), nullptr, { }))]->last());
            ASSERT_TRUE(bool(id));
            EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
            EXPECT_EQ("foo/bar", id->slot_key()->parse_value().raw_value());
            EXPECT_EQ(SlotName("foo"), id->slot_key()->parse_value().match_values().first);
            EXPECT_EQ(SlotName("bar"), id->slot_key()->parse_value().match_values().second);
            EXPECT_EQ(SlotName("foo"), id->slot_key()->parse_value().parallel_value());

            id->perform_action(install_action);
        }

        installed_repo->invalidate();

        {
            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/subslots-5::installed",
                                    &env, { })), nullptr, { }))]->last());
            ASSERT_TRUE(bool(id));
            EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
            EXPECT_EQ("foo/bar", id->slot_key()->parse_value().raw_value());
            EXPECT_EQ(SlotName("foo"), id->slot_key()->parse_value().match_values().first);
            EXPECT_EQ(SlotName("bar"), id->slot_key()->parse_value().match_values().second);
            EXPECT_EQ(SlotName("foo"), id->slot_key()->parse_value().parallel_value());
        }

        {
            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/subslot-dep-5",
                                    &env, { })), nullptr, { }))]->last());
            ASSERT_TRUE(bool(id));
            EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
            EXPECT_EQ("cat/subslots:= cat/subslots:foo=", id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
            id->perform_action(install_action);
        }

        installed_repo->invalidate();

        {
            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/subslot-dep-5::installed",
                                    &env, { })), nullptr, { }))]->last());
            ASSERT_TRUE(bool(id));
            EXPECT_EQ("5", visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->parse_value());
            EXPECT_EQ("cat/subslots:foo/bar= cat/subslots:foo/bar=", id->build_dependencies_key()->pretty_print_value(UnformattedPrettyPrinter(), { }));
        }
    }
}

