/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 David Leverton
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/elike_use_requirement.hh>
#include <paludis/choice.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>

#include <list>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void set_conditionals(const std::shared_ptr<FakePackageID> & id, const std::string & s)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(s, std::back_inserter(tokens));

        for (const auto & token : tokens)
        {
            std::string::size_type p(token.find(':'));
            if (std::string::npos == p)
                id->choices_key()->add("", token);
            else
                id->choices_key()->add(token.substr(0, p), token.substr(p + 1));
        }
    }

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> parse_elike_use_requirement_no_accumulate(
            const std::string & s, const ELikeUseRequirementOptions & o)
    {
        return parse_elike_use_requirement(s, o, nullptr);
    }
}

TEST(ELikeUseRequirements, Simple)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id, "enabled disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("enabled", { euro_strict_parsing }));
    EXPECT_EQ("[enabled]", req1->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled", req1->as_human_string(nullptr));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("disabled", { euro_strict_parsing }));
    EXPECT_EQ("[disabled]", req2->as_raw_string());
    EXPECT_EQ("Flag 'disabled' enabled", req2->as_human_string(nullptr));
    EXPECT_TRUE(! req2->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
        parse_elike_use_requirement_no_accumulate("-enabled", { euro_strict_parsing }));
    EXPECT_EQ("[-enabled]", req3->as_raw_string());
    EXPECT_EQ("Flag 'enabled' disabled", req3->as_human_string(nullptr));
    EXPECT_TRUE(! req3->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
        parse_elike_use_requirement_no_accumulate("-disabled", { euro_strict_parsing }));
    EXPECT_EQ("[-disabled]", req4->as_raw_string());
    EXPECT_EQ("Flag 'disabled' disabled", req4->as_human_string(nullptr));
    EXPECT_TRUE(req4->requirement_met(&env, nullptr, id, nullptr, nullptr).first);
}

