/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for instantiation_policy.hh .
 *
 */

namespace
{
    /**
     * Test class for InstantiationPolicy.
     *
     */
    class MyClass :
        public InstantiationPolicy<MyClass, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<MyClass, instantiation_method::SingletonTag>;

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

    class MyClassTwo :
        public InstantiationPolicy<MyClassTwo, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<MyClassTwo, instantiation_method::SingletonTag>;

        private:
            MyClassTwo()
            {
                ++instances;
            }

            ~MyClassTwo()
            {
                --instances;
            }

        public:
            std::string s;

            static int instances;
    };

    int MyClassTwo::instances = 0;

    class MyRecursiveClass :
        public InstantiationPolicy<MyRecursiveClass, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<MyRecursiveClass, instantiation_method::SingletonTag>;

        public:
            std::string s;

        private:
            MyRecursiveClass() :
                s(MyRecursiveClass::get_instance()->s)
            {
            }
    };
}

namespace test_cases
{
    /**
     * \test Test singleton behaviour.
     *
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
            TEST_CHECK(MyClass::get_instance() == MyClass::get_instance());
            TEST_CHECK(MyClass::get_instance()->s.empty());
            MyClass::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyClass::get_instance()->s, "foo");
        }
    } test_singleton_pattern;

    /**
     * \test Test singleton behaviour.
     *
     */
    struct SingletonPatternDeleteTest : TestCase
    {
        SingletonPatternDeleteTest() : TestCase("singleton delete test") { }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TEST_CHECK_EQUAL(MyClassTwo::instances, 0);
            TEST_CHECK(0 != MyClassTwo::get_instance());
            TEST_CHECK_EQUAL(MyClassTwo::instances, 1);
            TEST_CHECK(MyClassTwo::get_instance() == MyClassTwo::get_instance());
            TEST_CHECK(MyClassTwo::get_instance()->s.empty());
            MyClassTwo::get_instance()->s = "foo";
            TEST_CHECK_EQUAL(MyClassTwo::get_instance()->s, "foo");
            MyClassTwo::destroy_instance();
            TEST_CHECK_EQUAL(MyClassTwo::instances, 0);
            TEST_CHECK(0 != MyClassTwo::get_instance());
            TEST_CHECK_EQUAL(MyClassTwo::instances, 1);
            TEST_CHECK(MyClassTwo::get_instance()->s.empty());
        }
    } test_singleton_pattern_delete;

    struct SingletonPatternRecursiveTest : TestCase
    {
        SingletonPatternRecursiveTest() : TestCase("singleton recursive test") { }

        void run()
        {
            TEST_CHECK_THROWS(MyRecursiveClass * x = MyRecursiveClass::get_instance(), InternalError);
        }
    } test_singleton_pattern_recurse;
}

