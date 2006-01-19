/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "stringify.hh"
#include "key_value_config_file.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace paludis;
using namespace test;

namespace test_cases
{
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
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("one"), "first");
            TEST_CHECK_EQUAL(ff.get("two"), "second");
            TEST_CHECK_EQUAL(ff.get("three"), "");
            TEST_CHECK_EQUAL(ff.get("four"), "fourth");
            TEST_CHECK_EQUAL(ff.get("five"), "");
        }
    } test_key_value_config_file;

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
}