TEST(ELikeUseRequirements, Portage)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id, "enabled disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("enabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[enabled]", req1->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled", req1->as_human_string(nullptr));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("disabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[disabled]", req2->as_raw_string());
    EXPECT_EQ("Flag 'disabled' enabled", req2->as_human_string(nullptr));
    EXPECT_TRUE(! req2->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
        parse_elike_use_requirement_no_accumulate("-enabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[-enabled]", req3->as_raw_string());
    EXPECT_EQ("Flag 'enabled' disabled", req3->as_human_string(nullptr));
    EXPECT_TRUE(! req3->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
        parse_elike_use_requirement_no_accumulate("-disabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[-disabled]", req4->as_raw_string());
    EXPECT_EQ("Flag 'disabled' disabled", req4->as_human_string(nullptr));
    EXPECT_TRUE(req4->requirement_met(&env, nullptr, id, nullptr, nullptr).first);
}

TEST(ELikeUseRequirements, Multiple)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id, "enabled disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("enabled,-disabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[enabled,-disabled]", req1->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled; Flag 'disabled' disabled", req1->as_human_string(nullptr));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("enabled,disabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[enabled,disabled]", req2->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled; Flag 'disabled' enabled", req2->as_human_string(nullptr));
    EXPECT_TRUE(! req2->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
        parse_elike_use_requirement_no_accumulate("-enabled,-disabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[-enabled,-disabled]", req3->as_raw_string());
    EXPECT_EQ("Flag 'enabled' disabled; Flag 'disabled' disabled", req3->as_human_string(nullptr));
    EXPECT_TRUE(! req3->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
        parse_elike_use_requirement_no_accumulate("enabled,-disabled,-enabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[enabled,-disabled,-enabled]", req4->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled; Flag 'disabled' disabled; Flag 'enabled' disabled", req4->as_human_string(nullptr));
    EXPECT_TRUE(! req4->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req5(
        parse_elike_use_requirement_no_accumulate("enabled,-disabled,enabled", { euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[enabled,-disabled,enabled]", req5->as_raw_string());
    EXPECT_EQ("Flag 'enabled' enabled; Flag 'disabled' disabled; Flag 'enabled' enabled", req5->as_human_string(nullptr));
    EXPECT_TRUE(req5->requirement_met(&env, nullptr, id, nullptr, nullptr).first);
}

TEST(ELikeUseRequirements, Complex)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");
    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
    set_conditionals(id2, "pkgname");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req1->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'", req1->as_human_string(id));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req2->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'", req2->as_human_string(id2));
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname?]", req3->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'", req3->as_human_string(id));
    EXPECT_TRUE(! req3->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req3->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname?]", req4->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'", req4->as_human_string(id2));
    EXPECT_TRUE(req4->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req4->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req5(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!?]", req5->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'", req5->as_human_string(id));
    EXPECT_TRUE(req5->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req5->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req6(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!?]", req6->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'", req6->as_human_string(id2));
    EXPECT_TRUE(req6->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req6->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname!?]", req7->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req7->as_human_string(id));
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname!?]", req8->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req8->as_human_string(id2));
    EXPECT_TRUE(! req8->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req8->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req9->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'", req9->as_human_string(id));
    EXPECT_TRUE(req9->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req9->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req10->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'", req10->as_human_string(id2));
    EXPECT_TRUE(! req10->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req10->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!=]", req11->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req11->as_human_string(id));
    EXPECT_TRUE(! req11->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req11->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!=]", req12->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req12->as_human_string(id2));
    EXPECT_TRUE(req12->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req12->requirement_met(&env, nullptr, id2, id2, nullptr).first);
}

TEST(ELikeUseRequirements, ComplexPortage)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");
    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
    set_conditionals(id2, "pkgname");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req1->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'", req1->as_human_string(id));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req2->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'", req2->as_human_string(id2));
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname?]", req7->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req7->as_human_string(id));
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname?]", req8->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req8->as_human_string(id2));
    EXPECT_TRUE(! req8->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req8->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req9->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'", req9->as_human_string(id));
    EXPECT_TRUE(req9->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req9->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req10->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'", req10->as_human_string(id2));
    EXPECT_TRUE(! req10->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req10->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname=]", req11->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req11->as_human_string(id));
    EXPECT_TRUE(! req11->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req11->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname=]", req12->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req12->as_human_string(id2));
    EXPECT_TRUE(req12->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req12->requirement_met(&env, nullptr, id2, id2, nullptr).first);
}

TEST(ELikeUseRequirements, Both)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");
    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
    set_conditionals(id2, "pkgname");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req1->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'", req1->as_human_string(id));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname?]", req2->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'", req2->as_human_string(id2));
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname?]", req7->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req7->as_human_string(id));
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname?]", req8->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req8->as_human_string(id2));
    EXPECT_TRUE(! req8->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req8->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req9->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'", req9->as_human_string(id));
    EXPECT_TRUE(req9->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req9->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname=]", req10->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'", req10->as_human_string(id2));
    EXPECT_TRUE(! req10->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req10->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname=]", req11->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req11->as_human_string(id));
    EXPECT_TRUE(! req11->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req11->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[!pkgname=]", req12->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req12->as_human_string(id2));
    EXPECT_TRUE(req12->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req12->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname?]", req13->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'", req13->as_human_string(id));
    EXPECT_TRUE(! req13->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req13->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname?]", req14->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'", req14->as_human_string(id2));
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!?]", req15->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'", req15->as_human_string(id));
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!?]", req16->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'", req16->as_human_string(id2));
    EXPECT_TRUE(req16->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req16->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname!?]", req17->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req17->as_human_string(id));
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[-pkgname!?]", req18->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req18->as_human_string(id2));
    EXPECT_TRUE(! req18->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req18->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!=]", req19->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req19->as_human_string(id));
    EXPECT_TRUE(! req19->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req19->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
    EXPECT_EQ("[pkgname!=]", req20->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req20->as_human_string(id2));
    EXPECT_TRUE(req20->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req20->requirement_met(&env, nullptr, id2, id2, nullptr).first);
}

TEST(ELikeUseRequirements, Malformed)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("?", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-?", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!?", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-!?", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("=", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!=", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!test?", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!test=", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test1,test2", { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
}

