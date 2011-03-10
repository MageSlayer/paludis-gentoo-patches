/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_shared_copy.hh>
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
    struct ResolverBlockersTestCase : ResolverTestCase
    {
        ResolverBlockersTestCase(const std::string & s, const std::string & e = "exheres-0") :
            ResolverTestCase("blockers", s, e, "exheres")
        {
            get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_never);
        }
    };
}

namespace test_cases
{
    struct TestHardBlocker : ResolverBlockersTestCase
    {
        const bool transient;

        TestHardBlocker(const bool t) :
            ResolverBlockersTestCase("hard" + std::string(t ? " transient" : "")),
            transient(t)
        {
            install("hard", "a-pkg", "1");
            install("hard", "z-pkg", "1");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("hard/target"));
            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("hard/a-pkg"))
                        .change(QualifiedPackageName("hard/z-pkg"))
                        .change(QualifiedPackageName("hard/target"))
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
    } test_hard_blocker(false), test_hard_blocker_transient(true);

    struct TestUnfixableBlocker : ResolverBlockersTestCase
    {
        const bool transient;

        TestUnfixableBlocker(const bool t) :
            ResolverBlockersTestCase("unfixable" + std::string(t ? " transient" : "")),
            transient(t)
        {
            install("unfixable", "a-pkg", "1")->behaviours_set()->insert(transient ? "transient" : "");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("unfixable/target"));
            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("unfixable/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName("unfixable/a-pkg"))
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
    } test_unfixable_blocker(false), test_unfixable_blocker_transient(true);

    struct TestRemoveBlocker : ResolverBlockersTestCase
    {
        const bool transient;

        TestRemoveBlocker(const bool t) :
            ResolverBlockersTestCase("remove" + std::string(t ? " transient" : "")),
            transient(t)
        {
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("remove/a-pkg", &env, { }));
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("remove/z-pkg", &env, { }));
            install("remove", "a-pkg", "1")->behaviours_set()->insert(transient ? "transient" : "");
            install("remove", "z-pkg", "1")->behaviours_set()->insert(transient ? "transient" : "");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("remove/target"));
            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("remove/a-pkg"))
                        .remove(QualifiedPackageName("remove/z-pkg"))
                        .change(QualifiedPackageName("remove/target"))
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
    } test_remove_blocker(false), test_remove_blocker_transient(true);

    struct TestTargetBlocker : ResolverBlockersTestCase
    {
        const bool exists;

        TestTargetBlocker(const bool x) :
            ResolverBlockersTestCase("target" + std::string(x ? " exists" : "")),
            exists(x)
        {
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("target/target", &env, { }));

            if (exists)
                install("target", "target", "1");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved(make_uninstall_blocker(
                            parse_user_package_dep_spec("target/target", &env, { }))));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = exists ? make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("target/target"))
                        .finished()) : make_shared_copy(DecisionChecks()
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
    } test_target(false), test_target_exists(true);

    struct BlockedAndDep : ResolverBlockersTestCase
    {
        const bool exists;
        const bool allowed;

        BlockedAndDep(const bool x, const bool a) :
            ResolverBlockersTestCase("blocked and dep"
                    + std::string(x ? " exists" : "")
                    + std::string(a ? " allowed" : "")),
            exists(x),
            allowed(a)
        {
            if (allowed)
                allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("blocked-and-dep/both", &env, { }));

            if (exists)
                install("blocked-and-dep", "both", "1");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("blocked-and-dep/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("blocked-and-dep/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName("blocked-and-dep/both"))
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
    } test_blocked_and_dep(false, false),
        test_blocked_and_dep_exists(true, false),
        test_blocked_and_dep_allowed(false, true),
        test_blocked_and_dep_exists_allowed(true, true);

    struct BlockAndDepCycle : ResolverBlockersTestCase
    {
        BlockAndDepCycle() :
            ResolverBlockersTestCase("block and dep cycle", "0")
        {
            install("block-and-dep-cycle", "target", "0");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("block-and-dep-cycle/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("block-and-dep-cycle/dep"))
                        .change(QualifiedPackageName("block-and-dep-cycle/target"))
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
    } test_block_and_dep_cycle;

    struct HardBlockAndDepCycle : ResolverBlockersTestCase
    {
        HardBlockAndDepCycle() :
            ResolverBlockersTestCase("hard block and dep cycle", "0")
        {
            install("hard-block-and-dep-cycle", "target", "0");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("hard-block-and-dep-cycle/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("hard-block-and-dep-cycle/dep"))
                        .change(QualifiedPackageName("hard-block-and-dep-cycle/target"))
                        .finished()),
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_hard_block_and_dep_cycle;

    struct SelfBlock : ResolverBlockersTestCase
    {
        const int installed_version;
        const int dep_version;
        const bool strong;
        const std::string cat;

        SelfBlock(int i, int d, bool s) :
            ResolverBlockersTestCase("self block " + stringify(i) + " " + stringify(d) + " " + stringify(s), "0"),
            installed_version(i),
            dep_version(d),
            strong(s),
            cat(std::string("self-block-") +
                    (-1 == installed_version ? "x" : stringify(installed_version)) + "-" +
                    (-1 == dep_version ? "x" : stringify(dep_version)) + "-" +
                    (strong ? "s" : "w"))
        {
            if (installed_version != -1)
                install(cat, "dep", stringify(installed_version));

            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec(stringify(cat) + "/dep", &env, { }));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved(cat + "/target"));
            std::shared_ptr<DecisionChecks> checks, u_checks, o_checks;

            if (dep_version != 0)
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName(cat + "/dep"))
                        .finished());
                o_checks = make_shared_copy(DecisionChecks()
                        .finished());
            }
            else if (installed_version == -1)
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/dep"))
                        .change(QualifiedPackageName(cat + "/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .finished());
                o_checks = make_shared_copy(DecisionChecks()
                        .finished());
            }
            else if (installed_version == 1 || ((! strong) && installed_version == 0))
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/dep"))
                        .change(QualifiedPackageName(cat + "/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .finished());
                o_checks = make_shared_copy(DecisionChecks()
                        .finished());
            }
            else if (strong && installed_version == 0 && dep_version == 0)
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .finished());
                o_checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/dep"))
                        .finished());
            }

            TEST_CHECK(bool(checks));
            TEST_CHECK(bool(u_checks));
            TEST_CHECK(bool(o_checks));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = checks,
                    n::taken_unable_to_make_decisions() = u_checks,
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = o_checks,
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    } test_self_block_x_x_w(-1, -1, false), test_self_block_x_x_s(-1, -1, true),
      test_self_block_0_x_w( 0, -1, false), test_self_block_0_x_s( 0, -1, true),
      test_self_block_1_x_w( 1, -1, false), test_self_block_1_x_s( 1, -1, true),
      test_self_block_x_0_w(-1,  0, false), test_self_block_x_0_s(-1,  0, true),
      test_self_block_0_0_w( 0,  0, false), test_self_block_0_0_s( 0,  0, true),
      test_self_block_1_0_w( 1,  0, false), test_self_block_1_0_s( 1,  0, true),
      test_self_block_x_1_w(-1,  1, false), test_self_block_x_1_s(-1,  1, true),
      test_self_block_0_1_w( 0,  1, false), test_self_block_0_1_s( 0,  1, true),
      test_self_block_1_1_w( 1,  1, false), test_self_block_1_1_s( 1,  1, true);

    struct UninstallBlockedAfter : ResolverBlockersTestCase
    {
        UninstallBlockedAfter() :
            ResolverBlockersTestCase("uninstall blocked after")
        {
            install("uninstall-blocked-after", "dep", "1");
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("uninstall-blocked-after/dep", &env, { }));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("uninstall-blocked-after/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("uninstall-blocked-after/target"))
                        .remove(QualifiedPackageName("uninstall-blocked-after/dep"))
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
    } test_uninstall_blocked_after;

    struct UninstallBlockedBefore : ResolverBlockersTestCase
    {
        UninstallBlockedBefore() :
            ResolverBlockersTestCase("uninstall blocked before")
        {
            install("uninstall-blocked-before", "dep", "1");
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("uninstall-blocked-before/dep", &env, { }));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("uninstall-blocked-before/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("uninstall-blocked-before/dep"))
                        .change(QualifiedPackageName("uninstall-blocked-before/target"))
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
    } test_uninstall_blocked_before;

    struct UpgradeBlockedBefore : ResolverBlockersTestCase
    {
        UpgradeBlockedBefore() :
            ResolverBlockersTestCase("upgrade blocked before")
        {
            install("upgrade-blocked-before", "dep", "1");
            allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("upgrade-blocked-before/dep", &env, { }));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("upgrade-blocked-before/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("upgrade-blocked-before/dep"))
                        .change(QualifiedPackageName("upgrade-blocked-before/target"))
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
    } test_upgrade_blocked_before;

    struct Manual : ResolverBlockersTestCase
    {
        Manual() :
            ResolverBlockersTestCase("manual")
        {
            install("manual", "dep", "1");
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("manual/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("manual/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName("manual/dep"))
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
    } test_manual;

    struct UpgradeOtherSlotFirst : ResolverBlockersTestCase
    {
        UpgradeOtherSlotFirst() :
            ResolverBlockersTestCase("other-slot-first")
        {
            install("other-slot-first", "dep", "1")->set_slot(SlotName("1"));
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved("other-slot-first/target"));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change_slot(QualifiedPackageName("other-slot-first/dep"), SlotName("1"))
                        .change_slot(QualifiedPackageName("other-slot-first/dep"), SlotName("2"))
                        .change(QualifiedPackageName("other-slot-first/target"))
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
    } test_upgrade_other_slot_first;
}

