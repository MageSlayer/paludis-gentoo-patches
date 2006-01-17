/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "translate_insert_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

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

int f(const int & v)
{
    return -v;
}
#endif

namespace test_cases
{
    struct TranslateInsertIteratorTest : TestCase
    {
        TranslateInsertIteratorTest() : TestCase("translate insert iterator") { }

        void run()
        {
            std::vector<int> v;
            std::generate_n(translate_inserter(std::back_inserter(v), std::ptr_fun(&f)),
                    5, Counter());
            TEST_CHECK_EQUAL(v.size(), 5);
            for (int n = 0 ; n < 5 ; ++n)
            {
                TestMessageSuffix s("n=" + stringify(n));
                TEST_CHECK_EQUAL(v.at(n), -n);
            }
        }
    } test_translate_insert_iterator;
}

