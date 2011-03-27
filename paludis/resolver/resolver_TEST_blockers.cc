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
#include <paludis/util/stringify.hh>

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
    struct ResolverBlockersTestCase :
        ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("blockers", "exheres-0", "exheres");
            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_never);
        }

        void TearDown()
        {
            data.reset();
        }
    };

    struct ResolverBlockers0TestCase :
        ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("blockers", "0", "exheres");
            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_never);
        }

        void TearDown()
        {
            data.reset();
        }
    };

    template <bool transient_>
    struct TestHardBlockerTestCase :
        ResolverBlockersTestCase
    {
        void common_test_code()
        {
            data->install("hard", "a-pkg", "1")->behaviours_set()->insert(transient_ ? "transient" : "");
            data->install("hard", "z-pkg", "1")->behaviours_set()->insert(transient_ ? "transient" : "");

            std::shared_ptr<const Resolved> resolved(data->get_resolved("hard/target"));
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
    };
}

typedef TestHardBlockerTestCase<false> HardBlockerTestF;
typedef TestHardBlockerTestCase<true> HardBlockerTestT;

TEST_F(HardBlockerTestF, Works) { common_test_code(); }
TEST_F(HardBlockerTestT, Works) { common_test_code(); }

namespace
{
    template <bool transient_>
    struct TestUnfixableBlockerTestCase :
        ResolverBlockersTestCase
    {
        void common_test_code()
        {
            data->install("unfixable", "a-pkg", "1")->behaviours_set()->insert(transient_ ? "transient" : "");

            std::shared_ptr<const Resolved> resolved(data->get_resolved("unfixable/target"));
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
    };
}

typedef TestUnfixableBlockerTestCase<true> UnfixableBlockerT;
typedef TestUnfixableBlockerTestCase<false> UnfixableBlockerF;

TEST_F(UnfixableBlockerT, Works) { common_test_code(); }
TEST_F(UnfixableBlockerF, Works) { common_test_code(); }

namespace
{
    template <bool transient_>
    struct TestRemoveBlockerTestCase :
        ResolverBlockersTestCase
    {
        void common_test_code()
        {
            data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("remove/a-pkg", &data->env, UserPackageDepSpecOptions()));
            data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("remove/z-pkg", &data->env, UserPackageDepSpecOptions()));
            data->install("remove", "a-pkg", "1")->behaviours_set()->insert(transient_ ? "transient" : "");
            data->install("remove", "z-pkg", "1")->behaviours_set()->insert(transient_ ? "transient" : "");

            std::shared_ptr<const Resolved> resolved(data->get_resolved("remove/target"));
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
    };
}

typedef TestRemoveBlockerTestCase<true> RemoveBlockerT;
typedef TestRemoveBlockerTestCase<false> RemoveBlockerF;

TEST_F(RemoveBlockerT, Worsk) { common_test_code(); }
TEST_F(RemoveBlockerF, Worsk) { common_test_code(); }

