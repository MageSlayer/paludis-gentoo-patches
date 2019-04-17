/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 David Leverton
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

#include <paludis/util/make_shared_copy.hh>

#include <paludis/partially_made_package_dep_spec.hh>

#include <paludis/resolver/resolver_test.hh>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

namespace
{
    struct ResolverSubslotsTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp() override
        {
            data = std::make_shared<ResolverTestData>("subslots", "exheres-0", "exheres");
        }

        void TearDown() override
        {
            data.reset();
        }
    };
}

TEST_F(ResolverSubslotsTestCase, SubslotChange)
{
    data->install("subslot", "dependency", "1")->set_slot(SlotName("0"), SlotName("1"));
    data->install("subslot", "uses-library", "1")->build_dependencies_target_key()->set_from_string("=subslot/dependency-1:=0/1");

    data->remove_if_dependent_helper.add_remove_if_dependent_spec(make_package_dep_spec({ }));
    data->get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(make_package_dep_spec({ }));
    data->order_early_helper.add_early_spec(make_package_dep_spec({ }).package(QualifiedPackageName("subslot/uses-library")));

    std::shared_ptr<const Resolved> resolved(data->get_resolved("=subslot/dependency-2"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("subslot/dependency"))
                .change(QualifiedPackageName("subslot/uses-library"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverSubslotsTestCase, SubslotNoChange)
{
    data->install("subslot", "dependency", "1")->set_slot(SlotName("0"), SlotName("1"));
    data->install("subslot", "uses-library", "1")->build_dependencies_target_key()->set_from_string("=subslot/dependency-1:=0/1");

    data->remove_if_dependent_helper.add_remove_if_dependent_spec(make_package_dep_spec({ }));
    data->get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(make_package_dep_spec({ }));
    data->order_early_helper.add_early_spec(make_package_dep_spec({ }).package(QualifiedPackageName("subslot/uses-library")));
    data->confirm_helper.add_permit_old_version_spec(make_package_dep_spec({ }));

    std::shared_ptr<const Resolved> resolved(data->get_resolved("=subslot/dependency-1.1"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("subslot/uses-library"))
                .change(QualifiedPackageName("subslot/dependency"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverSubslotsTestCase, SubslotIrrelevant)
{
    data->install("subslot", "dependency", "1")->set_slot(SlotName("0"), SlotName("1"));
    data->install("subslot", "uses-tool", "1")->build_dependencies_target_key()->set_from_string("=subslot/dependency-1:*");

    data->remove_if_dependent_helper.add_remove_if_dependent_spec(make_package_dep_spec({ }));
    data->get_constraints_for_dependent_helper.add_less_restrictive_remove_blockers_spec(make_package_dep_spec({ }));
    data->order_early_helper.add_early_spec(make_package_dep_spec({ }).package(QualifiedPackageName("subslot/uses-tool")));

    std::shared_ptr<const Resolved> resolved(data->get_resolved("=subslot/dependency-2"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("subslot/uses-tool"))
                .change(QualifiedPackageName("subslot/dependency"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

