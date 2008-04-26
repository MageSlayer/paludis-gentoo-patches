/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2007 Fernando J. Pereda
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

#include "function_keyword.hh"
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

        TestReporter() :
            count(0)
        {
        }

        void message(const QAMessage &)
        {
            ++count;
        }

        void status(const std::string &)
        {
        }
    };
}

namespace test_cases
{
    struct FunctionKeywordTest : TestCase
    {
        FunctionKeywordTest() : TestCase("function_keyword") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));
            id->build_dependencies_key()->set_from_string("cat/other");

            TestReporter r1;
            TEST_CHECK(function_keyword_check(FSEntry("/var/empty"), r1, id, "function something () {\n    : ;\n}\n", "function_keyword"));
            TEST_CHECK_EQUAL(r1.count, 1u);

            TestReporter r2;
            TEST_CHECK(function_keyword_check(FSEntry("/var/empty"), r2, id, "function foo\n{\n    : ;\n}\n", "function_keyword"));
            TEST_CHECK_EQUAL(r2.count, 1u);

            TestReporter r3;
            TEST_CHECK(function_keyword_check(FSEntry("/var/empty"), r3, id, "function       bar      (){\n    : ;\n}\n", "function_keyword"));
            TEST_CHECK_EQUAL(r3.count, 1u);

            TestReporter r4;
            TEST_CHECK(function_keyword_check(FSEntry("/var/empty"), r4, id, "# function something\nbah() {\n    : ;\n}\n", "function_keyword"));
            TEST_CHECK_EQUAL(r4.count, 0u);
        }
    } test_function_keyword;
}

