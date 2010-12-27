/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 David Leverton
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
#include <paludis/package_database.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/choice.hh>
#include <paludis/additional_package_dep_spec_requirement.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <list>

using namespace paludis;
using namespace test;

namespace
{
    void set_conditionals(const std::shared_ptr<FakePackageID> & id, const std::string & s)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(s, std::back_inserter(tokens));

        for (std::list<std::string>::const_iterator t(tokens.begin()), t_end(tokens.end()) ;
                t != t_end ; ++t)
        {
            std::string::size_type p(t->find(':'));
            if (std::string::npos == p)
                id->choices_key()->add("", *t);
            else
                id->choices_key()->add(t->substr(0, p), t->substr(p + 1));
        }
    }
}

namespace test_cases
{
    struct SimpleUseRequirementsTest : TestCase
    {
        SimpleUseRequirementsTest() : TestCase("simple use requirements") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id, "enabled disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("enabled", std::shared_ptr<const PackageID>(), { euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[enabled]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'enabled' enabled");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("disabled", std::shared_ptr<const PackageID>(), { euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[disabled]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'disabled' enabled");
            TEST_CHECK(! req2->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
                parse_elike_use_requirement("-enabled", std::shared_ptr<const PackageID>(), { euro_strict_parsing }));
            TEST_CHECK_EQUAL(req3->as_raw_string(), "[-enabled]");
            TEST_CHECK_EQUAL(req3->as_human_string(), "Flag 'enabled' disabled");
            TEST_CHECK(! req3->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
                parse_elike_use_requirement("-disabled", std::shared_ptr<const PackageID>(), { euro_strict_parsing }));
            TEST_CHECK_EQUAL(req4->as_raw_string(), "[-disabled]");
            TEST_CHECK_EQUAL(req4->as_human_string(), "Flag 'disabled' disabled");
            TEST_CHECK(req4->requirement_met(&env, 0, id, 0).first);
        }
    } test_simple_use_requirements;

