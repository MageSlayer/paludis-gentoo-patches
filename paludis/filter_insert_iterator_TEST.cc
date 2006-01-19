/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "filter_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <set>
#include <algorithm>

using namespace test;
using namespace paludis;

#ifndef DOXYGEN
struct Counter
{
    int n;

    Counter() :
        n(0)
    {
    }

    int operator() ()
    {
        return n++;
    }
};

int is_even(const int & v)
{
    return ! (v & 1);
}
#endif


namespace test_cases
{
    struct FilterInsertIteratorTest : TestCase
    {
        FilterInsertIteratorTest() : TestCase("filter insert iterator") { }

        void run()
        {
            std::set<int> v;
            std::generate_n(filter_inserter(std::inserter(v, v.begin()), std::ptr_fun(&is_even)),
                    5, Counter());
            TEST_CHECK_EQUAL(v.size(), 3);
            for (int n = 0 ; n < 5 ; ++n)
            {
                TestMessageSuffix s("n=" + stringify(n));
                if (is_even(n))
                {
                    TEST_CHECK(v.end() != v.find(n));
                    TEST_CHECK_EQUAL(*v.find(n), n);
                }
                else
                    TEST_CHECK(v.end() == v.find(n));
            }
        }
    } test_filter_insert_iterator;
}

