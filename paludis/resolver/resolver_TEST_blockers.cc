/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/resolver/resolver_lists.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
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
#include <tr1/functional>
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
        ResolverBlockersTestCase(const std::string & s) :
            ResolverTestCase("blockers", s, "exheres-0", "exheres")
        {
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
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("hard/target"));
            check_resolved(resolved,
                    n::display_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("hard/a-pkg"))
                        .change(QualifiedPackageName("hard/z-pkg"))
                        .change(QualifiedPackageName("hard/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
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
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("unfixable/target"));
            check_resolved(resolved,
                    n::display_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("unfixable/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName("unfixable/a-pkg"))
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
            allowed_to_remove_names->insert(QualifiedPackageName("remove/a-pkg"));
            install("remove", "a-pkg", "1")->behaviours_set()->insert(transient ? "transient" : "");
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("remove/target"));
            check_resolved(resolved,
                    n::display_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("remove/a-pkg"))
                        .change(QualifiedPackageName("remove/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
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
            allowed_to_remove_names->insert(QualifiedPackageName("target/target"));

            if (exists)
                install("target", "target", "1");
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved(BlockDepSpec(
                            "!target/target",
                            parse_user_package_dep_spec("target/target", &env, UserPackageDepSpecOptions()),
                            false)));

            check_resolved(resolved,
                    n::display_change_or_remove_decisions() = exists ? make_shared_copy(DecisionChecks()
                        .remove(QualifiedPackageName("target/target"))
                        .finished()) : make_shared_copy(DecisionChecks()
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
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
                allowed_to_remove_names->insert(QualifiedPackageName("blocked-and-dep/both"));

            if (exists)
                install("blocked-and-dep", "both", "1");
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("blocked-and-dep/target"));

            check_resolved(resolved,
                    n::display_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("blocked-and-dep/target"))
                        .finished()),
                    n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                        .unable(QualifiedPackageName("blocked-and-dep/both"))
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
}

