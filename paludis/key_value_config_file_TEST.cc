/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "stringify.hh"
#include "key_value_config_file.hh"
#include "configuration_error.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test KeyValueConfigFile basics.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileTest : TestCase
    {
        KeyValueConfigFileTest() : TestCase("key value config file") { }

        void run()
        {
            std::stringstream s;
            s << "one=first" << std::endl;
            s << "two = second" << std::endl;
            s << "three" << std::endl;
            s << "four = \"fourth\" " << std::endl;
            s << "five = ''" << std::endl;
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("one"), "first");
            TEST_CHECK_EQUAL(ff.get("two"), "second");
            TEST_CHECK_EQUAL(ff.get("three"), "");
            TEST_CHECK_EQUAL(ff.get("four"), "fourth");
            TEST_CHECK_EQUAL(ff.get("five"), "");
            TEST_CHECK_EQUAL(ff.get("six"), "");
        }
    } test_key_value_config_file;

    /**
     * \test Test KeyValueConfigFile variables.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileVarsTest : TestCase
    {
        KeyValueConfigFileVarsTest() : TestCase("key value config file with vars") { }

        void run()
        {
            std::stringstream s;
            s << "x=foo" << std::endl;
            s << "y = \"${x}\\\\${y}\\$${z}\"" << std::endl;
            s << "z = $x$y$z" << std::endl;
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("x"), "foo");
            TEST_CHECK_EQUAL(ff.get("y"), "foo\\$");
            TEST_CHECK_EQUAL(ff.get("z"), "foofoo\\$");
        }
    } test_key_value_config_file_vars;

    /**
     * \test Test KeyValueConfigFile errors.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileErrorsTest : TestCase
    {
        KeyValueConfigFileErrorsTest() : TestCase("key value config file with errors") { }

        void run()
        {
            std::stringstream s1;
            s1 << "x='" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s1), ConfigurationError);

            std::stringstream s2;
            s2 << "x='moo\"" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s2), ConfigurationError);

            std::stringstream s3;
            s3 << "x=${foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s3), ConfigurationError);

            std::stringstream s4;
            s4 << "x=$~" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s4), ConfigurationError);

            std::stringstream s5;
            s5 << "x=abc\\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s5), ConfigurationError);
        }
    } test_key_value_config_file_errors;
}