TEST(ELikeUseRequirements, MalformedPortage)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate(",", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test,", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate(",test", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test,,test", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test!?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-test?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-test!?", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test!=", { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
}

TEST(ELikeUseRequirements, MalformedBoth)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("-", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!?", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("!=", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);

    EXPECT_THROW(parse_elike_use_requirement_no_accumulate(",", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test,", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate(",test", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
    EXPECT_THROW(parse_elike_use_requirement_no_accumulate("test,,test", { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
}

TEST(ELikeUseRequirements, ComplexNonStrict)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");
    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
    set_conditionals(id2, "pkgname");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname?]", req1->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'", req1->as_human_string(id));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname?]", req2->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'", req2->as_human_string(id2));
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[!pkgname?]", req7->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req7->as_human_string(id));
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[!pkgname?]", req8->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req8->as_human_string(id2));
    EXPECT_TRUE(! req8->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req8->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname=]", req9->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'", req9->as_human_string(id));
    EXPECT_TRUE(req9->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req9->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname=]", req10->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'", req10->as_human_string(id2));
    EXPECT_TRUE(! req10->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req10->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps }));
    EXPECT_EQ("[!pkgname=]", req11->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req11->as_human_string(id));
    EXPECT_TRUE(! req11->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req11->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps }));
    EXPECT_EQ("[!pkgname=]", req12->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req12->as_human_string(id2));
    EXPECT_TRUE(req12->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req12->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[-pkgname?]", req13->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'", req13->as_human_string(id));
    EXPECT_TRUE(! req13->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req13->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps }));
    EXPECT_EQ("[-pkgname?]", req14->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'", req14->as_human_string(id2));
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname!?]", req15->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'", req15->as_human_string(id));
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname!?]", req16->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'", req16->as_human_string(id2));
    EXPECT_TRUE(req16->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req16->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps }));
    EXPECT_EQ("[-pkgname!?]", req17->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req17->as_human_string(id));
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps }));
    EXPECT_EQ("[-pkgname!?]", req18->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req18->as_human_string(id2));
    EXPECT_TRUE(! req18->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req18->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname!=]", req19->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req19->as_human_string(id));
    EXPECT_TRUE(! req19->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req19->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps }));
    EXPECT_EQ("[pkgname!=]", req20->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req20->as_human_string(id2));
    EXPECT_TRUE(req20->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req20->requirement_met(&env, nullptr, id2, id2, nullptr).first);
}

