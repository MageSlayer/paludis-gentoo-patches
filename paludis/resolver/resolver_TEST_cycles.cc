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

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
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
    struct ResolverCyclesTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("cycles", "exheres-0", "exheres");
            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_never);
        }

        void TearDown()
        {
            data.reset();
        }
    };
}

TEST_F(ResolverCyclesTestCase, NoChanges)
{
    data->install("no-changes", "dep-a", "1")->build_dependencies_key()->set_from_string("no-changes/dep-b");
    data->install("no-changes", "dep-b", "1")->build_dependencies_key()->set_from_string("no-changes/dep-a");

    data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same);

    std::shared_ptr<const Resolved> resolved(data->get_resolved("no-changes/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("no-changes/target"))
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

TEST_F(ResolverCyclesTestCase, ExistingUsable)
{
    data->install("existing-usable", "dep", "1");
    std::shared_ptr<const Resolved> resolved(data->get_resolved("existing-usable/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("existing-usable/target"))
                .change(QualifiedPackageName("existing-usable/dep"))
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

TEST_F(ResolverCyclesTestCase, MutualRunDeps)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("mutual-run-deps/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("mutual-run-deps/dep-a"))
                .change(QualifiedPackageName("mutual-run-deps/dep-b"))
                .change(QualifiedPackageName("mutual-run-deps/dep-c"))
                .change(QualifiedPackageName("mutual-run-deps/target"))
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

TEST_F(ResolverCyclesTestCase, MutualBuildDeps)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("mutual-build-deps/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("mutual-build-deps/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("mutual-build-deps/dep-a"))
                .change(QualifiedPackageName("mutual-build-deps/dep-b"))
                .change(QualifiedPackageName("mutual-build-deps/dep-c"))
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

namespace
{
    template <bool b_installed_, bool c_installed_>
    struct ResolverCyclesTriangleInstalledTestCase :
        ResolverCyclesTestCase
    {
        void common_test_code()
        {
            if (b_installed_)
                data->install("triangle", "dep-b", "1");
            if (c_installed_)
                data->install("triangle", "dep-c", "1");

            std::shared_ptr<const Resolved> resolved(data->get_resolved("triangle/target"));
            std::shared_ptr<DecisionChecks> checks, u_checks;

            if (b_installed_)
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("triangle/dep-c"))
                        .change(QualifiedPackageName("triangle/dep-a"))
                        .change(QualifiedPackageName("triangle/dep-b"))
                        .change(QualifiedPackageName("triangle/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .finished());
            }
            else if (c_installed_)
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("triangle/dep-a"))
                        .change(QualifiedPackageName("triangle/dep-b"))
                        .change(QualifiedPackageName("triangle/dep-c"))
                        .change(QualifiedPackageName("triangle/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .finished());
            }
            else
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("triangle/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("triangle/dep-a"))
                        .change(QualifiedPackageName("triangle/dep-b"))
                        .change(QualifiedPackageName("triangle/dep-c"))
                        .finished());
            }

            this->check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = checks,
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = u_checks,
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    };
}

typedef ResolverCyclesTriangleInstalledTestCase<false, false> TriangleInstalledFF;
typedef ResolverCyclesTriangleInstalledTestCase<false, true> TriangleInstalledFT;
typedef ResolverCyclesTriangleInstalledTestCase<true, false> TriangleInstalledTF;

TEST_F(TriangleInstalledFF, Works) { common_test_code(); }
TEST_F(TriangleInstalledFT, Works) { common_test_code(); }
TEST_F(TriangleInstalledTF, Works) { common_test_code(); }

namespace
{
    template <int installed_version_, bool runtime_>
    struct ResolverCyclesSelfTestCase :
        ResolverCyclesTestCase
    {
        void common_test_code()
        {
            std::string cat(runtime_ ? "self-run" : "self-build");

            if (-1 != installed_version_)
                data->install(cat, "dep", stringify(installed_version_));

            data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same);

            std::shared_ptr<const Resolved> resolved(data->get_resolved(cat + "/target"));
            std::shared_ptr<DecisionChecks> checks, u_checks;

            if (runtime_ || installed_version_ == 1)
            {
                if (installed_version_ == 1)
                {
                    checks = make_shared_copy(DecisionChecks()
                            .change(QualifiedPackageName(cat + "/target"))
                            .finished());
                    u_checks = make_shared_copy(DecisionChecks()
                            .finished());
                }
                else
                {
                    checks = make_shared_copy(DecisionChecks()
                            .change(QualifiedPackageName(cat + "/dep"))
                            .change(QualifiedPackageName(cat + "/target"))
                            .finished());
                    u_checks = make_shared_copy(DecisionChecks()
                            .finished());
                }
            }
            else
            {
                checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/target"))
                        .finished());
                u_checks = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName(cat + "/dep"))
                        .finished());
            }

            this->check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = checks,
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unorderable_decisions() = u_checks,
                    n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .finished()),
                    n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .finished())
                    );
        }
    };
}

typedef ResolverCyclesSelfTestCase<-1, false> TriangleSelfNoneFalse;
typedef ResolverCyclesSelfTestCase<-1, true> TriangleSelfNoneTrue;
typedef ResolverCyclesSelfTestCase<0, false> TriangleSelf0False;
typedef ResolverCyclesSelfTestCase<0, true> TriangleSelf0True;
typedef ResolverCyclesSelfTestCase<1, false> TriangleSelf1False;
typedef ResolverCyclesSelfTestCase<1, true> TriangleSelf1True;

TEST_F(TriangleSelfNoneFalse, Works) { common_test_code(); }
TEST_F(TriangleSelfNoneTrue, Works) { common_test_code(); }
TEST_F(TriangleSelf0False, Works) { common_test_code(); }
TEST_F(TriangleSelf0True, Works) { common_test_code(); }
TEST_F(TriangleSelf1False, Works) { common_test_code(); }
TEST_F(TriangleSelf1True, Works) { common_test_code(); }

TEST_F(ResolverCyclesTestCase, CycleDeps)
{
    data->install("cycle-deps", "dep-g", "1")->build_dependencies_key()->set_from_string("cycle-deps/dep-c");

    data->get_use_existing_nothing_helper.set_use_existing_for_dependencies(ue_if_same);

    std::shared_ptr<const Resolved> resolved(data->get_resolved("cycle-deps/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("cycle-deps/target"))
                .finished()),
            n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("cycle-deps/dep-d"))
                .change(QualifiedPackageName("cycle-deps/dep-e"))
                .change(QualifiedPackageName("cycle-deps/dep-f"))
                .change(QualifiedPackageName("cycle-deps/dep-a"))
                .change(QualifiedPackageName("cycle-deps/dep-b"))
                .change(QualifiedPackageName("cycle-deps/dep-c"))
                .finished()),
            n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .finished()),
            n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                .finished())
            );
}

TEST_F(ResolverCyclesTestCase, BuildAgainstBlock)
{
    std::shared_ptr<const Resolved> resolved(data->get_resolved("build-against-block/target"));

    this->check_resolved(resolved,
            n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                .change(QualifiedPackageName("build-against-block/dep-b"))
                .change(QualifiedPackageName("build-against-block/dep-a"))
                .change(QualifiedPackageName("build-against-block/target"))
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

