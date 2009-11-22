/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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
#include <paludis/resolver/resolutions.hh>
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
    struct ResolverSimpleTestCase : ResolverTestCase
    {
        ResolverSimpleTestCase(const std::string & s) :
            ResolverTestCase("simple", s, "exheres-0", "exheres")
        {
        }
    };
}

namespace test_cases
{
    struct TestNoDeps : ResolverSimpleTestCase
    {
        TestNoDeps() : ResolverSimpleTestCase("no-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const ResolverLists> resolutions(get_resolutions("no-deps/target"));

            {
                TestMessageSuffix s("errors");
                check_resolution_list(resolutions->jobs(), resolutions->error_resolutions(), ResolutionListChecks()
                        .finished()
                        );
            }

            {
                TestMessageSuffix s("ordered");
                check_resolution_list(resolutions->jobs(), resolutions->ordered_job_ids(), ResolutionListChecks()
                        .qpn(QualifiedPackageName("no-deps/target"))
                        .finished()
                        );
            }
        }
    } test_no_deps;

    struct TestBuildDeps : ResolverSimpleTestCase
    {
        TestBuildDeps() : ResolverSimpleTestCase("build-deps") { }

        void run()
        {
            std::tr1::shared_ptr<const ResolverLists> resolutions(get_resolutions("build-deps/target"));

            {
                TestMessageSuffix s("errors");
                check_resolution_list(resolutions->jobs(), resolutions->error_resolutions(), ResolutionListChecks()
                        .finished()
                        );
            }

            {
                TestMessageSuffix s("ordered");
                check_resolution_list(resolutions->jobs(), resolutions->ordered_job_ids(), ResolutionListChecks()
                        .qpn(QualifiedPackageName("build-deps/a-dep"))
                        .qpn(QualifiedPackageName("build-deps/b-dep"))
                        .qpn(QualifiedPackageName("build-deps/z-dep"))
                        .qpn(QualifiedPackageName("build-deps/target"))
                        .finished()
                        );
            }
        }
    } test_build_deps;
}

