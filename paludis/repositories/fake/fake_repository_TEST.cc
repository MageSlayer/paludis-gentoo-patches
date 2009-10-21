/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct FakeRepositoryTest : TestCase
    {
        FakeRepositoryTest() : TestCase("fake repository") { }

        void run()
        {
            TestEnvironment env;
            const std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("repo"))
                            )));
        }
    } test_fake_repository;
}


