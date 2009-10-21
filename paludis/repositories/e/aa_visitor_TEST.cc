/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
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

#include "aa_visitor.hh"
#include "dep_parser.hh"
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/package_database.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;
using namespace paludis::erepository;

namespace test_cases
{
    struct AAVisitorTest : TestCase
    {
        AAVisitorTest() : TestCase("aa visitor") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("repo")))));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            AAVisitor p1;
            parse_fetchable_uri("( a -> b c x? ( d e ) )", &env, id, *EAPIData::get_instance()->eapi_from_string("paludis-1"))->root()->accept(p1);
            TEST_CHECK_EQUAL(join(p1.begin(), p1.end(), " "), "b c d e");
        }
    } test_aa_visitor;
}