TEST(ELikeUseRequirements, PortageNonStrict)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
    set_conditionals(id, "pkgname");
    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
    set_conditionals(id2, "pkgname");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname?]", req1->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'", req1->as_human_string(id));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname?]", req2->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'", req2->as_human_string(id2));
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[!pkgname?]", req7->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req7->as_human_string(id));
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req7->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
        parse_elike_use_requirement_no_accumulate("!pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[!pkgname?]", req8->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req8->as_human_string(id2));
    EXPECT_TRUE(! req8->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req8->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname=]", req9->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'", req9->as_human_string(id));
    EXPECT_TRUE(req9->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(! req9->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
        parse_elike_use_requirement_no_accumulate("pkgname=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname=]", req10->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'", req10->as_human_string(id2));
    EXPECT_TRUE(! req10->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req10->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[!pkgname=]", req11->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req11->as_human_string(id));
    EXPECT_TRUE(! req11->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req11->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
        parse_elike_use_requirement_no_accumulate("!pkgname=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[!pkgname=]", req12->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req12->as_human_string(id2));
    EXPECT_TRUE(req12->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req12->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[-pkgname?]", req13->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'", req13->as_human_string(id));
    EXPECT_TRUE(! req13->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req13->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
        parse_elike_use_requirement_no_accumulate("-pkgname?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[-pkgname?]", req14->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'", req14->as_human_string(id2));
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req14->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname!?]", req15->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'", req15->as_human_string(id));
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req15->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
        parse_elike_use_requirement_no_accumulate("pkgname!?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname!?]", req16->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'", req16->as_human_string(id2));
    EXPECT_TRUE(req16->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req16->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[-pkgname!?]", req17->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'", req17->as_human_string(id));
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req17->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
        parse_elike_use_requirement_no_accumulate("-pkgname!?", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[-pkgname!?]", req18->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'", req18->as_human_string(id2));
    EXPECT_TRUE(! req18->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(req18->requirement_met(&env, nullptr, id2, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname!=]", req19->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'", req19->as_human_string(id));
    EXPECT_TRUE(! req19->requirement_met(&env, nullptr, id, id, nullptr).first);
    EXPECT_TRUE(req19->requirement_met(&env, nullptr, id2, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
        parse_elike_use_requirement_no_accumulate("pkgname!=", { euro_allow_self_deps, euro_portage_syntax }));
    EXPECT_EQ("[pkgname!=]", req20->as_raw_string());
    EXPECT_EQ("Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'", req20->as_human_string(id2));
    EXPECT_TRUE(req20->requirement_met(&env, nullptr, id, id2, nullptr).first);
    EXPECT_TRUE(! req20->requirement_met(&env, nullptr, id2, id2, nullptr).first);
}

TEST(ELikeUseRequirements, Defaults)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id, "enabled disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("missing(+)", { euro_allow_default_values, euro_strict_parsing }));
    EXPECT_EQ("[missing(+)]", req1->as_raw_string());
    EXPECT_EQ("Flag 'missing' enabled, assuming enabled if missing", req1->as_human_string(nullptr));
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("missing(-)", { euro_allow_default_values, euro_strict_parsing }));
    EXPECT_EQ("[missing(-)]", req2->as_raw_string());
    EXPECT_EQ("Flag 'missing' enabled, assuming disabled if missing", req2->as_human_string(nullptr));
    EXPECT_TRUE(! req2->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
        parse_elike_use_requirement_no_accumulate("-missing(+)", { euro_allow_default_values, euro_strict_parsing }));
    EXPECT_EQ("[-missing(+)]", req3->as_raw_string());
    EXPECT_EQ("Flag 'missing' disabled, assuming enabled if missing", req3->as_human_string(nullptr));
    EXPECT_TRUE(! req3->requirement_met(&env, nullptr, id, nullptr, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
        parse_elike_use_requirement_no_accumulate("-missing(-)", { euro_allow_default_values, euro_strict_parsing }));
    EXPECT_EQ("[-missing(-)]", req4->as_raw_string());
    EXPECT_EQ("Flag 'missing' disabled, assuming disabled if missing", req4->as_human_string(nullptr));
    EXPECT_TRUE(req4->requirement_met(&env, nullptr, id, nullptr, nullptr).first);
}

TEST(ELikeUseRequirements, PrefixStar)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);
    std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id, "foo:enabled foo:disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("foo:*", { euro_allow_default_values, euro_allow_self_deps }));
    EXPECT_EQ("[foo:*]", req1->as_raw_string());
    EXPECT_TRUE(! req1->requirement_met(&env, nullptr, id, id, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("foo:*=", { euro_allow_default_values, euro_allow_self_deps }));
    EXPECT_EQ("[foo:*=]", req2->as_raw_string());
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id, id, nullptr).first);
}

TEST(ELikeUseRequirements, QuestionDefaults)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("fake")
                    )));
    env.add_repository(1, fake);

    std::shared_ptr<FakePackageID> id1(fake->add_version("cat", "pkg1", "1"));
    set_conditionals(id1, "foo:enabled foo:disabled");

    std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "pkg2", "1"));
    set_conditionals(id2, "foo:enabled foo:disabled bar:enabled bar:disabled");

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
        parse_elike_use_requirement_no_accumulate("foo:*(?)=", { euro_allow_default_values, euro_allow_self_deps, euro_allow_default_question_values }));
    EXPECT_EQ("[foo:*(?)=]", req1->as_raw_string());
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id2, id2, nullptr).first);
    EXPECT_TRUE(req1->requirement_met(&env, nullptr, id1, id2, nullptr).first);

    std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
        parse_elike_use_requirement_no_accumulate("bar:*(?)=", { euro_allow_default_values, euro_allow_self_deps, euro_allow_default_question_values }));
    EXPECT_EQ("[bar:*(?)=]", req2->as_raw_string());
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id2, id2, nullptr).first);
    EXPECT_TRUE(req2->requirement_met(&env, nullptr, id1, id2, nullptr).first);
}

