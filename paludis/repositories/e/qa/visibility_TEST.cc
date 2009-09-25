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

#include "visibility.hh"
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/qa.hh>
#include <paludis/package_database.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
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

    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

namespace test_cases
{
    struct VisibilityTest : TestCase
    {
        VisibilityTest() : TestCase("visibility") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "visibility_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "visibility_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            {
                std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/visible-1", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r1;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r1, &env, repo, id1, "visibility"));
                TEST_CHECK_EQUAL(r1.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/visible-2", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r2;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r2, &env, repo, id2, "visibility"));
                TEST_CHECK_EQUAL(r2.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/masked-1", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r3;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r3, &env, repo, id3, "visibility"));
                TEST_CHECK_EQUAL(r3.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id4(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/needs-masked-1", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r4;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r4, &env, repo, id4, "visibility"));
                TestMessageSuffix s4(r4.messages);
                TEST_CHECK_EQUAL(r4.count, 1u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id5(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-1", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r5;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r5, &env, repo, id5, "visibility"));
                TestMessageSuffix s5(r5.messages);
                TEST_CHECK_EQUAL(r5.count, 1u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id6(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-2", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r6;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r6, &env, repo, id6, "visibility"));
                TestMessageSuffix s6(r6.messages);
                TEST_CHECK_EQUAL(r6.count, 1u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id7(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-3", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r7;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r7, &env, repo, id7, "visibility"));
                TEST_CHECK_EQUAL(r7.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id8(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-4", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r8;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r8, &env, repo, id8, "visibility"));
                TEST_CHECK_EQUAL(r8.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id9(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-5", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r9;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r9, &env, repo, id9, "visibility"));
                TEST_CHECK_EQUAL(r9.count, 0u);
            }

            {
                std::tr1::shared_ptr<const PackageID> id10(*env[selection::RequireExactlyOne(generator::Matches(parse_user_package_dep_spec(
                                    "=cat-one/use-masking-6", &env, UserPackageDepSpecOptions()), MatchPackageOptions()))]->begin());
                TestReporter r10;
                TEST_CHECK(visibility_check(FSEntry("/var/empty"), r10, &env, repo, id10, "visibility"));
                TEST_CHECK_EQUAL(r10.count, 0u);
            }
        }
    } test_visibility;
}


