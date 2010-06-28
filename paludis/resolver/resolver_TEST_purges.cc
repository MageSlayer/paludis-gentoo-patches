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
    struct ResolverPurgesTestCase : ResolverTestCase
    {
        ResolverPurgesTestCase(const std::string & s) :
            ResolverTestCase("purges", s, "exheres-0", "exheres")
        {
        }
    };

    UseExisting
    use_existing_if_possible_except_target(
            const std::tr1::shared_ptr<const Resolution> &,
            const PackageDepSpec & s,
            const std::tr1::shared_ptr<const Reason> &)
    {
        if (s.package_ptr() && s.package_ptr()->package() == PackageNamePart("target"))
            return ue_never;
        else
            return ue_if_possible;
    }
}

namespace test_cases
{
    struct TestPurges : ResolverPurgesTestCase
    {
        TestPurges() :
            ResolverPurgesTestCase("purges")
        {
            install("purges", "target", "0")->build_dependencies_key()->set_from_string("purges/still-used-dep purges/old-dep purges/unrelated-dep");
            install("purges", "old-dep", "0");
            install("purges", "still-used-dep", "0");
            install("purges", "unrelated-dep", "0");
            install("purges", "unrelated", "0")->build_dependencies_key()->set_from_string("purges/unrelated-dep");

            allowed_to_remove_names->insert(QualifiedPackageName("purges/old-dep"));
        }

        virtual ResolverFunctions get_resolver_functions(InitialConstraints & initial_constraints)
        {
            ResolverFunctions result(ResolverPurgesTestCase::get_resolver_functions(initial_constraints));
            result.get_use_existing_fn() = std::tr1::bind(&use_existing_if_possible_except_target, std::tr1::placeholders::_1,
                    std::tr1::placeholders::_2, std::tr1::placeholders::_3);
            return result;
        }

        void run()
        {
            std::tr1::shared_ptr<const Resolved> resolved(get_resolved("purges/target"));
            check_resolved(resolved,
                    n::taken_change_or_remove_decisions() = make_shared_copy(DecisionChecks()
                        .change(QualifiedPackageName("purges/new-dep"))
                        .remove(QualifiedPackageName("purges/old-dep"))
                        .change(QualifiedPackageName("purges/target"))
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
}

