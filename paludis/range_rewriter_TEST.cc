/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/range_rewriter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/environments/test/test_environment.hh>

#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <algorithm>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct RangeRewriterTestCase :
        TestCase
    {
        RangeRewriterTestCase() : TestCase("range rewriter") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<DependencySpecTree> a(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>()));
            a->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("=a/b-1",
                                &env, { })));
            a->root()->append(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec("=a/b-2",
                                &env, { })));

            RangeRewriter r;
            TEST_CHECK(! r.spec());
            std::for_each(indirect_iterator(a->root()->begin()), indirect_iterator(a->root()->end()), accept_visitor(r));
            TEST_CHECK(bool(r.spec()));

            TEST_CHECK(bool(r.spec()));
            TEST_CHECK_STRINGIFY_EQUAL(*r.spec(), "a/b[=1|=2] (rewritten from { =a/b-1, =a/b-2 })");
        }
    } test_range_rewriter;
}

