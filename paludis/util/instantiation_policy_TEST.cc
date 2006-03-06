/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/util/instantiation_policy.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>

using namespace test;
using namespace paludis;

/**
 * \file
 * Test cases for instantiation_policy.hh .
 */

#ifndef DOXYGEN

class MyClass :
    public InstantiationPolicy<MyClass, instantiation_method::SingletonAsNeededTag>
{
    friend class InstantiationPolicy<MyClass, instantiation_method::SingletonAsNeededTag>;

    private:
        MyClass()
        {
            ++instances;
        }

    public:
        std::string s;

        static int instances;
};

int MyClass::instances = 0;

struct MyLoadAtStartupClass :
    public InstantiationPolicy<MyLoadAtStartupClass, instantiation_method::SingletonAtStartupTag>
{
    friend class InstantiationPolicy<MyLoadAtStartupClass, instantiation_method::SingletonAtStartupTag>;

    private:
        MyLoadAtStartupClass()
        {
            ++instances;
        }

    public:
        std::string s;

        static int instances;
};

int MyLoadAtStartupClass::instances = 0;

#endif

namespace test_cases
{
    /**
     * \test Test singleton behaviour.
     */
    struct SingletonPatternTest : TestCase
    {
        SingletonPatternTest() : TestCase("singleton test") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TEST_CHECK_EQUAL(MyClass::instances, 0);
            TEST_CHECK(0 != MyClass::get_instance());
            TEST_CHECK_EQUAL(MyClass::instances, 1);
            TEST_CHECK_EQUAL(MyClass::get_instance(), MyClass::get_instance());
            TEST_CHECK(MyClass::get_instance()->s.empty());
            MyClass::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyClass::get_instance()->s, "foo");
        }
    } test_singleton_pattern;

    /**
     * \test Test singleton create at startup behaviour.
     */
    struct SingletonPatternCreateAtStartupTest : TestCase
    {
        SingletonPatternCreateAtStartupTest() : TestCase("singleton create at startup test") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TEST_CHECK_EQUAL(MyLoadAtStartupClass::instances, 1);
            TEST_CHECK(0 != MyLoadAtStartupClass::get_instance());
            TEST_CHECK_EQUAL(MyLoadAtStartupClass::instances, 1);
            TEST_CHECK_EQUAL(MyLoadAtStartupClass::get_instance(), MyLoadAtStartupClass::get_instance());
            TEST_CHECK(MyLoadAtStartupClass::get_instance()->s.empty());
            MyLoadAtStartupClass::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyLoadAtStartupClass::get_instance()->s, "foo");
        }
    } test_singleton_pattern_create_at_startup;
}