namespace
{
    template <bool exists_>
    struct TestTargetBlockerTestCase :
        ResolverBlockersTestCase
    {
        void common_test_code()
        {
            data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("target/target", &data->env, UserPackageDepSpecOptions()));

            if (exists_)
                data->install("target", "target", "1");

            std::shared_ptr<const Resolved> resolved(data->get_resolved(make_uninstall_blocker(
                            parse_user_package_dep_spec("target/target", &data->env, UserPackageDepSpecOptions()))));

            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = exists_ ? make_shared_copy(DecisionChecks()
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
    };
}

typedef TestTargetBlockerTestCase<true> TestTargetT;
typedef TestTargetBlockerTestCase<false> TestTargetF;

TEST_F(TestTargetT, Works) { common_test_code(); }
TEST_F(TestTargetF, Works) { common_test_code(); }

namespace
{
    template <bool exists_, bool allowed_>
    struct BlockedAndDepTestCase :
        ResolverBlockersTestCase
    {
        void common_test_code()
        {
            if (allowed_)
                data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("blocked-and-dep/both", &data->env, UserPackageDepSpecOptions()));

            if (exists_)
                data->install("blocked-and-dep", "both", "1");

            std::shared_ptr<const Resolved> resolved(data->get_resolved("blocked-and-dep/target"));

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
    };
}

typedef BlockedAndDepTestCase<false, false> BlockedAndDepFF;
typedef BlockedAndDepTestCase<false, true> BlockedAndDepFT;
typedef BlockedAndDepTestCase<true, false> BlockedAndDepTF;
typedef BlockedAndDepTestCase<true, true> BlockedAndDepTT;

TEST_F(BlockedAndDepFF, Works) { common_test_code(); }
TEST_F(BlockedAndDepFT, Works) { common_test_code(); }
TEST_F(BlockedAndDepTF, Works) { common_test_code(); }
TEST_F(BlockedAndDepTT, Works) { common_test_code(); }

TEST_F(ResolverBlockers0TestCase, Cycle)
{
    data->install("block-and-dep-cycle", "target", "0");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("block-and-dep-cycle/target"));

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

TEST_F(ResolverBlockers0TestCase, HardBlockAndDepCycle)
{
    data->install("hard-block-and-dep-cycle", "target", "0");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("hard-block-and-dep-cycle/target"));

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

namespace
{
    template <int installed_version_, int dep_version_, bool strong_>
    struct SelfBlockTestCase :
        ResolverBlockers0TestCase
    {
        void common_test_code()
        {
            std::string cat(std::string("self-block-") +
                    (-1 == installed_version_ ? "x" : stringify(installed_version_)) + "-" +
                    (-1 == dep_version_ ? "x" : stringify(dep_version_)) + "-" +
                    (strong_ ? "s" : "w"));

            if (installed_version_ != -1)
                data->install(cat, "dep", stringify(installed_version_));

            data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec(stringify(cat) + "/dep", &data->env, UserPackageDepSpecOptions()));

            std::shared_ptr<const Resolved> resolved(data->get_resolved(cat + "/target"));
            std::shared_ptr<DecisionChecks> checks, u_checks, o_checks;

            if (dep_version_ != 0)
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
            else if (installed_version_ == -1)
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
            else if (installed_version_ == 1 || ((! strong_) && installed_version_ == 0))
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
            else if (strong_ && installed_version_ == 0 && dep_version_ == 0)
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

            ASSERT_TRUE(bool(checks));
            ASSERT_TRUE(bool(u_checks));
            ASSERT_TRUE(bool(o_checks));

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
    };
}

typedef SelfBlockTestCase<-1, -1, false> TestSelfBlockXXW;
typedef SelfBlockTestCase<-1, -1, true> TestSelfBlockXXS;
typedef SelfBlockTestCase< 0, -1, false> TestSelfBlock0XW;
typedef SelfBlockTestCase< 0, -1, true> TestSelfBlock0XS;
typedef SelfBlockTestCase< 1, -1, false> TestSelfBlock1XW;
typedef SelfBlockTestCase< 1, -1, true> TestSelfBlock1XS;
typedef SelfBlockTestCase<-1,  0, false> TestSelfBlockX0W;
typedef SelfBlockTestCase<-1,  0, true> TestSelfBlockX0S;
typedef SelfBlockTestCase< 0,  0, false> TestSelfBlock00W;
typedef SelfBlockTestCase< 0,  0, true> TestSelfBlock00S;
typedef SelfBlockTestCase< 1,  0, false> TestSelfBlock10W;
typedef SelfBlockTestCase< 1,  0, true> TestSelfBlock10S;
typedef SelfBlockTestCase<-1,  1, false> TestSelfBlockX1W;
typedef SelfBlockTestCase<-1,  1, true> TestSelfBlockX1S;
typedef SelfBlockTestCase< 0,  1, false> TestSelfBlock01W;
typedef SelfBlockTestCase< 0,  1, true> TestSelfBlock01S;
typedef SelfBlockTestCase< 1,  1, false> TestSelfBlock11W;
typedef SelfBlockTestCase< 1,  1, true> TestSelfBlock11S;

TEST_F(TestSelfBlockXXW, Works) { common_test_code(); }
TEST_F(TestSelfBlockXXS, Works) { common_test_code(); }
TEST_F(TestSelfBlock0XW, Works) { common_test_code(); }
TEST_F(TestSelfBlock0XS, Works) { common_test_code(); }
TEST_F(TestSelfBlock1XW, Works) { common_test_code(); }
TEST_F(TestSelfBlock1XS, Works) { common_test_code(); }
TEST_F(TestSelfBlockX0W, Works) { common_test_code(); }
TEST_F(TestSelfBlockX0S, Works) { common_test_code(); }
TEST_F(TestSelfBlock00W, Works) { common_test_code(); }
TEST_F(TestSelfBlock00S, Works) { common_test_code(); }
TEST_F(TestSelfBlock10W, Works) { common_test_code(); }
TEST_F(TestSelfBlock10S, Works) { common_test_code(); }
TEST_F(TestSelfBlockX1W, Works) { common_test_code(); }
TEST_F(TestSelfBlockX1S, Works) { common_test_code(); }
TEST_F(TestSelfBlock01W, Works) { common_test_code(); }
TEST_F(TestSelfBlock01S, Works) { common_test_code(); }
TEST_F(TestSelfBlock11W, Works) { common_test_code(); }
TEST_F(TestSelfBlock11S, Works) { common_test_code(); }

TEST_F(ResolverBlockersTestCase, UninstallBlockedAfter)
{
    data->install("uninstall-blocked-after", "dep", "1");
    data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("uninstall-blocked-after/dep", &data->env, UserPackageDepSpecOptions()));

    std::shared_ptr<const Resolved> resolved(data->get_resolved("uninstall-blocked-after/target"));

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

TEST_F(ResolverBlockersTestCase, UninstallBlockedBefore)
{
    data->install("uninstall-blocked-before", "dep", "1");
    data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("uninstall-blocked-before/dep", &data->env, UserPackageDepSpecOptions()));
    std::shared_ptr<const Resolved> resolved(data->get_resolved("uninstall-blocked-before/target"));

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

TEST_F(ResolverBlockersTestCase, UpgradeBlockedBefore)
{
    data->install("upgrade-blocked-before", "dep", "1");
    data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("upgrade-blocked-before/dep", &data->env, UserPackageDepSpecOptions()));
    std::shared_ptr<const Resolved> resolved(data->get_resolved("upgrade-blocked-before/target"));

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

TEST_F(ResolverBlockersTestCase, Manual)
{
    data->install("manual", "dep", "1");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("manual/target"));

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

TEST_F(ResolverBlockersTestCase, OtherSlotFirst)
{
    data->install("other-slot-first", "dep", "1")->set_slot(SlotName("1"));
    std::shared_ptr<const Resolved> resolved(data->get_resolved("other-slot-first/target"));

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

