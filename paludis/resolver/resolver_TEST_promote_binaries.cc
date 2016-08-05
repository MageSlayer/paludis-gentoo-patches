/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 Dimitry Ishenko
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

#include <paludis/resolver/resolver.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/decisions.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor_cast.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/action.hh>
#include <paludis/choice.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

#include <paludis/resolver/resolver_test.hh>

#include <functional>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

namespace
{
    struct ResolverPromoteBinariesTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverWithBinaryTestData> data;
        static bool bin_created;

        void SetUp() override;
        void TearDown() override;

        std::shared_ptr<const PackageID> get_origin_id(std::shared_ptr<const Resolved> resolved)
        {
            return visitor_cast<const ChangesToMakeDecision>(*resolved->taken_change_or_remove_decisions()->begin()->first)->origin_id();
        }
    };
}

bool ResolverPromoteBinariesTestCase::bin_created(false);

void ResolverPromoteBinariesTestCase::SetUp()
{
    data = std::make_shared<ResolverWithBinaryTestData>("promote_binaries", "exheres-0", "exheres", true);

    if (!bin_created)
    {
        InstallAction install(make_named_values<InstallActionOptions>(
                n::destination() = data->bin_repo,
                n::make_output_manager() = [](const Action &) { return std::make_shared<StandardOutputManager>(); },
                n::perform_uninstall() = [](const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
                    { if (id) throw InternalError(PALUDIS_HERE, "cannot uninstall"); },
                n::replacing() = std::make_shared<PackageIDSequence>(),
                n::want_phase() = [](const std::string &) { return wp_yes; }
                ));

        auto pkg1(*data->env[selection::RequireExactlyOne(generator::Matches(
                parse_user_package_dep_spec("=cat/pkg1-1", &data->env, { }), nullptr, { }))]->last());

        auto pkg2(*data->env[selection::RequireExactlyOne(generator::Matches(
                parse_user_package_dep_spec("=cat/pkg2-1", &data->env, { }), nullptr, { }))]->last());

        data->env.set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("opt"), true);
        pkg1->perform_action(install);

        data->env.set_want_choice_enabled(ChoicePrefixName(""), UnprefixedChoiceName("opt"), false);
        pkg2->perform_action(install);

        bin_created = true;
    }
}

void ResolverPromoteBinariesTestCase::TearDown()
{
    data.reset();
}

TEST_F(ResolverPromoteBinariesTestCase, Cat_Pkg1_Repo)
{
    data->promote_binaries_helper.set_promote_binaries(pb_never);

    auto resolved(data->get_resolved("cat/pkg1"));
    ASSERT_FALSE(resolved->taken_change_or_remove_decisions()->empty());

    ASSERT_EQ(get_origin_id(resolved)->repository_name(), RepositoryName("repo"));
}

TEST_F(ResolverPromoteBinariesTestCase, Cat_Pkg1_Binrepo)
{
    data->promote_binaries_helper.set_promote_binaries(pb_if_same);

    auto resolved(data->get_resolved("cat/pkg1"));
    ASSERT_FALSE(resolved->taken_change_or_remove_decisions()->empty());

    ASSERT_NE(get_origin_id(resolved)->repository_name(), RepositoryName("binrepo"));
}

TEST_F(ResolverPromoteBinariesTestCase, Cat_Pkg2_Repo)
{
    data->promote_binaries_helper.set_promote_binaries(pb_never);

    auto resolved(data->get_resolved("cat/pkg2"));
    ASSERT_FALSE(resolved->taken_change_or_remove_decisions()->empty());

    ASSERT_EQ(get_origin_id(resolved)->repository_name(), RepositoryName("repo"));
}

TEST_F(ResolverPromoteBinariesTestCase, Cat_Pkg2_Binrepo)
{
    data->promote_binaries_helper.set_promote_binaries(pb_if_same);

    auto resolved(data->get_resolved("cat/pkg2"));
    ASSERT_FALSE(resolved->taken_change_or_remove_decisions()->empty());

    ASSERT_EQ(get_origin_id(resolved)->repository_name(), RepositoryName("binrepo"));
}

