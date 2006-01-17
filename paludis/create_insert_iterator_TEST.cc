/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "create_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

#ifndef DOXYGEN
struct C
{
    std::string s;

    explicit C(const std::string & ss) :
        s(ss)
    {
    }
};
#endif

namespace test_cases
{
    struct CreateInsertIteratorTest : TestCase
    {
        CreateInsertIteratorTest() : TestCase("create insert iterator") { }

        void run()
        {
            std::vector<std::string> v;
            v.push_back("one");
            v.push_back("two");

            std::vector<C> vv;
            std::copy(v.begin(), v.end(), create_inserter<C>(std::back_inserter(vv)));

            TEST_CHECK_EQUAL(vv.size(), 2);
            TEST_CHECK_EQUAL(vv.at(0).s, "one");
            TEST_CHECK_EQUAL(vv.at(1).s, "two");
        }
    } test_create_insert_iterator;
}

