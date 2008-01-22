/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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

#include "subshell_die.hh"
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
    struct SubshellDieCheckTest : TestCase
    {
        SubshellDieCheckTest() : TestCase("subshell_die") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakePackageID> id(repo->add_version("cat", "pkg", "1"));

            TestReporter r1;
            TEST_CHECK(subshell_die_check(FSEntry("/var/empty"), r1, id, "src_unpack() {\n\tunpack \"${A}\"\n\tcd \"${S}\"\n\ttrue && ( epatch \"${FILESDIR}\"/${PN}-cookie.patch || die \"subshelled!\")\n}\n", "subshell_die"));
            TEST_CHECK_EQUAL(r1.count, 1u);

            TestReporter r2;
            TEST_CHECK(subshell_die_check(FSEntry("/var/empty"), r2, id, "src_unpack() {\n\t:\n}\n", "subshell_die"));
            TEST_CHECK_EQUAL(r2.count, 0u);
        }
    } test_subshell_die;
}


