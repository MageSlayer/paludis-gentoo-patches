/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/package_database_entry.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/join.hh>

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
            const KeywordName & k, const PackageDatabaseEntry & e)
    {
        std::tr1::shared_ptr<KeywordNameCollection> kk(new KeywordNameCollection::Concrete);
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

            PackageDatabaseEntry x(QualifiedPackageName("x/x"), VersionSpec("0"), RepositoryName("repo"));

            TEST_CHECK(env.query_use(UseFlagName("one"), x));
            TEST_CHECK(env.query_use(UseFlagName("two"), x));
            TEST_CHECK(! env.query_use(UseFlagName("three"), x));
            TEST_CHECK(! env.query_use(UseFlagName("four"), x));
            TEST_CHECK(! env.query_use(UseFlagName("five"), x));

            PackageDatabaseEntry d(QualifiedPackageName("app/one"), VersionSpec("1"),
                    RepositoryName("repo"));

            TEST_CHECK(! env.query_use(UseFlagName("one"), d));
            TEST_CHECK(env.query_use(UseFlagName("two"), d));
            TEST_CHECK(! env.query_use(UseFlagName("three"), d));
            TEST_CHECK(env.query_use(UseFlagName("four"), d));
            TEST_CHECK(! env.query_use(UseFlagName("five"), d));
        }
    } test_query_use;

    struct KnownUseNamesTest : TestCase
    {
        KnownUseNamesTest() : TestCase("known_use_expand_names") { }

        void run()
        {
            PortageEnvironment env("portage_environment_TEST_dir/known_use_expand_names");

            PackageDatabaseEntry pde1(QualifiedPackageName("app/one"), VersionSpec("1"), RepositoryName("foo"));
            std::tr1::shared_ptr<const UseFlagNameCollection> k1(env.known_use_expand_names(UseFlagName("foo_cards"), pde1));
            TEST_CHECK_EQUAL(join(k1->begin(), k1->end(), " "), "foo_cards_one foo_cards_three");
        }
    } test_known_use_expand;

    struct AcceptKeywordsTest : TestCase
    {
        AcceptKeywordsTest() : TestCase("accept_keywords") { }

        void run()
        {
            TestPortageEnvironment env("portage_environment_TEST_dir/accept_keywords");

            PackageDatabaseEntry x(QualifiedPackageName("x/x"), VersionSpec("0"), RepositoryName("repo"));

            TEST_CHECK(accept_keyword(env, KeywordName("arch"), x));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), x));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), x));

            PackageDatabaseEntry d1(QualifiedPackageName("app/one"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(accept_keyword(env, KeywordName("arch"), d1));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), d1));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), d1));

            PackageDatabaseEntry d2(QualifiedPackageName("app/two"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(accept_keyword(env, KeywordName("other_arch"), d2));
            TEST_CHECK(accept_keyword(env, KeywordName("arch"), d2));
            TEST_CHECK(accept_keyword(env, KeywordName("~arch"), d2));

            PackageDatabaseEntry d3(QualifiedPackageName("app/three"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(! accept_keyword(env, KeywordName("other_arch"), d3));
            TEST_CHECK(! accept_keyword(env, KeywordName("arch"), d3));
            TEST_CHECK(! accept_keyword(env, KeywordName("~arch"), d3));

            PackageDatabaseEntry d4(QualifiedPackageName("app/four"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(accept_keyword(env, KeywordName("fred"), d4));

        }
    } test_accept_keywords;
}

