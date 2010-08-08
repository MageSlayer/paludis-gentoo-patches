/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/repository_factory.hh>
#include <paludis/package_database.hh>

#include <paludis/resolver/resolver_test.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <list>
#include <functional>
#include <algorithm>
#include <map>

using namespace paludis;
using namespace paludis::resolver;
using namespace paludis::resolver::resolver_test;
using namespace test;

namespace
{
    struct ResolverPurgesTestCase : ResolverTestCase
    {
        ResolverPurgesTestCase(const std::string & s) :
            ResolverTestCase("purges", s, "exheres-0", "exheres")
        {
        }
    };
}

namespace test_cases
{
    struct TestPurges : ResolverPurgesTestCase
    {
        TestPurges() :
            ResolverPurgesTestCase("purges")
        {
            install("purges", "target", "0")->build_dependencies_key()->set_from_string(
                    "purges/still-used-dep purges/old-dep purges/old-dep-locked purges/unrelated-dep");
            install("purges", "old-dep", "0");
            install("purges", "old-dep-locked", "0")->behaviours_set()->insert("used");
            install("purges", "still-used-dep", "0");
            install("purges", "unrelated-dep", "0");
            install("purges", "unrelated", "0")->build_dependencies_key()->set_from_string("purges/unrelated-dep");

            get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("purges/old-dep", &env, { }));

            get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_possible);
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("purges/target"));
            check_resolved(resolved,
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
    } test_purges;

    struct TestStarSlotPurges : ResolverPurgesTestCase
    {
        TestStarSlotPurges() :
            ResolverPurgesTestCase("star slot purges")
        {
            install("star-slot-purges", "target", "1")->set_slot(SlotName("1"));
            install("star-slot-purges", "target", "2")->set_slot(SlotName("2"));

            install("star-slot-purges", "uses", "1")->build_dependencies_key()->set_from_string("star-slot-purges/target:*");

            get_constraints_for_purge_helper.add_purge_spec(parse_user_package_dep_spec("star-slot-purges/target", &env, { }));

            get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_possible);
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved(BlockDepSpec(
                            "!star-slot-purges/target:1",
                            parse_user_package_dep_spec("star-slot-purges/target:1", &env, { }),
                            false)));

            check_resolved(resolved,
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
    } test_star_slot_purges;
}

