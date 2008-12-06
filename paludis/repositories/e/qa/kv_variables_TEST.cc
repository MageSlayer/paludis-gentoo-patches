/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "kv_variables.hh"
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/qa.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::erepository;
using namespace test;

namespace
{
    struct TestReporter :
        QAReporter
    {
        unsigned count;
        std::string messages;

        TestReporter() :
            count(0)
        {
        }

        void message(const QAMessage & m)
        {
            ++count;
            if (! messages.empty())
                messages.append(", ");
            messages.append(m.message());
        }

        void status(const std::string &)
        {
        }
    };
}

namespace test_cases
{
    struct GoodTest : TestCase
    {
        GoodTest() : TestCase("good") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("cat/other");

            TestReporter r;
            TEST_CHECK(kv_variables_check(FSEntry("/var/empty"), r, id, "X=KV\n", "kv_variables"));
            TEST_CHECK_EQUAL(r.count, 0u);
        }
    } test_good;

    struct BadTest : TestCase
    {
        BadTest() : TestCase("bad") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("cat/other");

            TestReporter r;
            TEST_CHECK(kv_variables_check(FSEntry("/var/empty"), r, id, "SLOT=\"foo-$KV\"\n", "kv_variables"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_bad;
}

