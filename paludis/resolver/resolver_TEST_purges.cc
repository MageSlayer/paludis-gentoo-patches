/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/resolver/resolver_functions.hh>
#include <paludis/resolver/resolution.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/constraint.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/suggest_restart.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>
#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/return_literal_function.hh>

#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>

#include <paludis/resolver/resolver_test.hh>

#include <list>
#include <functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;

namespace
{
    struct ResolverPurgesTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("purges", "exheres-0", "exheres");
        }

        void TearDown()
        {
        }
    };
}

TEST_F(ResolverPurgesTestCase, Purges)
{
    data->install("purges", "target", "0")->build_dependencies_key()->set_from_string(
            "purges/still-used-dep purges/old-dep purges/old-dep-locked purges/unrelated-dep");
    data->install("purges", "old-dep", "0");
    data->install("purges", "old-dep-locked", "0")->behaviours_set()->insert("used");
    data->install("purges", "still-used-dep", "0");
    data->install("purges", "unrelated-dep", "0");
    data->install("purges", "unrelated", "0")->build_dependencies_key()->set_from_string("purges/unrelated-dep");

    data->get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("purges/old-dep", &data->env, { }));

    data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_possible);

    std::shared_ptr<const Resolved> resolved(data->get_resolved("purges/target"));
    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("purges/new-dep"))
                .change(QualifiedPackageName("purges/target"))
                .remove(QualifiedPackageName("purges/old-dep"))
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

TEST_F(ResolverPurgesTestCase, StarSlotPurges)
{
    data->install("star-slot-purges", "target", "1")->set_slot(SlotName("1"));
    data->install("star-slot-purges", "target", "2")->set_slot(SlotName("2"));

    data->install("star-slot-purges", "uses", "1")->build_dependencies_key()->set_from_string("star-slot-purges/target:*");

    data->get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("star-slot-purges/target", &data->env, { }));

    data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_possible);

    std::shared_ptr<const Resolved> resolved(data->get_resolved(make_uninstall_blocker(
                    parse_user_package_dep_spec("star-slot-purges/target:1", &data->env, { }))));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .remove(QualifiedPackageName("star-slot-purges/target"))
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

