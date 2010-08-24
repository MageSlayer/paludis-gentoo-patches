/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/util/wildcard_expander.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/join.hh>

#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct WildcardExpanderGeneralTest : TestCase
    {
        WildcardExpanderGeneralTest() : TestCase("WildcardExpander general") {}

        std::string expand(const std::string & pattern)
        {
            return join(WildcardExpander(pattern, FSPath::cwd() / "wildcard_expander_TEST_dir"),
                        WildcardExpander(), " ");
        }

        void run()
        {
            TEST_CHECK_EQUAL(expand("/xyz*zy"), "/xyz1zy /xyz22zy /xyzzy");
            TEST_CHECK_EQUAL(expand("/plugh"), "/plugh");
            TEST_CHECK_EQUAL(expand("/quux"), "/quux");
            TEST_CHECK_EQUAL(expand("/quux*quux"), "/quux*quux");
            TEST_CHECK_EQUAL(expand("/meh/?"), "/meh/1 /meh/2 /meh/3");
            TEST_CHECK_EQUAL(expand("/foo*\\*"), "/foo* /foo123*");
            TEST_CHECK_EQUAL(expand("/foo\\*"), "/foo*");
        }
    } wildcard_expander_general_test;

    struct WildcardExpanderIteratorSanityTest : TestCase
    {
        WildcardExpanderIteratorSanityTest() : TestCase("WildcardExpander iterator sanity") {}

        void run()
        {
            WildcardExpander it("/foo*bar", FSPath::cwd() / "wildcard_expander_TEST_dir");
            TEST_CHECK(it == it);
            TEST_CHECK(! (it != it));
            TEST_CHECK(it != WildcardExpander());
            TEST_CHECK_STRINGIFY_EQUAL(*it, "/fooAbar");
            TEST_CHECK_EQUAL(it->basename(), "fooAbar");

            WildcardExpander it2(it);
            TEST_CHECK(it == it2);
            TEST_CHECK(! (it != it2));
            TEST_CHECK(it2 != WildcardExpander());
            TEST_CHECK_STRINGIFY_EQUAL(*++it2, "/fooBbar");
            TEST_CHECK_STRINGIFY_EQUAL(*it2, "/fooBbar");
            TEST_CHECK_EQUAL(it2->basename(), "fooBbar");
            TEST_CHECK(it != it2);
            TEST_CHECK(! (it == it2));
            TEST_CHECK(it2 != WildcardExpander());

            WildcardExpander it3(it2);
            TEST_CHECK(it2 == it3++);
            TEST_CHECK(it2 != it3);
            TEST_CHECK(it3 != WildcardExpander());
            TEST_CHECK_STRINGIFY_EQUAL(*it3, "/fooCbar");
            TEST_CHECK_EQUAL(it3->basename(), "fooCbar");

            it3 = it2;
            TEST_CHECK(it2 == it3);
            TEST_CHECK_STRINGIFY_EQUAL(*it3, "/fooBbar");
            TEST_CHECK_STRINGIFY_EQUAL(*it3++, "/fooBbar");
            TEST_CHECK(it3 != WildcardExpander());

            TEST_CHECK(++it3 != WildcardExpander());
            TEST_CHECK(++it3 != WildcardExpander());
            TEST_CHECK(++it3 == WildcardExpander());

            TEST_CHECK_EQUAL(join(WildcardExpander("/foo*bar", FSPath::cwd() / "wildcard_expander_TEST_dir"),
                                  WildcardExpander(), " "),
                             "/fooAbar /fooBbar /fooCbar /fooDbar /fooEbar");
        }
    } wildcard_expander_iterator_sanity_test;
}

