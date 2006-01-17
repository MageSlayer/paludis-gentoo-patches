/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "getenv.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct GetenvWithDefaultTest : TestCase
    {
        GetenvWithDefaultTest() : TestCase("getenv_with_default") { }

        void run()
        {
            TEST_CHECK(! getenv_with_default("HOME", "!").empty());
            TEST_CHECK_EQUAL(getenv_with_default("HOME", "!").at(0), '/');
            TEST_CHECK_EQUAL(getenv_with_default("THEREISNOSUCHVARIABLE", "moo"), "moo");
        }
    } test_getenv_with_default;

    struct GetenvOrErrorTest : TestCase
    {
        GetenvOrErrorTest() : TestCase("getenv_or_error") { }

        void run()
        {
            TEST_CHECK(! getenv_or_error("HOME").empty());
            TEST_CHECK_THROWS(getenv_or_error("THEREISNOSUCHVARIABLE"), GetenvError);
        }
    } test_getenv_or_error;
}
