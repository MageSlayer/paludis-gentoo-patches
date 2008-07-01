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

#include "portage_environment.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/util/join.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

using namespace paludis;
using namespace test;

namespace
{
    class TestPortageEnvironment :
        public PortageEnvironment
    {
        public:
            using PortageEnvironment::accept_keywords;

            TestPortageEnvironment(const std::string & e) :
                PortageEnvironment(e)
            {
            }
    };

    bool accept_keyword(const TestPortageEnvironment & env,
            const KeywordName & k, const PackageID & e)
    {
        std::tr1::shared_ptr<KeywordNameSet> kk(new KeywordNameSet);
        kk->insert(k);
        return env.accept_keywords(kk, e);
    }
}

namespace test_cases
{
    struct QueryUseTest : TestCase
    {
        QueryUseTest() : TestCase("query_use") { }

        void run()
        {
            PortageEnvironment env("portage_environment_TEST_dir/query_use");

            const std::tr1::shared_ptr<const PackageID> idx(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-x-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK(env.query_use(UseFlagName("one"), *idx));
            TEST_CHECK(env.query_use(UseFlagName("two"), *idx));
            TEST_CHECK(! env.query_use(UseFlagName("three"), *idx));
            TEST_CHECK(! env.query_use(UseFlagName("four"), *idx));
            TEST_CHECK(! env.query_use(UseFlagName("five"), *idx));
            TEST_CHECK(! env.query_use(UseFlagName("six"), *idx));

            TEST_CHECK(! env.query_use(UseFlagName("one"), *id1));
            TEST_CHECK(env.query_use(UseFlagName("two"), *id1));
            TEST_CHECK(! env.query_use(UseFlagName("three"), *id1));
            TEST_CHECK(env.query_use(UseFlagName("four"), *id1));
            TEST_CHECK(! env.query_use(UseFlagName("five"), *id1));
            TEST_CHECK(! env.query_use(UseFlagName("six"), *id1));
        }
    } test_query_use;

    struct KnownUseNamesTest : TestCase
    {
        KnownUseNamesTest() : TestCase("known_use_expand_names") { }

        void run()
        {
            PortageEnvironment env("portage_environment_TEST_dir/known_use_expand_names");

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());
            std::tr1::shared_ptr<const UseFlagNameSet> k1(env.known_use_expand_names(UseFlagName("foo_cards"), *id1));
            TEST_CHECK_EQUAL(join(k1->begin(), k1->end(), " "), "foo_cards_one foo_cards_three");
        }
    } test_known_use_expand;

    struct AcceptKeywordsTest : TestCase
    {
        AcceptKeywordsTest() : TestCase("accept_keywords") { }

        void run()
        {
            TestPortageEnvironment env("portage_environment_TEST_dir/accept_keywords");

            const std::tr1::shared_ptr<const PackageID> idx(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-x-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *idx));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *idx));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), *idx));

            const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *id1));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *id1));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), *id1));

            const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), *id2));
            TEST_CHECK(accept_keyword(env, KeywordName("arch"), *id2));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), *id2));

            const std::tr1::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-three-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());

            TEST_CHECK(! accept_keyword(env, KeywordName("other_arch"), *id3));
            TEST_CHECK(! accept_keyword(env, KeywordName("arch"), *id3));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), *id3));

            const std::tr1::shared_ptr<const PackageID> id4(*env[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-four-1",
                                    &env, UserPackageDepSpecOptions()))))]->begin());
            TEST_CHECK(accept_keyword(env, KeywordName("fred"), *id4));
        }
    } test_accept_keywords;
}

