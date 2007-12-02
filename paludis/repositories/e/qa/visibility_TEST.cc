/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include "visibility.hh"
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/qa.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
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
            messages.append(m.message);
        }

        void status(const std::string &)
        {
        }
    };
}

namespace test_cases
{
    struct VisibilityTest : TestCase
    {
        VisibilityTest() : TestCase("visibility") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "visibility_TEST_dir/repo1");
            keys->insert("profiles", stringify(FSEntry::cwd() / "visibility_TEST_dir/repo1/profiles/test"));
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            {
                tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(parse_user_package_dep_spec(
                                    "=cat-one/visible-1", UserPackageDepSpecOptions())), qo_require_exactly_one)->begin());
                TestReporter r1;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r1, &env, repo, id1, "visibility"));
                TEST_CHECK_EQUAL(r1.count, 0u);
            }

            {
                tr1::shared_ptr<const PackageID> id2(*env.package_database()->query(query::Matches(parse_user_package_dep_spec(
                                    "=cat-one/visible-2", UserPackageDepSpecOptions())), qo_require_exactly_one)->begin());
                TestReporter r2;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r2, &env, repo, id2, "visibility"));
                TEST_CHECK_EQUAL(r2.count, 0u);
            }

            {
                tr1::shared_ptr<const PackageID> id3(*env.package_database()->query(query::Matches(parse_user_package_dep_spec(
                                    "=cat-one/masked-1", UserPackageDepSpecOptions())), qo_require_exactly_one)->begin());
                TestReporter r3;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r3, &env, repo, id3, "visibility"));
                TEST_CHECK_EQUAL(r3.count, 0u);
            }

            {
                tr1::shared_ptr<const PackageID> id4(*env.package_database()->query(query::Matches(parse_user_package_dep_spec(
                                    "=cat-one/needs-masked-1", UserPackageDepSpecOptions())), qo_require_exactly_one)->begin());
                TestReporter r4;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r4, &env, repo, id4, "visibility"));
                TestMessageSuffix s4(r4.messages);
                TEST_CHECK_EQUAL(r4.count, 1u);
            }
        }
    } test_visibility;
}


