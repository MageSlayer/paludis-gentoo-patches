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
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/simple_visitor_cast.hh>
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
    struct ResolverUninstallsTestCase : ResolverTestCase
    {
        ResolverUninstallsTestCase(const std::string & s) :
            ResolverTestCase("uninstalls", s, "exheres-0", "exheres")
        {
        }
    };
}

namespace test_cases
{
    struct TestUninstallBreaking : ResolverUninstallsTestCase
    {
        const bool allowed_to_remove;
        const bool confirm;

        TestUninstallBreaking(const bool ar, const bool c) :
            ResolverUninstallsTestCase("uninstall breaking " + stringify(ar) + " " + stringify(c)),
            allowed_to_remove(ar),
            confirm(c)
        {
            install("breaking", "dep", "1")->run_dependencies_key()->set_from_string("breaking/target");
            install("breaking", "target", "1");

            allowed_to_remove_names->insert(QualifiedPackageName("breaking/target"));
            if (allowed_to_remove)
            {
                remove_if_dependent_names->insert(QualifiedPackageName("breaking/dep"));
                allowed_to_remove_names->insert(QualifiedPackageName("breaking/dep"));
            }
        }

        virtual ResolverFunctions get_resolver_functions(InitialConstraints & initial_constraints)
        {
            ResolverFunctions result(ResolverUninstallsTestCase::get_resolver_functions(initial_constraints));
            result.confirm_fn() = std::bind(return_literal_function(confirm));
            return result;
        }

        void run()
        {
            std::shared_ptr<const Resolved> resolved(get_resolved(BlockDepSpec(
                            "!breaking/target",
                            parse_user_package_dep_spec("breaking/target", &env, { }),
                            false)));

            if (allowed_to_remove)
                check_resolved(resolved,
                        n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                            .remove(QualifiedPackageName("breaking/dep"))
                            .remove(QualifiedPackageName("breaking/target"))
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
            else if (confirm)
                check_resolved(resolved,
                        n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                            .remove(QualifiedPackageName("breaking/target"))
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
            else
                check_resolved(resolved,
                        n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                            .remove(QualifiedPackageName("breaking/target"))
                            .finished()),
                        n::taken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                            .finished()),
                        n::taken_unconfirmed_decisions() = make_shared_copy(DecisionChecks()
                            .breaking(QualifiedPackageName("breaking/dep"))
                            .finished()),
                        n::taken_unorderable_decisions() = make_shared_copy(DecisionChecks()
                            .finished()),
                        n::untaken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                            .finished()),
                        n::untaken_unable_to_make_decisions() = make_shared_copy(DecisionChecks()
                            .finished())
                        );
        }
    } test_uninstall_breaking_f_f(false, false),
      test_uninstall_breaking_f_t(false, true),
      test_uninstall_breaking_t_f(true, false),
      test_uninstall_breaking_t_t(true, true);
}

