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

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct QueryUseTest : TestCase
    {
        QueryUseTest() : TestCase("query_use") { }

        void run()
        {
            PortageEnvironment env("portage_environment_TEST_dir/query_use");

            TEST_CHECK(env.query_use(UseFlagName("one"), 0));
            TEST_CHECK(env.query_use(UseFlagName("two"), 0));
            TEST_CHECK(! env.query_use(UseFlagName("three"), 0));
            TEST_CHECK(! env.query_use(UseFlagName("four"), 0));
            TEST_CHECK(! env.query_use(UseFlagName("five"), 0));

            PackageDatabaseEntry d(QualifiedPackageName("app/one"), VersionSpec("1"),
                    RepositoryName("repo"));

            TEST_CHECK(! env.query_use(UseFlagName("one"), &d));
            TEST_CHECK(env.query_use(UseFlagName("two"), &d));
            TEST_CHECK(! env.query_use(UseFlagName("three"), &d));
            TEST_CHECK(env.query_use(UseFlagName("four"), &d));
            TEST_CHECK(! env.query_use(UseFlagName("five"), &d));
        }
    } test_query_use;

    struct AcceptKeywordsTest : TestCase
    {
        AcceptKeywordsTest() : TestCase("accept_keywords") { }

        void run()
        {
            PortageEnvironment env("portage_environment_TEST_dir/accept_keywords");

            TEST_CHECK(env.accept_keyword(KeywordName("arch"), 0));
            TEST_CHECK(env.accept_keyword(KeywordName("other_arch"), 0));
            TEST_CHECK(! env.accept_keyword(KeywordName("~arch"), 0));

            PackageDatabaseEntry d1(QualifiedPackageName("app/one"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(env.accept_keyword(KeywordName("arch"), &d1));
            TEST_CHECK(env.accept_keyword(KeywordName("other_arch"), &d1));
            TEST_CHECK(env.accept_keyword(KeywordName("~arch"), &d1));

            PackageDatabaseEntry d2(QualifiedPackageName("app/two"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(env.accept_keyword(KeywordName("other_arch"), &d2));
            TEST_CHECK(env.accept_keyword(KeywordName("arch"), &d2));
            TEST_CHECK(env.accept_keyword(KeywordName("~arch"), &d2));

            PackageDatabaseEntry d3(QualifiedPackageName("app/three"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(! env.accept_keyword(KeywordName("other_arch"), &d3));
            TEST_CHECK(! env.accept_keyword(KeywordName("arch"), &d3));
            TEST_CHECK(! env.accept_keyword(KeywordName("~arch"), &d3));

            PackageDatabaseEntry d4(QualifiedPackageName("app/four"), VersionSpec("1"),
                    RepositoryName("repo"));
            TEST_CHECK(env.accept_keyword(KeywordName("fred"), &d4));

        }
    } test_accept_keywords;
}