    struct SimpleUseRequirementsPortageSyntaxTest : TestCase
    {
        SimpleUseRequirementsPortageSyntaxTest() : TestCase("simple use requirements portage syntax") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("repo")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id, "enabled disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("enabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[enabled]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'enabled' enabled");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("disabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[disabled]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'disabled' enabled");
            TEST_CHECK(! req2->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
                parse_elike_use_requirement("-enabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req3->as_raw_string(), "[-enabled]");
            TEST_CHECK_EQUAL(req3->as_human_string(), "Flag 'enabled' disabled");
            TEST_CHECK(! req3->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
                parse_elike_use_requirement("-disabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req4->as_raw_string(), "[-disabled]");
            TEST_CHECK_EQUAL(req4->as_human_string(), "Flag 'disabled' disabled");
            TEST_CHECK(req4->requirement_met(&env, 0, id, 0).first);
        }
    } test_simple_use_requirements_portage_syntax;

    struct MultipleUseRequirementsPortageSyntaxTest : TestCase
    {
        MultipleUseRequirementsPortageSyntaxTest() : TestCase("multiple use requirements portage syntax") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id, "enabled disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("enabled,-disabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[enabled,-disabled]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'enabled' enabled; Flag 'disabled' disabled");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("enabled,disabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[enabled,disabled]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'enabled' enabled; Flag 'disabled' enabled");
            TEST_CHECK(! req2->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
                parse_elike_use_requirement("-enabled,-disabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req3->as_raw_string(), "[-enabled,-disabled]");
            TEST_CHECK_EQUAL(req3->as_human_string(), "Flag 'enabled' disabled; Flag 'disabled' disabled");
            TEST_CHECK(! req3->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
                parse_elike_use_requirement("enabled,-disabled,-enabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req4->as_raw_string(), "[enabled,-disabled,-enabled]");
            TEST_CHECK_EQUAL(req4->as_human_string(), "Flag 'enabled' enabled; Flag 'disabled' disabled; Flag 'enabled' disabled");
            TEST_CHECK(! req4->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req5(
                parse_elike_use_requirement("enabled,-disabled,enabled", std::shared_ptr<const PackageID>(), { euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req5->as_raw_string(), "[enabled,-disabled,enabled]");
            TEST_CHECK_EQUAL(req5->as_human_string(), "Flag 'enabled' enabled; Flag 'disabled' disabled; Flag 'enabled' enabled");
            TEST_CHECK(req5->requirement_met(&env, 0, id, 0).first);
        }
    } test_multiple_use_requirements_portage_syntax;

    struct ComplexUseRequirementsTest : TestCase
    {
        ComplexUseRequirementsTest() : TestCase("complex use requirements") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");
            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
            set_conditionals(id2, "pkgname");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("pkgname?", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req1->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("pkgname?", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
                parse_elike_use_requirement("-pkgname?", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req3->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req3->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req3->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req3->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
                parse_elike_use_requirement("-pkgname?", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req4->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req4->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req4->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req4->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req5(
                parse_elike_use_requirement("pkgname!?", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req5->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req5->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req5->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req5->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req6(
                parse_elike_use_requirement("pkgname!?", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req6->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req6->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req6->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req6->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
                parse_elike_use_requirement("-pkgname!?", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req7->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req7->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req7->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req7->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
                parse_elike_use_requirement("-pkgname!?", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req8->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req8->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req8->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req8->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
                parse_elike_use_requirement("pkgname=", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req9->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req9->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req9->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req9->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
                parse_elike_use_requirement("pkgname=", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req10->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req10->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req10->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req10->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
                parse_elike_use_requirement("pkgname!=", id, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req11->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req11->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req11->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req11->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
                parse_elike_use_requirement("pkgname!=", id2, { euro_allow_self_deps, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req12->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req12->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req12->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req12->requirement_met(&env, 0, id2, 0).first);
        }
    } test_complex_use_requirements;

    struct ComplexUseRequirementsPortageSyntaxTest : TestCase
    {
        ComplexUseRequirementsPortageSyntaxTest() : TestCase("complex use requirements portage syntax") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");
            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
            set_conditionals(id2, "pkgname");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("pkgname?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req1->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("pkgname?", id2, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
                parse_elike_use_requirement("!pkgname?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req7->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req7->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req7->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req7->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
                parse_elike_use_requirement("!pkgname?", id2, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req8->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req8->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req8->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req8->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
                parse_elike_use_requirement("pkgname=", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req9->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req9->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req9->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req9->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
                parse_elike_use_requirement("pkgname=", id2, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req10->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req10->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req10->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req10->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
                parse_elike_use_requirement("!pkgname=", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req11->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req11->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req11->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req11->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
                parse_elike_use_requirement("!pkgname=", id2, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req12->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req12->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req12->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req12->requirement_met(&env, 0, id2, 0).first);
        }
    } test_complex_use_requirements_portage_syntax;

    struct ComplexUseRequirementsBothSyntaxesTest : TestCase
    {
        ComplexUseRequirementsBothSyntaxesTest() : TestCase("complex use requirements both syntaxes") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");
            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
            set_conditionals(id2, "pkgname");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("pkgname?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req1->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("pkgname?", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
                parse_elike_use_requirement("!pkgname?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req7->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req7->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req7->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req7->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
                parse_elike_use_requirement("!pkgname?", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req8->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req8->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req8->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req8->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
                parse_elike_use_requirement("pkgname=", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req9->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req9->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req9->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req9->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
                parse_elike_use_requirement("pkgname=", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req10->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req10->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req10->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req10->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
                parse_elike_use_requirement("!pkgname=", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req11->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req11->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req11->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req11->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
                parse_elike_use_requirement("!pkgname=", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req12->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req12->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req12->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req12->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
                parse_elike_use_requirement("-pkgname?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req13->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req13->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req13->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req13->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
                parse_elike_use_requirement("-pkgname?", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req14->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req14->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req14->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req14->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
                parse_elike_use_requirement("pkgname!?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req15->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req15->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req15->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req15->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
                parse_elike_use_requirement("pkgname!?", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req16->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req16->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req16->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req16->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
                parse_elike_use_requirement("-pkgname!?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req17->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req17->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req17->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req17->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
                parse_elike_use_requirement("-pkgname!?", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req18->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req18->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req18->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req18->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
                parse_elike_use_requirement("pkgname!=", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req19->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req19->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req19->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req19->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
                parse_elike_use_requirement("pkgname!=", id2, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req20->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req20->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req20->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req20->requirement_met(&env, 0, id2, 0).first);
        }
    } test_complex_use_requirements_both_syntaxes;

    struct MalformedUseRequirementsTest : TestCase
    {
        MalformedUseRequirementsTest() : TestCase("malformed use requirements") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");

            TEST_CHECK_THROWS(parse_elike_use_requirement("", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("?", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-?", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!?", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-!?", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("=", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!=", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);

            TEST_CHECK_THROWS(parse_elike_use_requirement("!test?", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!test=", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test1,test2", id, { euro_allow_self_deps, euro_strict_parsing }), ELikeUseRequirementError);
        }
    } test_malformed_use_requirements;

    struct MalformedUseRequirementsPortageSyntaxTest : TestCase
    {
        MalformedUseRequirementsPortageSyntaxTest() : TestCase("malformed use requirements portage syntax") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");

            TEST_CHECK_THROWS(parse_elike_use_requirement("", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("=", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!=", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);

            TEST_CHECK_THROWS(parse_elike_use_requirement(",", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test,", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement(",test", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test,,test", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);

            TEST_CHECK_THROWS(parse_elike_use_requirement("test!?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-test?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-test!?", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test!=", id, { euro_allow_self_deps, euro_portage_syntax, euro_strict_parsing }), ELikeUseRequirementError);
        }
    } test_malformed_use_requirements_portage_syntax;

    struct MalformedUseRequirementsBothSyntaxesTest : TestCase
    {
        MalformedUseRequirementsBothSyntaxesTest() : TestCase("malformed use requirements both syntaxes") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");

            TEST_CHECK_THROWS(parse_elike_use_requirement("", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("-", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!?", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("=", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("!=", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);

            TEST_CHECK_THROWS(parse_elike_use_requirement(",", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test,", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement(",test", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
            TEST_CHECK_THROWS(parse_elike_use_requirement("test,,test", id, { euro_allow_self_deps, euro_both_syntaxes, euro_strict_parsing }), ELikeUseRequirementError);
        }
    } test_malformed_use_requirements_both_syntaxes;

    struct ComplexUseRequirementsNonstrictTest : TestCase
    {
        ComplexUseRequirementsNonstrictTest() : TestCase("complex use requirements non-strict") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");
            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
            set_conditionals(id2, "pkgname");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("pkgname?", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req1->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("pkgname?", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
                parse_elike_use_requirement("!pkgname?", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req7->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req7->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req7->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req7->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
                parse_elike_use_requirement("!pkgname?", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req8->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req8->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req8->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req8->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
                parse_elike_use_requirement("pkgname=", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req9->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req9->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req9->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req9->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
                parse_elike_use_requirement("pkgname=", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req10->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req10->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req10->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req10->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
                parse_elike_use_requirement("!pkgname=", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req11->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req11->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req11->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req11->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
                parse_elike_use_requirement("!pkgname=", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req12->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req12->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req12->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req12->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
                parse_elike_use_requirement("-pkgname?", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req13->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req13->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req13->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req13->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
                parse_elike_use_requirement("-pkgname?", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req14->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req14->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req14->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req14->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
                parse_elike_use_requirement("pkgname!?", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req15->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req15->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req15->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req15->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
                parse_elike_use_requirement("pkgname!?", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req16->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req16->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req16->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req16->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
                parse_elike_use_requirement("-pkgname!?", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req17->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req17->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req17->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req17->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
                parse_elike_use_requirement("-pkgname!?", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req18->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req18->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req18->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req18->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
                parse_elike_use_requirement("pkgname!=", id, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req19->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req19->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req19->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req19->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
                parse_elike_use_requirement("pkgname!=", id2, { euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req20->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req20->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req20->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req20->requirement_met(&env, 0, id2, 0).first);
        }
    } test_complex_use_requirements_nonstrict;

    struct ComplexUseRequirementsPortageSyntaxNonstrictTest : TestCase
    {
        ComplexUseRequirementsPortageSyntaxNonstrictTest() : TestCase("complex use requirements portage syntax non-strict") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "enabled", "1"));
            set_conditionals(id, "pkgname");
            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "disabled", "1"));
            set_conditionals(id2, "pkgname");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("pkgname?", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req1->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("pkgname?", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[pkgname?]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'pkgname' enabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req7(
                parse_elike_use_requirement("!pkgname?", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req7->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req7->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req7->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req7->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req8(
                parse_elike_use_requirement("!pkgname?", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req8->as_raw_string(), "[!pkgname?]");
            TEST_CHECK_EQUAL(req8->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req8->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req8->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req9(
                parse_elike_use_requirement("pkgname=", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req9->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req9->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req9->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req9->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req10(
                parse_elike_use_requirement("pkgname=", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req10->as_raw_string(), "[pkgname=]");
            TEST_CHECK_EQUAL(req10->as_human_string(), "Flag 'pkgname' enabled or disabled like it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req10->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req10->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req11(
                parse_elike_use_requirement("!pkgname=", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req11->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req11->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req11->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req11->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req12(
                parse_elike_use_requirement("!pkgname=", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req12->as_raw_string(), "[!pkgname=]");
            TEST_CHECK_EQUAL(req12->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req12->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req12->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req13(
                parse_elike_use_requirement("-pkgname?", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req13->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req13->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req13->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req13->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req14(
                parse_elike_use_requirement("-pkgname?", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req14->as_raw_string(), "[-pkgname?]");
            TEST_CHECK_EQUAL(req14->as_human_string(), "Flag 'pkgname' disabled if it is enabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req14->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req14->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req15(
                parse_elike_use_requirement("pkgname!?", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req15->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req15->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req15->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req15->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req16(
                parse_elike_use_requirement("pkgname!?", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req16->as_raw_string(), "[pkgname!?]");
            TEST_CHECK_EQUAL(req16->as_human_string(), "Flag 'pkgname' enabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req16->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req16->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req17(
                parse_elike_use_requirement("-pkgname!?", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req17->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req17->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/enabled-1:0::fake'");
            TEST_CHECK(req17->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req17->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req18(
                parse_elike_use_requirement("-pkgname!?", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req18->as_raw_string(), "[-pkgname!?]");
            TEST_CHECK_EQUAL(req18->as_human_string(), "Flag 'pkgname' disabled if it is disabled for 'cat/disabled-1:0::fake'");
            TEST_CHECK(! req18->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req18->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req19(
                parse_elike_use_requirement("pkgname!=", id, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req19->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req19->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/enabled-1:0::fake'");
            TEST_CHECK(! req19->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(req19->requirement_met(&env, 0, id2, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req20(
                parse_elike_use_requirement("pkgname!=", id2, { euro_allow_self_deps, euro_portage_syntax }));
            TEST_CHECK_EQUAL(req20->as_raw_string(), "[pkgname!=]");
            TEST_CHECK_EQUAL(req20->as_human_string(), "Flag 'pkgname' enabled or disabled opposite to how it is for 'cat/disabled-1:0::fake'");
            TEST_CHECK(req20->requirement_met(&env, 0, id, 0).first);
            TEST_CHECK(! req20->requirement_met(&env, 0, id2, 0).first);
        }
    } test_complex_use_requirements_portage_syntax_nonstrict;

    struct UseRequirementsWithDefaultsTest : TestCase
    {
        UseRequirementsWithDefaultsTest() : TestCase("use requirements with defaults") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id, "enabled disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("missing(+)", std::shared_ptr<const PackageID>(), { euro_allow_default_values, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[missing(+)]");
            TEST_CHECK_EQUAL(req1->as_human_string(), "Flag 'missing' enabled, assuming enabled if missing");
            TEST_CHECK(req1->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("missing(-)", std::shared_ptr<const PackageID>(), { euro_allow_default_values, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[missing(-)]");
            TEST_CHECK_EQUAL(req2->as_human_string(), "Flag 'missing' enabled, assuming disabled if missing");
            TEST_CHECK(! req2->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req3(
                parse_elike_use_requirement("-missing(+)", std::shared_ptr<const PackageID>(), { euro_allow_default_values, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req3->as_raw_string(), "[-missing(+)]");
            TEST_CHECK_EQUAL(req3->as_human_string(), "Flag 'missing' disabled, assuming enabled if missing");
            TEST_CHECK(! req3->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req4(
                parse_elike_use_requirement("-missing(-)", std::shared_ptr<const PackageID>(), { euro_allow_default_values, euro_strict_parsing }));
            TEST_CHECK_EQUAL(req4->as_raw_string(), "[-missing(-)]");
            TEST_CHECK_EQUAL(req4->as_human_string(), "Flag 'missing' disabled, assuming disabled if missing");
            TEST_CHECK(req4->requirement_met(&env, 0, id, 0).first);
        }
    } test_use_requirements_with_defaults;

    struct PrefixStarUseRequirementsTest : TestCase
    {
        PrefixStarUseRequirementsTest() : TestCase("prefix:*") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);
            std::shared_ptr<FakePackageID> id(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id, "foo:enabled foo:disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("foo:*", id,
                    { euro_allow_default_values, euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[foo:*]");
            TEST_CHECK(! req1->requirement_met(&env, 0, id, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("foo:*=", id,
                    { euro_allow_default_values, euro_allow_self_deps }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[foo:*=]");
            TEST_CHECK(req2->requirement_met(&env, 0, id, 0).first);
        }
    } test_prefix_star_use_requirements;

    struct PrefixQuestionDefaultsRequirementsTest : TestCase
    {
        PrefixQuestionDefaultsRequirementsTest() : TestCase("(?) defaults") { }

        void run()
        {
            TestEnvironment env;
            const std::shared_ptr<FakeRepository> fake(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                            n::environment() = &env,
                            n::name() = RepositoryName("fake")
                            )));
            env.package_database()->add_repository(1, fake);

            std::shared_ptr<FakePackageID> id1(fake->add_version("cat", "pkg1", "1"));
            set_conditionals(id1, "foo:enabled foo:disabled");

            std::shared_ptr<FakePackageID> id2(fake->add_version("cat", "pkg2", "1"));
            set_conditionals(id2, "foo:enabled foo:disabled bar:enabled bar:disabled");

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req1(
                parse_elike_use_requirement("foo:*(?)=", id2,
                    { euro_allow_default_values, euro_allow_self_deps,
                    euro_allow_default_question_values }));
            TEST_CHECK_EQUAL(req1->as_raw_string(), "[foo:*(?)=]");
            TEST_CHECK(req1->requirement_met(&env, 0, id2, 0).first);
            TEST_CHECK(req1->requirement_met(&env, 0, id1, 0).first);

            std::shared_ptr<const AdditionalPackageDepSpecRequirement> req2(
                parse_elike_use_requirement("bar:*(?)=", id2,
                    { euro_allow_default_values, euro_allow_self_deps,
                    euro_allow_default_question_values }));
            TEST_CHECK_EQUAL(req2->as_raw_string(), "[bar:*(?)=]");
            TEST_CHECK(req2->requirement_met(&env, 0, id2, 0).first);
            TEST_CHECK(req2->requirement_met(&env, 0, id1, 0).first);
        }
    } test_question_default_requirements;
}

