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
#include <paludis/resolver/required_confirmations.hh>
#include <paludis/resolver/make_uninstall_blocker.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/map.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/visitor_cast.hh>
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
    struct ResolverUninstallsTestCase : ResolverTestCase
    {
        std::shared_ptr<ResolverTestData> data;

        void SetUp()
        {
            data = std::make_shared<ResolverTestData>("uninstalls", "exheres-0", "exheres");
        }

        void TearDown()
        {
            data.reset();
        }
    };
}

namespace
{
    template <bool allowed_to_remove_, bool confirm_>
    struct TestUninstallBreaking :
        ResolverUninstallsTestCase
    {
        void common_test_code()
        {
            data->install("breaking", "dep", "1")->run_dependencies_key()->set_from_string("breaking/target");
            data->install("breaking", "target", "1");

            data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("breaking/target", &data->env, UserPackageDepSpecOptions()));
            if (allowed_to_remove_)
            {
                data->remove_if_dependent_helper.add_remove_if_dependent_spec(parse_user_package_dep_spec("breaking/dep", &data->env, UserPackageDepSpecOptions()));
                data->allowed_to_remove_helper.add_allowed_to_remove_spec(parse_user_package_dep_spec("breaking/dep", &data->env, UserPackageDepSpecOptions()));
            }

            if (confirm_)
                data->confirm_helper.add_allowed_to_break_spec(parse_user_package_dep_spec("*/*", &data->env, UserPackageDepSpecOptions() + updso_allow_wildcards));

            std::shared_ptr<const Resolved> resolved(data->get_resolved(make_uninstall_blocker(
                            parse_user_package_dep_spec("breaking/target", &data->env, UserPackageDepSpecOptions()))));

            if (allowed_to_remove_)
                this->check_resolved(resolved,
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
            else if (confirm_)
                this->check_resolved(resolved,
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
                this->check_resolved(resolved,
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
    };
}

typedef TestUninstallBreaking<false, false> UninstallBreakingFF;
typedef TestUninstallBreaking<false, true> UninstallBreakingFT;
typedef TestUninstallBreaking<true, false> UninstallBreakingTF;
typedef TestUninstallBreaking<true, true> UninstallBreakingTT;

TEST_F(UninstallBreakingFF, Works) { common_test_code(); }
TEST_F(UninstallBreakingFT, Works) { common_test_code(); }
TEST_F(UninstallBreakingTF, Works) { common_test_code(); }
TEST_F(UninstallBreakingTT, Works) { common_test_code(); }

