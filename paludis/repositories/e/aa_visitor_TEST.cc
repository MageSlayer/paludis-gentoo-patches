/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Mike Kelly
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/aa_visitor.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::erepository;

TEST(AAVisitor, Works)
{
    TestEnvironment env;
    std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo"))));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    AAVisitor p1;
    parse_fetchable_uri("( a -> b c x? ( d e ) )", &env, *EAPIData::get_instance()->eapi_from_string("paludis-1"), false)->top()->accept(p1);
    EXPECT_EQ("b c d e", join(p1.begin(), p1.end(), " "));
}

